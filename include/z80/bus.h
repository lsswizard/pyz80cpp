#pragma once

#include <cstdint>
#include <string>
#include <cstring>
#include <vector>

namespace z80 {

// ============================================================
// AccessKind - Machine cycle types for access-type-specific timing
// ============================================================
enum class AccessKind {
    OpcodeFetch,   // M1 cycle - 4 T-states + wait
    MemoryRead,    // M read - 3 T-states + wait
    MemoryWrite,   // M write - 3 T-states + wait
    IORead,       // I/O read - 4 T-states + wait
    IOWrite,      // I/O write - 4 T-states + wait
    InterruptAck   // INT acknowledge - special handling
};

// ============================================================
// FastBus - Uses external memory pointer for direct access
// No Python callback overhead for memory operations
// ============================================================
class FastBus {
public:
    FastBus() : memory_ptr(nullptr), memory_size(0), memory_owner(false) {}

    ~FastBus() {
        if (memory_owner && memory_ptr) {
            delete[] memory_ptr;
        }
    }

    // Set external memory (Python passes pointer)
    void set_memory(uint8_t* ptr, size_t size) {
        if (memory_owner && memory_ptr) {
            delete[] memory_ptr;
        }
        memory_ptr = ptr;
        memory_size = size;
        memory_owner = false;
    }

    // Allocate internal memory
    void allocate_memory(size_t size = 65536) {
        if (memory_owner && memory_ptr) {
            delete[] memory_ptr;
        }
        memory_ptr = new uint8_t[size]();
        memory_size = size;
        memory_owner = true;
    }

    // Direct memory access - no callback overhead
    uint8_t read(uint16_t addr) const {
        return memory_ptr ? memory_ptr[addr] : 0xFF;
    }

    void write(uint16_t addr, uint8_t val) {
        if (memory_ptr) {
            memory_ptr[addr] = val;
        }
    }

    // I/O - these need callbacks (less frequent)
    virtual uint8_t in_(uint16_t port) { (void)port; return 0xFF; }
    virtual void out_(uint16_t port, uint8_t val) { (void)port; (void)val; }

    // Contention (optional)
    virtual void contend(uint16_t addr, int cycles) { (void)addr; (void)cycles; }
    virtual void m1_cycle() {}

    // Memory pointer access
    uint8_t* get_memory_ptr() { return memory_ptr; }
    size_t get_memory_size() const { return memory_size; }

protected:
    uint8_t* memory_ptr;
    size_t memory_size;
    bool memory_owner;
};

// ============================================================
// Bus Interface — machine-independent, timing tables owned by C++
//
// FIX Issue 3: Timing tables are COPIED into owned std::vector storage.
// This prevents use-after-free if Python buffer is garbage collected.
// ============================================================
class Bus {
public:
    virtual ~Bus() = default;

    // Memory
    virtual uint8_t read(uint16_t addr) = 0;
    virtual void    write(uint16_t addr, uint8_t val) = 0;

    // I/O — override in_() / out_() (not in() / out())
    virtual uint8_t in_(uint16_t port)           { (void)port; return 0xFF; }
    virtual void    out_(uint16_t port, uint8_t val) { (void)port; (void)val; }

    // Interrupt acknowledge — return the vector byte placed on the data bus.
    // For IM 0 / IM 2. Typically 0xFF (NOP) or the low byte of the vector.
    virtual uint8_t interrupt_acknowledge() { return 0xFF; }

    // ============================================================
    // EXTRA CYCLES: Access-type-specific timing (FIX for Issue 1)
    //
    // The CPU asks for extra wait states based on access type.
    // This is the primary interface for timing injection.
    // Default: no extra cycles (machine-independent).
    // ============================================================
    virtual int extra_cycles(AccessKind kind, uint16_t addr, int t_state) {
        (void)kind; (void)addr; (void)t_state;
        return 0;  // Default: no wait states
    }

    // ============================================================
    // FAST PATH: Pre-computed timing tables by access type (FIX Issue 3)
    //
    // Each table is indexed by t_state and returns wait states for that
    // access kind. Data is COPIED into owned vectors - no Python lifetime issues.
    //
    // MACHINE-INDEPENDENT: The masks below determine WHICH regions get delays.
    // Different machines can have different memory layouts by setting different masks.
    // ============================================================

    // OWNED storage - data copied from Python, owned by Bus
    std::vector<uint8_t> fetch_delay_table_vec;
    std::vector<uint8_t> mem_read_delay_table_vec;
    std::vector<uint8_t> mem_write_delay_table_vec;
    std::vector<uint8_t> io_read_delay_table_vec;
    std::vector<uint8_t> io_write_delay_table_vec;

    // Pointers to owned data (set after copying from Python)
    uint8_t* fetch_delay_table = nullptr;
    uint8_t* mem_read_delay_table = nullptr;
    uint8_t* mem_write_delay_table = nullptr;
    uint8_t* io_read_delay_table = nullptr;
    uint8_t* io_write_delay_table = nullptr;
    int timing_table_size = 0;

    // MACHINE-SPECIFIC masks (set by Python): which 4KB regions are contended
    // Bit i = 1 means 4KB region starting at i*0x1000 is contended
    // Different machines set these based on their memory layout
    uint16_t fetch_contention_mask = 0;       // Opcode fetch contended regions
    uint16_t mem_read_contention_mask = 0;    // Memory read contended regions
    uint16_t mem_write_contention_mask = 0;   // Memory write contended regions
    uint16_t io_read_contention_mask = 0;     // I/O read contended regions
    uint16_t io_write_contention_mask = 0;    // I/O write contended regions

    // Convenience: Set all timing tables at once
    // Data is COPIED into owned vectors (FIX Issue 3)
    template<typename T>
    void set_timing_tables(
        const T* fetch_tbl, size_t fetch_size,
        const T* mem_read_tbl, size_t mem_read_size,
        const T* mem_write_tbl, size_t mem_write_size,
        const T* io_read_tbl, size_t io_read_size,
        const T* io_write_tbl, size_t io_write_size) {

        // Copy fetch table
        if (fetch_size > 0 && fetch_tbl) {
            fetch_delay_table_vec.assign(fetch_tbl, fetch_tbl + fetch_size);
            fetch_delay_table = fetch_delay_table_vec.data();
        } else {
            fetch_delay_table_vec.clear();
            fetch_delay_table = nullptr;
        }

        // Copy mem read table
        if (mem_read_size > 0 && mem_read_tbl) {
            mem_read_delay_table_vec.assign(mem_read_tbl, mem_read_tbl + mem_read_size);
            mem_read_delay_table = mem_read_delay_table_vec.data();
        } else {
            mem_read_delay_table_vec.clear();
            mem_read_delay_table = nullptr;
        }

        // Copy mem write table
        if (mem_write_size > 0 && mem_write_tbl) {
            mem_write_delay_table_vec.assign(mem_write_tbl, mem_write_tbl + mem_write_size);
            mem_write_delay_table = mem_write_delay_table_vec.data();
        } else {
            mem_write_delay_table_vec.clear();
            mem_write_delay_table = nullptr;
        }

        // Copy I/O read table
        if (io_read_size > 0 && io_read_tbl) {
            io_read_delay_table_vec.assign(io_read_tbl, io_read_tbl + io_read_size);
            io_read_delay_table = io_read_delay_table_vec.data();
        } else {
            io_read_delay_table_vec.clear();
            io_read_delay_table = nullptr;
        }

        // Copy I/O write table
        if (io_write_size > 0 && io_write_tbl) {
            io_write_delay_table_vec.assign(io_write_tbl, io_write_tbl + io_write_size);
            io_write_delay_table = io_write_delay_table_vec.data();
        } else {
            io_write_delay_table_vec.clear();
            io_write_delay_table = nullptr;
        }

        // Set table size (use largest size)
        timing_table_size = 0;
        if (fetch_size > timing_table_size) timing_table_size = fetch_size;
        if (mem_read_size > timing_table_size) timing_table_size = mem_read_size;
        if (mem_write_size > timing_table_size) timing_table_size = mem_write_size;
        if (io_read_size > timing_table_size) timing_table_size = io_read_size;
        if (io_write_size > timing_table_size) timing_table_size = io_write_size;
    }

    // Clear all timing tables
    void clear_timing_tables() {
        fetch_delay_table_vec.clear();
        mem_read_delay_table_vec.clear();
        mem_write_delay_table_vec.clear();
        io_read_delay_table_vec.clear();
        io_write_delay_table_vec.clear();

        fetch_delay_table = nullptr;
        mem_read_delay_table = nullptr;
        mem_write_delay_table = nullptr;
        io_read_delay_table = nullptr;
        io_write_delay_table = nullptr;
        timing_table_size = 0;
    }

    // ============================================================
    // FAST PATH: Direct memory access pointer (64KB)
    // ============================================================
    uint8_t* fast_memory_ptr = nullptr;

    // ============================================================
    // FAST PATH: Flags to bypass virtual calls
    // ============================================================
    bool bypass_contend = true;
    bool bypass_m1 = true;
    bool bypass_io_wait = false;

    // ============================================================
    // FAST PATH: Direct I/O read access (256 ports)
    // ============================================================
    uint8_t* fast_io_read_ptr = nullptr;
    bool     fast_io_read_active[256] = {false};

    // Optional debug hook
    virtual std::string debug_info() const { return {}; }
};

// ============================================================
// SimpleBus — 64 KB flat memory, 256-byte I/O space
// ============================================================
class SimpleBus : public Bus {
public:
    uint8_t memory[65536] = {};
    uint8_t io_ports[256] = {};

    uint8_t read(uint16_t addr) override           { return memory[addr]; }
    void    write(uint16_t addr, uint8_t val) override { memory[addr] = val; }
    uint8_t in_(uint16_t port) override            { return io_ports[port & 0xFF]; }
    void    out_(uint16_t port, uint8_t val) override  { io_ports[port & 0xFF] = val; }
};

} // namespace z80
