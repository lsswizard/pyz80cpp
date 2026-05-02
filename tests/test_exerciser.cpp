/**
 * Z80 Exerciser Test - C++ direct test
 * Runs ZEXDOC and ZEXALL without Python bindings for accurate timing
 */

#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>

#include <z80/z80.h>

// FNV-1 hash (matching ZEXDOC/ZEXALL)
static const uint32_t fnv1_32_INIT = 0x811C9DC5;

static uint32_t fnv1_32(const uint8_t* data, size_t len, uint32_t h = fnv1_32_INIT) {
    for (size_t i = 0; i < len; i++) {
        h ^= data[i];
        h = (h * 0x01000193) & 0xFFFFFFFF;
    }
    return h;
}

// Simple memory bus
class TestBus : public z80::Bus {
public:
    uint8_t memory[65536] = {};

    uint8_t read(uint16_t addr) override {
        return memory[addr];
    }

    void write(uint16_t addr, uint8_t val) override {
        memory[addr] = val;
    }
};

// Test harness
class ExerciserHarness {
public:
    TestBus bus;
    z80::Z80 cpu;
    static constexpr uint16_t HOOK_ADDR = 0xF000;

    ExerciserHarness() : cpu(&bus) {
        reset();
    }

    void reset() {
        cpu.reset();
        memset(bus.memory, 0, sizeof(bus.memory));
    }

    void load_com(const uint8_t* data, size_t size, uint16_t addr = 0x0100) {
        reset();
        for (size_t i = 0; i < size; i++) {
            bus.memory[addr + i] = data[i];
        }
    }

    void setup_cpm() {
        // BDOS hook at 0x0005 -> JP HOOK_ADDR
        bus.memory[0x0005] = 0xC3;  // JP
        bus.memory[0x0006] = HOOK_ADDR & 0xFF;
        bus.memory[0x0007] = HOOK_ADDR >> 8;

        // RET at hook address
        bus.memory[HOOK_ADDR] = 0xC9;

        // Initial state
        cpu.regs.PC = 0x0100;
        cpu.regs.SP = 0xFFFE;

        // Exit address on stack
        bus.memory[0xFFFE] = 0x00;
        bus.memory[0xFFFF] = 0x00;
    }

    struct Result {
        uint64_t cycles;
        uint32_t hash;
        uint32_t lines;
        uint32_t columns;
        uint16_t final_pc;
        bool halted;
    };

    Result run(uint64_t max_cycles) {
        Result result = {};
        uint32_t hash = fnv1_32_INIT;
        uint32_t lines = 0;
        uint32_t columns = 0;
        uint32_t cursor_x = 0;
        bool completed = false;
        
        while (!completed && cpu.get_cycles() < max_cycles) {
            // Check if we're about to execute the BDOS hook (HOOK_ADDR)
            // If so, handle BDOS call BEFORE executing the RET instruction
            if (cpu.regs.PC == HOOK_ADDR) {
                // The stack has the return address from CALL 5
                // Pop it (don't use cpu.step() to execute RET)
                uint16_t ret_addr = bus.memory[cpu.regs.SP] | (bus.memory[cpu.regs.SP + 1] << 8);
                cpu.regs.SP += 2;
                
                // Handle BDOS function
                uint8_t c = cpu.regs.C;
                if (c == 2) {  // Print char (E = char)
                    uint8_t ch = cpu.regs.E;
                    hash = fnv1_32(&ch, 1, hash);
                    
                    if (ch == 0x0A) {  // LF
                        lines++;
                        if (cursor_x > columns) columns = cursor_x;
                        cursor_x = 0;
                    } else if (ch != 0x0D) {  // Ignore CR for cursor
                        cursor_x++;
                    }
                } else if (c == 9) {  // Print string (DE = $-terminated string)
                    uint16_t de = cpu.regs.DE();
                    for (int i = 0; i < 255; i++) {
                        uint8_t ch = bus.memory[de];
                        if (ch == 0x24) break;  // $ terminator
                        hash = fnv1_32(&ch, 1, hash);
                        
                        if (ch == 0x0A) {
                            lines++;
                            if (cursor_x > columns) columns = cursor_x;
                            cursor_x = 0;
                        } else if (ch != 0x0D) {
                            cursor_x++;
                        }
                        de = (de + 1) & 0xFFFF;
                    }
                } else if (c == 0) {  // Terminate
                    completed = true;
                }
                
                // Set PC to return address (skip the RET instruction)
                cpu.regs.PC = ret_addr;
                continue;  // Don't execute the RET, we already handled it
            }
            
            cpu.step();

            if (cpu.is_halted()) {
                completed = true;
            }

            if (cpu.regs.PC == 0) {
                completed = true;
            }
        }

        result.cycles = cpu.get_cycles();
        result.hash = hash;
        result.lines = lines;
        result.columns = columns;
        result.final_pc = cpu.regs.PC;
        result.halted = cpu.is_halted();

        return result;
    }
};

// Load file into vector
std::vector<uint8_t> load_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Cannot open: " << path << "\n";
        return {};
    }
    size_t size = file.tellg();
    file.seekg(0);
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

void run_test(const std::string& name, const std::vector<uint8_t>& data,
              uint32_t expected_hash, uint64_t expected_cycles) {
    ExerciserHarness harness;
    harness.load_com(data.data(), data.size());
    harness.setup_cpm();

    auto start = std::chrono::high_resolution_clock::now();
    auto result = harness.run(expected_cycles);
    auto end = std::chrono::high_resolution_clock::now();

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << name << ":\n";
    std::cout << "  Cycles:  " << result.cycles << " (expected " << expected_cycles << ")\n";
    std::cout << "  Hash:   " << std::hex << result.hash << " (expected " << expected_hash << std::dec << ")\n";
    std::cout << "  Lines:  " << result.lines << ", Columns: " << result.columns << "\n";
    std::cout << "  PC:     " << std::hex << result.final_pc << std::dec << "\n";
    std::cout << "  Time:   " << ms << "ms\n";

    if (result.hash == expected_hash) {
        std::cout << "  PASSED\n\n";
    } else {
        std::cout << "  FAILED\n\n";
    }
}

int main(int argc, char** argv) {
    // Find test files
    std::string zexdoc_path = "tests/z80_tests/zexdoc.com";
    std::string zexall_path = "tests/z80_tests/zexall.com";

    if (argc >= 3) {
        zexdoc_path = argv[1];
        zexall_path = argv[2];
    }

    std::cout << "Z80 Exerciser Test\n";
    std::cout << "==================\n\n";

    // Load test files
    auto zexdoc = load_file(zexdoc_path);
    auto zexall = load_file(zexall_path);

    if (zexdoc.empty()) {
        std::cerr << "ZEXDOC not found: " << zexdoc_path << "\n";
        return 1;
    }
    if (zexall.empty()) {
        std::cerr << "ZEXALL not found: " << zexall_path << "\n";
        return 1;
    }

    // Expected values from test-Z80.c
    // ZEXDOC: 46,734,977,146 cycles, hash 0xEDE3CB62
    // ZEXALL: 46,734,977,146 cycles, hash 0xEDE3CB62

    run_test("ZEXDOC", zexdoc, 0xEDE3CB62, 46734977146ULL);
    run_test("ZEXALL", zexall, 0xEDE3CB62, 46734977146ULL);

    return 0;
}
