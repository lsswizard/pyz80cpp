#include "cpu.h"
#include "flags.h"
#include <cstring>
#include <cstdio>

CPU::CPU(Bus* b) : bus(b), cycles(0), instruction_count(0), halted(false),
                   interrupt_data(0xFF), interrupt_pending(false), nmi_pending(false),
                   current_opcode(0), _is_iy(false), _mem(nullptr), _is_simple_bus(false),
                   _pc_modified(false), _is_ld_a_ir(false) {
    if (!bus) {
        // Create a default SimpleBus if none provided
        bus = new SimpleBus();
        _owns_bus = true;
    } else {
        _owns_bus = false;
    }
    _mem = nullptr;
    _is_simple_bus = false;

    // Check if bus is SimpleBus for fast path
    // We store the memory pointer directly for fast access
    auto* sb = dynamic_cast<SimpleBus*>(bus);
    if (sb) {
        _mem = sb->memory;
        _is_simple_bus = true;
    }

    z80flags::init_tables();
    build_handler_tables();
    reset();
}

CPU::~CPU() {
    if (_owns_bus) {
        delete bus;
    }
}

void CPU::reset() {
    regs.reset();
    halted = false;
    cycles = 0;
    instruction_count = 0;
    interrupt_pending = false;
    interrupt_data = 0xFF;
    nmi_pending = false;
    _pc_modified = false;
    _is_ld_a_ir = false;
    decoder.invalidate_all();
}

int CPU::step() {
    // Handle NMI (non-maskable interrupt) - takes priority
    if (nmi_pending) {
        nmi_pending = false;
        regs.IFF2 = regs.IFF1;
        regs.IFF1 = false;
        regs.R = (regs.R & 0x80) | ((regs.R + 1) & 0x7F);
        // When halted, return address is PC+1 (past the HALT)
        uint16_t ret_addr = halted ? ((regs.PC + 1) & 0xFFFF) : regs.PC;
        halted = false;
        regs.SP = (regs.SP - 2) & 0xFFFF;
        _bus_write_direct(regs.SP, ret_addr & 0xFF, cycles);
        _bus_write_direct((regs.SP + 1) & 0xFFFF, ret_addr >> 8, cycles);
        regs.PC = 0x0066;
        cycles += 11;
        return 11;
    }

    // EI deferral logic: resolve EI, then handle interrupt on same step
    if (regs.EI_PENDING) {
        regs.EI_PENDING = false;
        regs.EI_JUST_RESOLVED = true;
        regs.IFF1 = true;
        regs.IFF2 = true;
    }

    // Handle maskable interrupt
    if (interrupt_pending && regs.IFF1 && !regs.EI_JUST_RESOLVED) {
        interrupt_pending = false;
        halted = false;
        regs.IFF1 = false;
        regs.IFF2 = false;
        regs.R = (regs.R & 0x80) | ((regs.R + 1) & 0x7F);
        // When halted, return address is PC+1 (past the HALT)
        uint16_t ret_addr = halted ? ((regs.PC + 1) & 0xFFFF) : regs.PC;
        regs.SP = (regs.SP - 2) & 0xFFFF;
        _bus_write_direct(regs.SP, ret_addr & 0xFF, cycles);
        _bus_write_direct((regs.SP + 1) & 0xFFFF, ret_addr >> 8, cycles);

        // LD A,I / LD A,R interrupt bug: clear PV if previous instruction
        // was LD A,I or LD A,R (the IFF2 was cleared before P/V could be set)
        if (_is_ld_a_ir) {
            regs.F &= ~0x04; // Clear P/V flag (bit 2)
        }

        if (regs.IM == 0) {
            // Mode 0: read instruction from bus (simplified: use RST 38h)
            regs.PC = 0x0038;
        } else if (regs.IM == 1) {
            regs.PC = 0x0038;
        } else {
            // Mode 2: vector table lookup
            uint16_t vector_addr = (uint16_t)((regs.I << 8) | (interrupt_data & 0xFE));
            uint8_t lo = _bus_read(vector_addr, cycles);
            uint8_t hi = _bus_read(vector_addr + 1, cycles);
            regs.PC = (uint16_t)(lo | (hi << 8));
        }
        cycles += 13;
        return 13;
    }

    // Clear EI deferral flag AFTER interrupt check, so one instruction
    // executes with EI_JUST_RESOLVED set (preventing interrupt acceptance)
    if (regs.EI_JUST_RESOLVED) {
        regs.EI_JUST_RESOLVED = false;
    }

    if (halted) {
        regs.R = (regs.R & 0x80) | ((regs.R + 1) & 0x7F);
        cycles += 4;
        return 4;
    }

    uint16_t pc = regs.PC;
    uint8_t opcode;
    uint8_t b1, b2, b3;  // Next bytes for prefix handling

    // Fast path: direct memory access for SimpleBus
    if (_is_simple_bus) {
        opcode = _mem[pc];
        b1 = _mem[(pc + 1) & 0xFFFF];
        b2 = _mem[(pc + 2) & 0xFFFF];
        b3 = _mem[(pc + 3) & 0xFFFF];
    } else {
        opcode = bus->bus_read(pc, cycles);
        b1 = bus->bus_read((pc + 1) & 0xFFFF, cycles);
        b2 = bus->bus_read((pc + 2) & 0xFFFF, cycles);
        b3 = bus->bus_read((pc + 3) & 0xFFFF, cycles);
    }

    DecodeSlot slot;
    if (_is_simple_bus) {
        slot = decoder.decode(_mem, pc);
    } else {
        // For non-SimpleBus (e.g., CallbackBus), look up handler directly
        if (opcode == 0xCB) {
            slot = DecodeSlot{cb_handlers[b1].handler, cb_handlers[b1].cycles, 2, true, false};
        } else if (opcode == 0xED) {
            slot = DecodeSlot{ed_handlers[b1].handler, ed_handlers[b1].cycles, 2, ed_handlers[b1].affects_f, false};
        } else if (opcode == 0xDD) {
            if (b1 == 0xCB) {
                slot = DecodeSlot{ddcb_handlers[b3].handler, ddcb_handlers[b3].cycles, 4, ddcb_handlers[b3].affects_f, false};
            } else if (b1 == 0xED) {
                slot = DecodeSlot{dd_ed_handlers[b2].handler, dd_ed_handlers[b2].cycles, dd_ed_handlers[b2].length, dd_ed_handlers[b2].affects_f, false};
            } else if (dd_handlers[b1].handler) {
                slot = DecodeSlot{dd_handlers[b1].handler, dd_handlers[b1].cycles, dd_handlers[b1].length, dd_handlers[b1].affects_f, false};
            } else {
                // DD prefix fallthrough: execute base instruction
                // Temporarily advance PC so READ_PC works correctly
                regs.PC = (pc + 1) & 0xFFFF;
                slot = DecodeSlot{base_handlers[b1].handler, base_handlers[b1].cycles, (uint8_t)(base_handlers[b1].length + 1), base_handlers[b1].affects_f, base_handlers[b1].is_ld_a_ir};
            }
        } else if (opcode == 0xFD) {
            if (b1 == 0xCB) {
                slot = DecodeSlot{fdcb_handlers[b3].handler, fdcb_handlers[b3].cycles, 4, fdcb_handlers[b3].affects_f, false};
            } else if (b1 == 0xED) {
                slot = DecodeSlot{fd_ed_handlers[b2].handler, fd_ed_handlers[b2].cycles, fd_ed_handlers[b2].length, fd_ed_handlers[b2].affects_f, false};
            } else if (fd_handlers[b1].handler) {
                slot = DecodeSlot{fd_handlers[b1].handler, fd_handlers[b1].cycles, fd_handlers[b1].length, fd_handlers[b1].affects_f, false};
            } else {
                // FD prefix fallthrough: execute base instruction
                // Temporarily advance PC so READ_PC works correctly
                regs.PC = (pc + 1) & 0xFFFF;
                slot = DecodeSlot{base_handlers[b1].handler, base_handlers[b1].cycles, (uint8_t)(base_handlers[b1].length + 1), base_handlers[b1].affects_f, base_handlers[b1].is_ld_a_ir};
            }
        } else {
            slot = DecodeSlot{base_handlers[opcode].handler, base_handlers[opcode].cycles, base_handlers[opcode].length, base_handlers[opcode].affects_f, base_handlers[opcode].is_ld_a_ir};
        }
    }

    if (slot.handler == nullptr) {
        // Check for DD/FD prefix fallthrough (decoder returned null for non-indexed op)
        if ((opcode == 0xDD || opcode == 0xFD) && b1 != 0xCB && b1 != 0xED) {
            // Fallthrough: execute base instruction with adjusted PC
            regs.PC = (pc + 1) & 0xFFFF;
            current_opcode = b1;
            _is_iy = (opcode == 0xFD);
            _is_ld_a_ir = base_handlers[b1].is_ld_a_ir;
            regs.R = (regs.R & 0x80) | ((regs.R + 2) & 0x7F);
            _pc_modified = false;
            int t = base_handlers[b1].handler(*this);
            if (!_pc_modified) {
                regs.PC = (pc + 1 + base_handlers[b1].length) & 0xFFFF;
            }
            cycles += base_handlers[b1].cycles;
            instruction_count++;
            return base_handlers[b1].cycles;
        }
        cycles += 4;
        regs.PC = (pc + 1) & 0xFFFF;
        return 4;
    }

    // R register refresh
    // For CB/DD/FD/ED prefixed instructions, R increments by 2
    uint8_t r_inc = 1;
    if (opcode == 0xCB || opcode == 0xDD || opcode == 0xFD || opcode == 0xED) {
        r_inc = 2;
    }
    regs.R = (regs.R & 0x80) | ((regs.R + r_inc) & 0x7F);

    // For CB/ED/DD/FD prefixed instructions, current_opcode must be the
    // second byte (the actual operation), not the prefix byte
    current_opcode = opcode;
    _pc_modified = false;
    _is_ld_a_ir = false;
    _is_iy = false;

    if (opcode == 0xCB || opcode == 0xED) {
        current_opcode = b1;
    } else if (opcode == 0xDD || opcode == 0xFD) {
        _is_iy = (opcode == 0xFD);
        if (b1 == 0xCB) {
            current_opcode = b3;
        } else if (b1 == 0xED) {
            current_opcode = b2;
        } else {
            current_opcode = b1;
        }
    }

    int start_cycles = cycles;
    int t = slot.handler(*this);

    // Q factor tracking
    regs.LAST_Q = regs.Q;
    if (slot.affects_f) {
        regs.Q = regs.F;
    } else {
        regs.Q = 0;
    }

    if (!_pc_modified) {
        regs.PC = (pc + slot.length) & 0xFFFF;
    }

    cycles += t;
    instruction_count++;
    // Return total cycles consumed including contention delays
    // (bus callbacks may add to cycles during handler execution)
    return cycles - start_cycles;
}

int CPU::run_frame(int t_states_per_frame) {
    int start_cycles = cycles;
    int target = cycles + t_states_per_frame;

    while (cycles < target) {
        step();
    }
    return cycles - start_cycles;
}

int CPU::run(int max_cycles) {
    int start = cycles;
    while (cycles - start < max_cycles && !halted) {
        step();
    }
    return cycles - start;
}

int CPU::run_instructions(int count) {
    int start = instruction_count;
    while (instruction_count - start < count && !halted) {
        step();
    }
    return cycles;
}

void CPU::trigger_interrupt(uint8_t data) {
    interrupt_pending = true;
    interrupt_data = data;
}

void CPU::trigger_nmi() {
    nmi_pending = true;
}

uint8_t CPU::read_reg8(int reg) {
    switch (reg) {
        case 0: return regs.B;
        case 1: return regs.C;
        case 2: return regs.D;
        case 3: return regs.E;
        case 4: return regs.H;
        case 5: return regs.L;
        case 6: return _bus_read(regs.HL(), cycles);
        default: return regs.A;
    }
}

void CPU::write_reg8(int reg, uint8_t value) {
    switch (reg) {
        case 0: regs.B = value; break;
        case 1: regs.C = value; break;
        case 2: regs.D = value; break;
        case 3: regs.E = value; break;
        case 4: regs.H = value; break;
        case 5: regs.L = value; break;
        case 6: _bus_write(regs.HL(), value, cycles); break;
        default: regs.A = value; break;
    }
}