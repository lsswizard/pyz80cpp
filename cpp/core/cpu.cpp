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
    int start_cycles = cycles;
    uint16_t pc = regs.PC;

    // EI deferral logic
    if (regs.EI_PENDING) {
        regs.EI_PENDING = false;
        regs.EI_JUST_RESOLVED = true;
        regs.IFF1 = true;
        regs.IFF2 = true;
    }

    // Handle NMI (non-maskable interrupt)
    if (nmi_pending) {
        nmi_pending = false;
        halted = false;
        regs.IFF1 = false;
        regs.R = (regs.R & 0x80) | ((regs.R + 1) & 0x7F);
        // When exiting HALT, return address is PC+1 (past the HALT instruction)
        uint16_t ret_addr = (regs.PC + 1) & 0xFFFF;
        _wait(3); // 7T M1 instead of 4T
        regs.SP = (regs.SP - 1) & 0xFFFF;
        _bus_write(regs.SP, ret_addr >> 8);
        regs.SP = (regs.SP - 1) & 0xFFFF;
        _bus_write(regs.SP, ret_addr & 0xFF);
        regs.PC = 0x0066;
        regs.MEMPTR = 0x0066;
        return 11;
    }

    // Handle maskable interrupt
    if (interrupt_pending && regs.IFF1 && !regs.EI_JUST_RESOLVED) {
        interrupt_pending = false;
        halted = false;
        regs.IFF1 = false;
        regs.IFF2 = false;
        // Interrupt is 13T: 7T+3T+3T
        regs.R = (regs.R & 0x80) | ((regs.R + 1) & 0x7F);
        uint16_t ret_addr = halted ? ((regs.PC + 1) & 0xFFFF) : regs.PC;
        _wait(3); // 7T M1 instead of 4T
        regs.SP = (regs.SP - 1) & 0xFFFF;
        _bus_write(regs.SP, ret_addr >> 8);
        regs.SP = (regs.SP - 1) & 0xFFFF;
        _bus_write(regs.SP, ret_addr & 0xFF);

        if (_is_ld_a_ir) {
            regs.F &= ~0x04;
        }

        if (regs.IM == 0) {
            uint8_t opcode = interrupt_data;
            if ((opcode & 0xC7) == 0xC7) {
                regs.PC = opcode & 0x38;
            } else {
                regs.PC = 0x0038;
            }
            regs.MEMPTR = regs.PC;
            return 13;
        } else if (regs.IM == 1) {
            regs.PC = 0x0038;
            regs.MEMPTR = 0x0038;
            return 13;
        } else {
            uint16_t vector_addr = (uint16_t)((regs.I << 8) | (interrupt_data & 0xFE));
            uint8_t lo = _bus_read(vector_addr);
            uint8_t hi = _bus_read((vector_addr + 1) & 0xFFFF);
            regs.MEMPTR = (uint16_t)(lo | (hi << 8));
            regs.PC = regs.MEMPTR;
            return 19;
        }
    }

    if (regs.EI_JUST_RESOLVED) {
        regs.EI_JUST_RESOLVED = false;
    }

    if (halted) {
        // HALT: execute NOPs (4T)
        regs.R = (regs.R & 0x80) | ((regs.R + 1) & 0x7F);
        cycles += 4;
        return 4;
    }

    // peek for decoding (does not add cycles)
    uint8_t opcode = _is_simple_bus ? _mem[pc] : bus->bus_read(pc, cycles, CycleType::MEM_RD);
    uint8_t b1 = _is_simple_bus ? _mem[(pc + 1) & 0xFFFF] : bus->bus_read((pc + 1) & 0xFFFF, cycles, CycleType::MEM_RD);
    uint8_t b2 = _is_simple_bus ? _mem[(pc + 2) & 0xFFFF] : bus->bus_read((pc + 2) & 0xFFFF, cycles, CycleType::MEM_RD);
    uint8_t b3 = _is_simple_bus ? _mem[(pc + 3) & 0xFFFF] : bus->bus_read((pc + 3) & 0xFFFF, cycles, CycleType::MEM_RD);

    DecodeSlot slot;
    if (_is_simple_bus) {
        slot = decoder.decode(_mem, pc);
    } else {
        slot = decoder.decode_from_bytes(opcode, b1, b2, b3);
    }

    if (slot.handler == nullptr) {
        // Check for DD/FD fallthrough
        if (opcode == 0xDD || opcode == 0xFD) {
            _bus_fetch(regs.PC++); // Fetch DD/FD (4T)
            _is_iy = (opcode == 0xFD);
            // Consume all consecutive DD/FD prefixes
            while (true) {
                uint16_t next_pc = regs.PC;
                uint8_t next_op = _is_simple_bus ? _mem[next_pc] : bus->bus_read(next_pc, cycles, CycleType::MEM_RD);
                if (next_op == 0xDD || next_op == 0xFD) {
                    _bus_fetch(regs.PC++); // Fetch next prefix (4T)
                    _is_iy = (next_op == 0xFD);
                    continue;
                }
                // Execute the base instruction
                current_opcode = _bus_fetch(regs.PC++); // Fetch actual opcode (4T)
                auto base_slot = base_handlers[current_opcode];
                if (base_slot.handler == nullptr) {
                    // Unknown base opcode after prefix - treat as NOP
                    instruction_count++;
                    return cycles - start_cycles;
                }
                _is_ld_a_ir = base_slot.is_ld_a_ir;
                _pc_modified = false;
                base_slot.handler(*this);
                instruction_count++;
                return cycles - start_cycles;
            }
        }
        // Truly unknown: 4T NOP
        _bus_fetch(regs.PC++);
        instruction_count++;
        return 4;
    }

    _bus_fetch(regs.PC++); // Fetch first opcode byte (4T)
    current_opcode = opcode;
    _pc_modified = false;
    _is_ld_a_ir = slot.is_ld_a_ir;
    _is_iy = (opcode == 0xFD);

    uint8_t old_f = regs.F;
    slot.handler(*this);

    // Q factor tracking (Patrik Rak discovery)
    // Q = new F if flags actually changed, unless EX AF,AF' or POP AF
    regs.LAST_Q = regs.Q;
    if (slot.affects_f && regs.F != old_f && opcode != 0x08 && opcode != 0xF1) {
        regs.Q = regs.F;
    } else {
        regs.Q = 0;
    }

    instruction_count++;
    return cycles - start_cycles;
}

int CPU::run_frame(int t_states_per_frame) {
    int target = cycles + t_states_per_frame;
    while (cycles < target) {
        step();
    }
    return cycles;
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
        case 6: return _bus_read(regs.HL());
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
        case 6: _bus_write(regs.HL(), value); break;
        default: regs.A = value; break;
    }
}
