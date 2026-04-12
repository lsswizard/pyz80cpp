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
    , owns_bus(false)
{
    if (!this->bus_ptr) {
        this->bus_ptr = new SimpleBus();
        owns_bus = true;
    }
    reset();

    // Initialize opcode tables
    OpcodeTable::init();
}

Z80::~Z80() {
    if (owns_bus) {
        delete bus_ptr;
    }
}

void Z80::set_bus(Bus* new_bus) {
    if (owns_bus) {
        delete bus_ptr;
    }
    bus_ptr = new_bus;
    owns_bus = false;
    if (!bus_ptr) {
        bus_ptr = new SimpleBus();
        owns_bus = true;
    }
}

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
    current_opcode = 0;
    prefix_ix = false;
    // NOTE: prefix_state removed - was declared in reset() but never read
    // during execution. If re-added, ensure it is actually used.
}

// ============================================================
// Execute one instruction - returns T-states consumed
//
// Interrupt timing reference (Z80 CPU User Manual):
//   NMI response  : 11 T-states  (5 ack M1 + 3+3 push)
//   IM 0 response : 13 T-states  (6 ack M1 + 6 push  + 1 internal)
//   IM 1 response : 13 T-states  (6 ack M1 + 6 push  + 1 internal)
//   IM 2 response : 19 T-states  (6 ack M1 + 6 push  + 3+3 vector reads + 1 internal)
//
// push() is assumed to account for 3+3 = 6 T-states internally.
// Memory reads inside this function each explicitly call wait(3).
// ============================================================
int Z80::step() {
    int start_cycles = total_cycles;

    // --------------------------------------------------------
    // Non-maskable interrupt (highest priority)
    // NMI: 5 (M1 ack) + 6 (push) = 11 T-states
    // --------------------------------------------------------
    if (nmi_pending) {
        nmi_pending = false;
        halted = false;
        regs.IFF1 = false;          // IFF2 is preserved per Z80 spec
        wait(5);                    // NMI M1 acknowledge cycle
        push(regs.PC);              // push() accounts for 3+3 = 6 T-states
        regs.PC = 0x0066;
        regs.MEMPTR = 0x0066;
        return total_cycles - start_cycles;
    }

    // --------------------------------------------------------
    // Maskable interrupt
    // EI_JUST_RESOLVED suppresses interrupt for the one instruction
    // immediately following EI (the "EI then immediately HALT" pattern).
    // --------------------------------------------------------
    if (interrupt_pending && regs.IFF1 && !regs.EI_JUST_RESOLVED) {
        interrupt_pending = false;
        regs.IFF1 = false;
        regs.IFF2 = false;
        halted = false;

        uint8_t vector = bus_ptr ? bus_ptr->interrupt_acknowledge() : 0xFF;
        wait(6);    // Interrupt acknowledge M1 cycle (4 clocks + 2 extra wait states)

        switch (regs.IM) {
            case 0:
                // Mode 0: execute RST instruction placed on data bus.
                // For a RST n opcode the full cost is 13 T-states.
                // wait(6) done above + push(6) + wait(1) internal = 13.
                push(regs.PC);
                regs.PC = (uint16_t)(vector & 0x38);
                regs.MEMPTR = regs.PC;
                wait(1);
                break;

            case 1:
                // Mode 1: unconditional jump to 0x0038.
                // wait(6) + push(6) + wait(1) = 13 T-states.
                push(regs.PC);
                regs.PC = 0x0038;
                regs.MEMPTR = 0x0038;
                wait(1);
                break;

            case 2: {
                // Mode 2: vectored interrupt via I register.
                // wait(6) + push(6) + wait(3) + wait(3) + wait(1) = 19 T-states.
                push(regs.PC);

                uint16_t vector_addr = ((uint16_t)regs.I << 8) | (vector & 0xFE);
                uint16_t vector_addr_hi = (vector_addr + 1) & 0xFFFF;

                uint8_t lo = bus_ptr->read(vector_addr);
                wait(3);    // 3 T-states for low byte read
                uint8_t hi = bus_ptr->read(vector_addr_hi);
                wait(3);    // 3 T-states for high byte read

                regs.PC = (uint16_t)((hi << 8) | lo);
                regs.MEMPTR = regs.PC;
                wait(1);    // 1 internal T-state
                break;
            }

            default:
                // Undefined IM - treat as IM 1 per hardware behaviour
                push(regs.PC);
                regs.PC = 0x0038;
                regs.MEMPTR = 0x0038;
                wait(1);
                break;
        }

        // Interrupt service is complete; do NOT fall through to execute_instruction().
        return total_cycles - start_cycles;
    }

    // --------------------------------------------------------
    // HALT: re-execute NOP (4 T-states) until an interrupt occurs
    // --------------------------------------------------------
    if (halted) {
        wait(4);
        return 4;
    }

    // --------------------------------------------------------
    // Normal instruction execution
    // --------------------------------------------------------
    execute_instruction();
    return total_cycles - start_cycles;
}

// ============================================================
// Execute single instruction
// ============================================================
void Z80::execute_instruction() {
    // Fetch opcode (M1 cycle: 4 T-states including RFSH)
    uint8_t opcode = fetch_opcode();
    current_opcode = opcode;

    if (opcode == 0xCB) {
        // CB prefix: rotate/shift/bit operations (4+4 = 8 T-states for two fetches)
        opcode = fetch_opcode();
        current_opcode = opcode;
        const Instruction& inst = OpcodeTable::get_cb(opcode);
        if (inst.exec) inst.exec(*this);

    } else if (opcode == 0xED) {
        // ED prefix: extended instructions (4+4 = 8 T-states for two fetches)
        opcode = fetch_opcode();
        current_opcode = opcode;
        const Instruction& inst = OpcodeTable::get_ed(opcode);
        if (inst.exec) inst.exec(*this);

    } else if (opcode == 0xDD) {
        // DD prefix: IX-indexed instructions
        prefix_ix = true;
        opcode = fetch_opcode();

        if (opcode == 0xCB) {
            // DDCB: displacement and real opcode follow the prefix pair.
            // Both are fetched here so handlers do not double-read them.
            ddcb_displacement = (int8_t)fetch_byte();
            ddcb_opcode       = fetch_byte();
            current_opcode    = ddcb_opcode;
            const Instruction& inst = OpcodeTable::get_ddcb(ddcb_opcode);
            if (inst.exec) inst.exec(*this);
        } else {
            current_opcode = opcode;
            const Instruction& inst = OpcodeTable::get_dd(opcode);
            if (inst.exec) {
                inst.exec(*this);
            } else {
                // Unrecognised DD opcode: fall through to main table
                const Instruction& main_inst = OpcodeTable::get_main(opcode);
                if (main_inst.exec) main_inst.exec(*this);
            }
        }
        prefix_ix = false;

    } else if (opcode == 0xFD) {
        // FD prefix: IY-indexed instructions
        prefix_ix = false;  // IY path; handlers check prefix_ix to select IX vs IY
        opcode = fetch_opcode();

        if (opcode == 0xCB) {
            ddcb_displacement = (int8_t)fetch_byte();
            ddcb_opcode       = fetch_byte();
            current_opcode    = ddcb_opcode;
            const Instruction& inst = OpcodeTable::get_fdcb(ddcb_opcode);
            if (inst.exec) inst.exec(*this);
        } else {
            current_opcode = opcode;
            const Instruction& inst = OpcodeTable::get_fd(opcode);
            if (inst.exec) {
                inst.exec(*this);
            } else {
                // Unrecognised FD opcode: fall through to main table
                const Instruction& main_inst = OpcodeTable::get_main(opcode);
                if (main_inst.exec) main_inst.exec(*this);
            }
        }
        prefix_ix = false;

    } else {
        // Normal (unprefixed) opcode
        const Instruction& inst = OpcodeTable::get_main(opcode);
        if (inst.exec) inst.exec(*this);
    }

    // --------------------------------------------------------
    // EI two-phase enable:
    //   Phase 1 (EI executes): handler sets EI_PENDING.
    //   End of that instruction: transition to EI_JUST_RESOLVED, enable IFFs.
    //   Phase 2 (next instruction completes): clear EI_JUST_RESOLVED.
    // This ensures interrupts cannot fire until AFTER the instruction
    // following EI, matching real Z80 behaviour.
    // --------------------------------------------------------
    if (regs.EI_JUST_RESOLVED) {
        regs.EI_JUST_RESOLVED = false;
    } else if (regs.EI_PENDING) {
        regs.EI_PENDING       = false;
        regs.EI_JUST_RESOLVED = true;
        regs.IFF1 = regs.IFF2 = true;
    }

    instruction_count++;
}

// ============================================================
// Run for at most max_cycles T-states
// ============================================================
int Z80::run(int max_cycles) {
    int start_cycles = total_cycles;
    while ((total_cycles - start_cycles) < max_cycles) {
        step();
    }
    return total_cycles - start_cycles;
}

// ============================================================
// Run exactly count instructions (stops early if HALT)
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
    interrupt_data = data;
    // Only latch the pending flag; IFF1 is checked in step()
    interrupt_pending = true;
}

void Z80::trigger_nmi() {
    nmi_pending = true;
}

// ============================================================
// 8-bit register read/write
// reg encoding: 0=B 1=C 2=D 3=E 4=H 5=L 6=(HL) 7=A
// Extended:     8=IXH/IYH  9=IXL/IYL  (with DD/FD prefix active)
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
        case 8: return prefix_ix ? (uint8_t)(regs.IX >> 8) : (uint8_t)(regs.IY >> 8);   // IXH / IYH
        case 9: return prefix_ix ? (uint8_t)(regs.IX & 0xFF) : (uint8_t)(regs.IY & 0xFF); // IXL / IYL
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
        case 8:  // IXH / IYH
            if (prefix_ix) regs.IX = (regs.IX & 0x00FF) | ((uint16_t)value << 8);
            else            regs.IY = (regs.IY & 0x00FF) | ((uint16_t)value << 8);
            break;
        case 9:  // IXL / IYL
            if (prefix_ix) regs.IX = (regs.IX & 0xFF00) | value;
            else            regs.IY = (regs.IY & 0xFF00) | value;
            break;
        default: break;
    }
}

} // namespace z80
