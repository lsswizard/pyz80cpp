#include "../include/z80/z80.h"
#include "../include/z80/opcode_table.h"
#include "../include/z80/decoder.h"
#include <cstring>
#include <algorithm>

namespace z80 {

// ============================================================
// Constructor / Destructor
// ============================================================
Z80::Z80(Bus* bus_ptr)
    : bus_ptr(bus_ptr)
    , use_direct_memory(bus_ptr == nullptr)
{
    reset();
    memset(direct_memory, 0, sizeof(direct_memory));
    memset(direct_io, 0, sizeof(direct_io));
    
    // Initialize opcode tables
    OpcodeTable::init();
}

Z80::~Z80() = default;

// ============================================================
// Reset
// ============================================================
void Z80::reset() {
    regs.reset();
    total_cycles = 0;
    instruction_count = 0;
    halted = false;
    interrupt_pending = false;
    nmi_pending = false;
    interrupt_data = 0;
    prefix_state = PrefixState::NONE;
    current_opcode = 0;
    prefix_ix = false;
}

// ============================================================
// Execute one instruction - returns T-states
// ============================================================
int Z80::step() {
    // Check for NMI
    if (nmi_pending) {
        nmi_pending = false;
        // NMI sequence: 2 cycles for push, then jump to 0x0066
        regs.IFF1 = false;
        push(regs.PC);
        regs.PC = 0x0066;
        return 2;  // Actually more, but handled in execution
    }

    // Check for interrupt
    if (interrupt_pending && regs.IFF1 && !halted) {
        // Interrupt accepted after current instruction
        interrupt_pending = false;
        regs.IFF1 = false;
        regs.IFF2 = false;

        // Interrupt acknowledge cycle
        uint8_t vector = bus_ptr ? bus_ptr->interrupt_acknowledge() : 0xFF;

        // Handle different interrupt modes
        switch (regs.IM) {
            case 0:  // Mode 0: execute instruction from data
                // For Z80, this is essentially a RST 0-7
                // Simplified: push PC and jump to vector
                push(regs.PC);
                regs.PC = (uint16_t)vector * 8;
                break;
            case 1:  // Mode 1: jump to 0x0038
                push(regs.PC);
                regs.PC = 0x0038;
                break;
            case 2:  // Mode 2: vectored interrupt
                push(regs.PC);
                // Read vector from (I << 8) | data
                uint16_t vector_addr = ((uint16_t)regs.I << 8) | vector;
                uint8_t lo = bus_ptr ? bus_ptr->read(vector_addr) : direct_memory[vector_addr];
                uint8_t hi = bus_ptr ? bus_ptr->read((vector_addr + 1) & 0xFFFF) : direct_memory[(vector_addr + 1) & 0xFFFF];
                regs.PC = (hi << 8) | lo;
                break;
        }
    }

    // Handle halted state
    if (halted) {
        wait(4);  // NOP while halted
        return 4;
    }

    // Execute instruction
    return execute_instruction();
}

// ============================================================
// Execute single instruction
// ============================================================
int Z80::execute_instruction() {
    // Fetch opcode (M1 cycle)
    uint8_t opcode = fetch();
    current_opcode = opcode;
    int cycles = 4;  // Already added by fetch()

    // Handle prefix opcodes
    if (opcode == 0xCB) {
        // CB prefix
        opcode = fetch();
        const Instruction& inst = OpcodeTable::get_cb(opcode);
        if (inst.exec) {
            inst.exec(*this);
            cycles += inst.base_cycles - 4;  // Subtract fetch cycles
        }
    } else if (opcode == 0xED) {
        // ED prefix
        opcode = fetch();
        const Instruction& inst = OpcodeTable::get_ed(opcode);
        if (inst.exec) {
            inst.exec(*this);
            cycles += inst.base_cycles - 4;
        }
    } else if (opcode == 0xDD) {
        // DD prefix (IX)
        prefix_ix = true;
        opcode = fetch();
        if (opcode == 0xCB) {
            // DDCB prefix
            uint8_t d = fetch();  // Displacement
            (void)d;  // Unused for now
            opcode = fetch();
            const Instruction& inst = OpcodeTable::get_ddcb(opcode);
            if (inst.exec) {
                inst.exec(*this);
                cycles += inst.base_cycles - 4;
            }
        } else {
            // DD regular opcode
            const Instruction& inst = OpcodeTable::get_dd(opcode);
            if (inst.exec) {
                inst.exec(*this);
                cycles += inst.base_cycles - 4;
            }
        }
        prefix_ix = false;
    } else if (opcode == 0xFD) {
        // FD prefix (IY)
        prefix_ix = false;
        opcode = fetch();
        if (opcode == 0xCB) {
            // FDCB prefix
            uint8_t d = fetch();  // Displacement
            (void)d;
            opcode = fetch();
            const Instruction& inst = OpcodeTable::get_fdcb(opcode);
            if (inst.exec) {
                inst.exec(*this);
                cycles += inst.base_cycles - 4;
            }
        } else {
            // FD regular opcode
            const Instruction& inst = OpcodeTable::get_fd(opcode);
            if (inst.exec) {
                inst.exec(*this);
                cycles += inst.base_cycles - 4;
            }
        }
        prefix_ix = false;
    } else {
        // Normal opcode
        const Instruction& inst = OpcodeTable::get_main(opcode);
        if (inst.exec) {
            inst.exec(*this);
            cycles += inst.base_cycles - 4;
        }
    }

    // Handle EI pending state
    if (regs.EI_PENDING && !regs.EI_JUST_RESOLVED) {
        regs.EI_JUST_RESOLVED = true;
        regs.EI_PENDING = false;
        regs.IFF1 = regs.IFF2 = true;
    } else if (regs.EI_JUST_RESOLVED) {
        regs.EI_JUST_RESOLVED = false;
    }

    instruction_count++;
    return cycles;
}

// ============================================================
// Run for specified number of cycles
// ============================================================
int Z80::run(int max_cycles) {
    int start_cycles = total_cycles;
    while (total_cycles - start_cycles < max_cycles) {
        int c = step();
        (void)c;  // Already added to total_cycles
    }
    return total_cycles - start_cycles;
}

// ============================================================
// Run for specified number of instructions
// ============================================================
int Z80::run_instructions(int count) {
    int start_count = instruction_count;
    for (int i = 0; i < count && !halted; i++) {
        step();
    }
    return instruction_count - start_count;
}

// ============================================================
// Interrupt handling
// ============================================================
void Z80::trigger_interrupt(uint8_t data) {
    if (regs.IFF1) {
        interrupt_data = data;
        interrupt_pending = true;
    }
}

void Z80::trigger_nmi() {
    nmi_pending = true;
}

// ============================================================
// Register access
// ============================================================
uint8_t Z80::read_reg8(int reg) {
    switch (reg) {
        case 0: return regs.B;
        case 1: return regs.C;
        case 2: return regs.D;
        case 3: return regs.E;
        case 4: return regs.H;
        case 5: return regs.L;
        case 6: return read(regs.HL());
        case 7: return regs.A;
        default: return 0;
    }
}

void Z80::write_reg8(int reg, uint8_t value) {
    switch (reg) {
        case 0: regs.B = value; break;
        case 1: regs.C = value; break;
        case 2: regs.D = value; break;
        case 3: regs.E = value; break;
        case 4: regs.H = value; break;
        case 5: regs.L = value; break;
        case 6: write(regs.HL(), value); break;
        case 7: regs.A = value; break;
    }
}

} // namespace z80