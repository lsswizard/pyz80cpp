#pragma once

#include <cstdint>
#include <string>

namespace z80 {

// ============================================================
// Cycle Types - used for timing and contention
// ============================================================
enum class CycleType {
    M1,           // Instruction fetch
    MEM_RD,       // Memory read
    MEM_WR,       // Memory write
    IO_RD,        // I/O read
    IO_WR,        // I/O write
    INT_ACK       // Interrupt acknowledge
};

// ============================================================
// Bus Interface - pure virtual, machine-independent
// ============================================================
class Bus {
public:
    virtual ~Bus() = default;

    // Memory operations
    virtual uint8_t read(uint16_t addr) = 0;
    virtual void write(uint16_t addr, uint8_t val) = 0;

    // I/O operations
    virtual uint8_t in(uint16_t port) = 0;
    virtual void out(uint16_t port, uint8_t val) = 0;

    // Contention simulation - CRITICAL for accurate timing
    // Called by CPU when accessing memory that may be contended
    virtual void contend(uint16_t addr, int cycles) {
        (void)addr;
        (void)cycles;
        // Default: no contention
    }

    // M1 cycle hook - called during instruction fetch
    // Used for R register increment
    virtual void m1_cycle() {}

    // Interrupt acknowledge - called when CPU accepts interrupt
    virtual uint8_t interrupt_acknowledge() {
        return 0xFF;  // Default: return NOP vector
    }

    // Wait state configuration (for accurate timing)
    // Returns additional wait states for memory/I/O access
    virtual int get_memory_wait_states(uint16_t addr) {
        (void)addr;
        return 0;  // Default: no wait states
    }
    
    virtual int get_io_wait_states(uint16_t port) {
        (void)port;
        return 0;  // Default: no wait states
    }

    // Optional: debug info
    virtual std::string debug_info() const { return ""; }
};

// ============================================================
// SimpleBus - basic implementation without contention
// ============================================================
class SimpleBus : public Bus {
public:
    uint8_t memory[65536] = {};
    uint8_t io_ports[256] = {};

    uint8_t read(uint16_t addr) override {
        return memory[addr & 0xFFFF];
    }

    void write(uint16_t addr, uint8_t val) override {
        memory[addr & 0xFFFF] = val;
    }

    uint8_t in(uint16_t port) override {
        return io_ports[port & 0xFF];
    }

    void out(uint16_t port, uint8_t val) override {
        io_ports[port & 0xFF] = val;
    }
};

} // namespace z80