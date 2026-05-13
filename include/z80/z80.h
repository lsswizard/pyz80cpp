#pragma once

#include "bus.h"
#include "registers.h"
#include "flags.h"
#include <unordered_map>

namespace z80 {

class Z80;
using OpHandler = void(*)(Z80&);

// ============================================================
// Instruction descriptor
// ============================================================
struct Instruction {
    OpHandler exec        = nullptr;
    uint8_t   base_cycles = 4;
    uint8_t   length      = 1;
    bool      affects_flags = false;

    Instruction() = default;
    Instruction(OpHandler e, uint8_t c, uint8_t l, bool f)
        : exec(e), base_cycles(c), length(l), affects_flags(f) {}
};

// ============================================================
// Z80 CPU — machine-independent, cycle-accurate
//
// FIX Issues 1 & 2:
// - Access-type-specific timing via delay table lookup
// - cpu.step() returns ACTUAL elapsed cycles (base + wait)
// ============================================================
class Z80 {
public:
    explicit Z80(Bus* bus = nullptr);
    ~Z80();

    void reset();

    // Execute one instruction; returns T-states consumed INCLUDING wait states
    // FIX Issue 2: Returns ACTUAL elapsed cycles, not just base cycles
    int step();

    // Run for at most max_cycles T-states
    int run(int max_cycles);

    // Run exactly count instructions (stops early on HALT)
    int run_instructions(int count);

    // --------------------------------------------------------
    // Interrupt interface
    // --------------------------------------------------------
    void trigger_interrupt(uint8_t data = 0xFF);
    void trigger_nmi();

    bool is_halted()            const { return halted; }
    bool has_pending_interrupt()const { return interrupt_pending; }
    bool has_pending_nmi()      const { return nmi_pending; }

    // --------------------------------------------------------
    // Register access
    // --------------------------------------------------------
    Registers&       get_registers()       { return regs; }
    const Registers& get_registers() const { return regs; }

    void    set_state(const std::unordered_map<std::string, int>& state);
    uint8_t read_reg8(int reg);
    void    write_reg8(int reg, uint8_t value);

    // --------------------------------------------------------
    // Cycle accounting
    // --------------------------------------------------------
    void add_cycles(int n) { total_cycles += n; }
    int  get_cycles()            const { return total_cycles; }
    int  get_instruction_count() const { return instruction_count; }

    // --------------------------------------------------------
    // Bus accessors — called by instruction handlers
    //
    // FIX Issues 1 & 2:
    // - Use access-type-specific delay tables
    // - Return total cycles (base + wait) for proper hardware sync
    // --------------------------------------------------------

    // M1 fetch: 4 T-states + wait states from fetch_delay_table
    inline uint8_t fetch_opcode() {
        regs.R = (regs.R & 0x80) | ((regs.R + 1) & 0x7F);

        uint8_t val;
        if (bus_ptr->fast_memory_ptr) {
            val = bus_ptr->fast_memory_ptr[regs.PC];
        } else {
            val = bus_ptr->read(regs.PC);
        }

        regs.PC = (regs.PC + 1) & 0xFFFF;

        // Base M1 cycle (4T) + access-type-specific wait states
        int base = 4;
        int wait = get_extra_cycles(AccessKind::OpcodeFetch, regs.PC - 1, t_state);

        int total = base + wait;
        add_cycles(total);
        t_state += total;
        return val;
    }

    // Operand/displacement byte fetch: 3 T-states + wait from mem_read_delay_table
    inline uint8_t fetch_byte() {
        uint8_t val;
        if (bus_ptr->fast_memory_ptr) {
            val = bus_ptr->fast_memory_ptr[regs.PC++];
        } else {
            val = bus_ptr->read(regs.PC++);
        }

        // Memory read timing
        int base = 3;
        int wait = get_extra_cycles(AccessKind::MemoryRead, regs.PC - 1, t_state);

        int total = base + wait;
        add_cycles(total);
        t_state += total;
        return val;
    }

    // Memory read: 3 T-states + wait from mem_read_delay_table
    inline uint8_t read(uint16_t addr) {
        uint8_t val;
        if (bus_ptr->fast_memory_ptr) {
            val = bus_ptr->fast_memory_ptr[addr];
        } else {
            val = bus_ptr->read(addr);
        }

        // Memory read timing
        int base = 3;
        int wait = get_extra_cycles(AccessKind::MemoryRead, addr, t_state);

        int total = base + wait;
        add_cycles(total);
        t_state += total;
        return val;
    }

    // Memory write: 3 T-states + wait from mem_write_delay_table
    //
    // MACHINE-INDEPENDENT CONTRACT:
    //   The bus_ptr->write() callback is the SINGLE authoritative path for writes.
    //   Python-side write() handles all bank mapping, paging, and unified buffer
    //   updates. DO NOT write directly to fast_memory_ptr here — doing so would:
    //     (a) bypass ROM write protection (corrupting ROM in unified buffer), and
    //     (b) bypass bank mapping logic for 128K/+3.
    //
    //   The fast_memory_ptr is READ-ONLY from C++ for the fast read path.
    //   Writes propagate correctly because Python-side updates _unified
    //   (which fast_memory_ptr points to) whenever a RAM bank is written.
    inline void write(uint16_t addr, uint8_t val) {
        bus_ptr->write(addr, val);

        // Memory write timing
        int base = 3;
        int wait = get_extra_cycles(AccessKind::MemoryWrite, addr, t_state);

        int total = base + wait;
        add_cycles(total);
        t_state += total;
    }

    // I/O read: 4 T-states + wait from io_read_delay_table
    inline uint8_t in(uint16_t port) {
        int base = 4;
        int wait = get_extra_cycles(AccessKind::IORead, port, t_state);

        int total = base + wait;
        add_cycles(total);
        t_state += total;

        // Fast path for I/O reads (avoids Python callback)
        if (bus_ptr->fast_io_read_ptr && bus_ptr->fast_io_read_active[port & 0xFF]) {
            return bus_ptr->fast_io_read_ptr[port & 0xFF];
        }

        return bus_ptr->in_(port);
    }

    // I/O write: 4 T-states + wait from io_write_delay_table
    inline void out(uint16_t port, uint8_t val) {
        bus_ptr->out_(port, val);

        // I/O write timing
        int base = 4;
        int wait = get_extra_cycles(AccessKind::IOWrite, port, t_state);

        int total = base + wait;
        add_cycles(total);
        t_state += total;
    }

    // Add idle cycles (internal delay states)
    inline void wait(int cycles) {
        add_cycles(cycles);
        t_state += cycles;
    }

    // --------------------------------------------------------
    // Stack helpers
    // --------------------------------------------------------
    inline uint16_t pop() {
        uint16_t lo = read(regs.SP++);
        uint16_t hi = read(regs.SP++);
        return lo | (hi << 8);
    }

    inline void push(uint16_t val) {
        write(--regs.SP, val >> 8);
        write(--regs.SP, val & 0xFF);
    }

    // --------------------------------------------------------
    // Condition check
    // --------------------------------------------------------
    inline bool check_condition(int cc) const {
        return z80::check_condition(regs.F, cc);
    }

    // --------------------------------------------------------
    // Bus management
    // --------------------------------------------------------
    void set_bus(Bus* new_bus);

    // --------------------------------------------------------
    // Public state — accessible to handlers
    // --------------------------------------------------------
    Registers regs;
    uint8_t   current_opcode = 0;
    bool      prefix_ix = false;   // true = DD prefix (IX), false = FD prefix (IY)
    Bus*      bus_ptr   = nullptr;
    int       total_cycles     = 0;     // Total T-states since reset (cumulative)
    int       t_state          = 0;     // Current T-state within frame (0 to tstates_per_frame-1)
    int       instruction_count = 0;
    bool      halted           = false;
    bool      interrupt_pending = false;
    bool      nmi_pending       = false;
    uint8_t   interrupt_data    = 0xFF;
    uint16_t  trap_address     = 0xFFFF; // If PC == trap_address, run() returns early

    // Pre-fetched values for DDCB/FDCB instructions (avoids double-read)
    int8_t  ddcb_displacement = 0;
    uint8_t ddcb_opcode       = 0;

private:
    bool owns_bus = false;

    void execute_instruction();

    // Get extra cycles from timing table or virtual call (FIX Issues 1 & 2)
    // Machine-independent: C++ uses contention masks to determine if delays apply.
    // Python sets the masks based on machine memory layout.
    inline int get_extra_cycles(AccessKind kind, uint16_t addr, int ts) const {
        uint8_t* table = nullptr;
        uint16_t* pMask = nullptr;
        int table_size = bus_ptr->timing_table_size;

        switch (kind) {
            case AccessKind::OpcodeFetch:
                table = bus_ptr->fetch_delay_table;
                pMask = &bus_ptr->fetch_contention_mask;
                break;
            case AccessKind::MemoryRead:
                table = bus_ptr->mem_read_delay_table;
                pMask = &bus_ptr->mem_read_contention_mask;
                break;
            case AccessKind::MemoryWrite:
                table = bus_ptr->mem_write_delay_table;
                pMask = &bus_ptr->mem_write_contention_mask;
                break;
            case AccessKind::IORead:
                table = bus_ptr->io_read_delay_table;
                pMask = &bus_ptr->io_read_contention_mask;
                break;
            case AccessKind::IOWrite:
                table = bus_ptr->io_write_delay_table;
                pMask = &bus_ptr->io_write_contention_mask;
                break;
            case AccessKind::InterruptAck:
                // Interrupt acknowledge has special timing - use memory read
                table = bus_ptr->mem_read_delay_table;
                pMask = &bus_ptr->mem_read_contention_mask;
                break;
        }

        if (table && table_size > 0 && pMask != nullptr) {
            // Check if this address region is marked as contended
            // addr >> 12 gives the 4KB region (0-15 for 64KB address space)
            uint16_t mask = *pMask;
            uint16_t region = addr >> 12;
            if ((mask & (1 << region)) == 0) {
                // This region is not contended for this access type
                return 0;
            }
            // Table lookup - machine-specific timing already baked in
            int idx = ts % table_size;
            return table[idx];
        }

        // Fallback to virtual call (Python implements machine-specific timing)
        return bus_ptr->extra_cycles(kind, addr, ts);
    }
};

} // namespace z80
