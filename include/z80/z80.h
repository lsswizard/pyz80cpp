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
// Z80 CPU
// ============================================================
class Z80 {
public:
    explicit Z80(Bus* bus_ptr = nullptr);
    ~Z80();

    void reset();

    // Execute one instruction; returns T-states consumed
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
    // --------------------------------------------------------

    // M1 fetch: 4 T-states, increments R (7-bit counter, bit 7 preserved)
    inline uint8_t fetch_opcode() {
        bus_ptr->m1_cycle();
        regs.R = (regs.R & 0x80) | ((regs.R + 1) & 0x7F);
        uint8_t val = bus_ptr->read(regs.PC);
        regs.PC = (regs.PC + 1) & 0xFFFF;
        add_cycles(4);
        return val;
    }

    // Operand/displacement byte fetch: 3 T-states, does NOT increment R
    inline uint8_t fetch_byte() {
        return read(regs.PC++);
    }

    // Memory read: 3 T-states + machine wait states
    inline uint8_t read(uint16_t addr) {
        add_cycles(3 + bus_ptr->get_memory_wait_states(addr));
        bus_ptr->contend(addr, 3);
        return bus_ptr->read(addr);
    }

    // Memory write: 3 T-states + machine wait states
    inline void write(uint16_t addr, uint8_t val) {
        add_cycles(3 + bus_ptr->get_memory_wait_states(addr));
        bus_ptr->contend(addr, 3);
        bus_ptr->write(addr, val);
    }

    // I/O read: 4 T-states + machine wait states
    inline uint8_t in(uint16_t port) {
        add_cycles(4 + bus_ptr->get_io_wait_states(port));
        return bus_ptr->in_(port);
    }

    // I/O write: 4 T-states + machine wait states
    inline void out(uint16_t port, uint8_t val) {
        add_cycles(4 + bus_ptr->get_io_wait_states(port));
        bus_ptr->out_(port, val);
    }

    // Add idle cycles (internal delay states)
    inline void wait(int cycles) { add_cycles(cycles); }

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
    int       total_cycles     = 0;
    int       instruction_count = 0;
    bool      halted           = false;
    bool      interrupt_pending = false;
    bool      nmi_pending       = false;
    uint8_t   interrupt_data    = 0xFF;


    // Pre-fetched values for DDCB/FDCB instructions (avoids double-read)
    int8_t  ddcb_displacement = 0;
    uint8_t ddcb_opcode       = 0;

private:
    bool owns_bus = false;

    void execute_instruction();
};

} // namespace z80
