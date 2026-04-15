#pragma once

#include "bus.h"
#include "registers.h"
#include "flags.h"
#include <functional>
#include <memory>

namespace z80 {

// ============================================================
// Instruction execute function signature
// ============================================================
class Z80;
using OpHandler = void(*)(Z80&);

// ============================================================
// Instruction descriptor
// ============================================================
struct Instruction {
    OpHandler exec = nullptr;
    uint8_t base_cycles = 4;
    uint8_t length = 1;
    bool affects_flags = false;

    // Default constructor
    Instruction() = default;
    
    // Constructor with values
    Instruction(OpHandler e, uint8_t c, uint8_t l, bool f) 
        : exec(e), base_cycles(c), length(l), affects_flags(f) {}
};

// ============================================================
// Z80 CPU Class
// ============================================================
class Z80 {
public:
    // Constructor
    explicit Z80(Bus* bus = nullptr);
    ~Z80();

    // Reset the CPU
    void reset();

    // ============================================================
    // Execution - returns T-states consumed
    // ============================================================
    int step();                    // Execute one instruction, return cycles
    int run(int max_cycles);      // Run until max_cycles consumed
    int run_instructions(int count); // Run exactly N instructions

    // ============================================================
    // Interrupt handling
    // ============================================================
    void trigger_interrupt(uint8_t data);   // Request interrupt with data
    void trigger_nmi();                       // Request NMI
    bool is_halted() const { return halted; }
    bool has_pending_interrupt() const { return interrupt_pending; }
    bool has_pending_nmi() const { return nmi_pending; }

    // ============================================================
    // Register access
    // ============================================================
    Registers& get_registers() { return regs; }
    const Registers& get_registers() const { return regs; }

    uint8_t read_reg8(int reg);      // reg: 0-7 (B,C,D,E,H,L,(HL),A)
    void write_reg8(int reg, uint8_t value);

    // ============================================================
    // Cycle management
    // ============================================================
    void add_cycles(int cycles) { total_cycles += cycles; }
    int get_cycles() const { return total_cycles; }
    int get_instruction_count() const { return instruction_count; }

    // ============================================================
    // Bus access (cycle-accurate) - PUBLIC for handlers
    // ============================================================
    
    // M1 cycle - instruction fetch (4 T-states, increments R)
    inline uint8_t fetch_opcode() {
        bus_ptr->m1_cycle();
        regs.R = (regs.R & 0x80) | ((regs.R + 1) & 0x7F);
        uint8_t val = bus_ptr->read(regs.PC);
        regs.PC = (regs.PC + 1) & 0xFFFF;
        add_cycles(4);
        return val;
    }

    // Fetch operand byte (3 T-states, no R increment)
    inline uint8_t fetch_byte() {
        return read(regs.PC++);
    }

    // Memory read (3 T-states + wait states)
    inline uint8_t read(uint16_t addr) {
        int wait_states = bus_ptr->get_memory_wait_states(addr);
        add_cycles(3 + wait_states);
        bus_ptr->contend(addr, 3);
        return bus_ptr->read(addr);
    }

    // Memory write (3 T-states + wait states)
    inline void write(uint16_t addr, uint8_t value) {
        int wait_states = bus_ptr->get_memory_wait_states(addr);
        add_cycles(3 + wait_states);
        bus_ptr->contend(addr, 3);
        bus_ptr->write(addr, value);
    }

    // I/O read (4 T-states + wait states)
    inline uint8_t in(uint16_t port) {
        int wait_states = bus_ptr->get_io_wait_states(port);
        add_cycles(4 + wait_states);
        return bus_ptr->in(port);
    }

    // I/O write (4 T-states + wait states)
    inline void out(uint16_t port, uint8_t value) {
        int wait_states = bus_ptr->get_io_wait_states(port);
        add_cycles(4 + wait_states);
        bus_ptr->out(port, value);
    }

    // Wait (add cycles without bus access)
    inline void wait(int cycles) {
        add_cycles(cycles);
    }

    // ============================================================
    // Stack operations
    // ============================================================
    inline uint16_t pop() {
        uint16_t lo = read(regs.SP++);
        uint16_t hi = read(regs.SP++);
        return lo | (hi << 8);
    }

    inline void push(uint16_t value) {
        write(--regs.SP, (value >> 8) & 0xFF);
        write(--regs.SP, value & 0xFF);
    }

    // ============================================================
    // Condition checking
    // ============================================================
    inline bool check_condition(int cc) const {
        return z80::check_condition(regs.F, cc);
    }

    // ============================================================
    // Set bus (can be changed at runtime)
    // ============================================================
    void set_bus(Bus* new_bus);

    // ============================================================
    // PUBLIC members - accessible to handlers
    // ============================================================
    Registers regs;
    uint8_t current_opcode;
    bool prefix_ix;
    Bus* bus_ptr;
    int total_cycles;
    int instruction_count;
    bool halted;
    bool interrupt_pending;
    bool nmi_pending;
    uint8_t interrupt_data;
    // DDCB/FDCB pre-fetched values (to avoid double-read)
    int8_t ddcb_displacement;
    uint8_t ddcb_opcode;

private:
    bool owns_bus;
    // Internal execution
    void execute_instruction();
    int handle_prefix(uint8_t opcode);
};

} // namespace z80