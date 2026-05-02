#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

// ============================================================
// Load and Exchange Instructions
// ============================================================

void handle_ld_r_r(Z80& cpu) {
    cpu.write_reg8((cpu.current_opcode >> 3) & 7,
                    cpu.read_reg8(cpu.current_opcode & 7));
    cpu.regs.Q = 0;
}

void handle_ld_r_n(Z80& cpu) {
    cpu.write_reg8((cpu.current_opcode >> 3) & 7,
                    cpu.read(cpu.regs.PC++));
    cpu.regs.Q = 0;
}

void handle_ld_r_hl(Z80& cpu) {
    cpu.write_reg8((cpu.current_opcode >> 3) & 7,
                    cpu.read(cpu.regs.HL()));
    cpu.regs.Q = 0;
}

void handle_ld_hl_r(Z80& cpu) {
    cpu.write(cpu.regs.HL(), cpu.read_reg8(cpu.current_opcode & 7));
    cpu.regs.Q = 0;
}

void handle_ld_hl_n(Z80& cpu) {
    cpu.write(cpu.regs.HL(), cpu.read(cpu.regs.PC++));
    cpu.regs.Q = 0;
}

// ============================================================
// LD A,(BC) / LD A,(DE) / LD A,(nn)
// ============================================================

void handle_ld_a_bc(Z80& cpu) {
    cpu.regs.A       = cpu.read(cpu.regs.BC());
    cpu.regs.MEMPTR  = (cpu.regs.BC() + 1) & 0xFFFF;
    cpu.regs.Q = 0;
}

void handle_ld_a_de(Z80& cpu) {
    cpu.regs.A       = cpu.read(cpu.regs.DE());
    cpu.regs.MEMPTR  = (cpu.regs.DE() + 1) & 0xFFFF;
    cpu.regs.Q = 0;
}

void handle_ld_a_nn(Z80& cpu) {
    uint8_t  lo   = cpu.read(cpu.regs.PC++);
    uint8_t  hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (uint16_t(hi) << 8) | lo;
    cpu.regs.A       = cpu.read(addr);
    cpu.regs.MEMPTR  = (addr + 1) & 0xFFFF;
    cpu.regs.Q = 0;
}

// ============================================================
// LD (BC),A / LD (DE),A / LD (nn),A
// MEMPTR handling per Z80 documented behaviour
// ============================================================

void handle_ld_bc_a(Z80& cpu) {
    cpu.write(cpu.regs.BC(), cpu.regs.A);
    // MEMPTR: high byte = A, low byte = (BC+1) & 0xFF
    cpu.regs.MEMPTR = (uint16_t(cpu.regs.A) << 8) | ((cpu.regs.BC() + 1) & 0xFF);
    cpu.regs.Q = 0;
}

void handle_ld_de_a(Z80& cpu) {
    cpu.write(cpu.regs.DE(), cpu.regs.A);
    cpu.regs.MEMPTR = (uint16_t(cpu.regs.A) << 8) | ((cpu.regs.DE() + 1) & 0xFF);
    cpu.regs.Q = 0;
}

void handle_ld_nn_a(Z80& cpu) {
    uint8_t  lo   = cpu.read(cpu.regs.PC++);
    uint8_t  hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (uint16_t(hi) << 8) | lo;
    cpu.write(addr, cpu.regs.A);
    cpu.regs.MEMPTR = (uint16_t(cpu.regs.A) << 8) | ((addr + 1) & 0xFF);
    cpu.regs.Q = 0;
}

// ============================================================
// 16-bit loads
// ============================================================

void handle_ld_rr_nn(Z80& cpu) {
    uint8_t  lo  = cpu.read(cpu.regs.PC++);
    uint8_t  hi  = cpu.read(cpu.regs.PC++);
    uint16_t val = (uint16_t(hi) << 8) | lo;
    switch ((cpu.current_opcode >> 4) & 3) {
        case 0: cpu.regs.set_BC(val); break;
        case 1: cpu.regs.set_DE(val); break;
        case 2: cpu.regs.set_HL(val); break;
        case 3: cpu.regs.SP = val;    break;
    }
    cpu.regs.Q = 0;
}

// LD HL,(nn) — main-table 0x2A  (16 T-states)
void handle_ld_hl_nn(Z80& cpu) {
    uint8_t  lo   = cpu.read(cpu.regs.PC++);
    uint8_t  hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (uint16_t(hi) << 8) | lo;
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    uint8_t vl = cpu.read(addr);
    uint8_t vh = cpu.read(addr + 1);
    cpu.regs.set_HL((uint16_t(vh) << 8) | vl);
    cpu.regs.Q = 0;
}

// LD (nn),HL — main-table 0x22  (16 T-states)
void handle_ld_nn_hl(Z80& cpu) {
    uint8_t  lo   = cpu.read(cpu.regs.PC++);
    uint8_t  hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (uint16_t(hi) << 8) | lo;
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    cpu.write(addr,     cpu.regs.L);
    cpu.write(addr + 1, cpu.regs.H);
    cpu.regs.Q = 0;
}

// LD rr,(nn) — ED-prefix variants  (20 T-states)
void handle_ld_rr_nn_ind(Z80& cpu) {
    uint8_t  lo   = cpu.read(cpu.regs.PC++);
    uint8_t  hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (uint16_t(hi) << 8) | lo;
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    uint16_t val  = cpu.read(addr) | (uint16_t(cpu.read(addr + 1)) << 8);
    switch ((cpu.current_opcode >> 4) & 3) {
        case 0: cpu.regs.set_BC(val); break;
        case 1: cpu.regs.set_DE(val); break;
        case 2: cpu.regs.set_HL(val); break;
        case 3: cpu.regs.SP = val;    break;
    }
    cpu.regs.Q = 0;
}

// LD (nn),rr — ED-prefix variants  (20 T-states)
void handle_ld_nn_rr_ind(Z80& cpu) {
    uint8_t  lo   = cpu.read(cpu.regs.PC++);
    uint8_t  hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (uint16_t(hi) << 8) | lo;
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    uint16_t val;
    switch ((cpu.current_opcode >> 4) & 3) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = cpu.regs.HL(); break;
        default: val = cpu.regs.SP;  break;
    }
    cpu.write(addr,     val & 0xFF);
    cpu.write(addr + 1, val >> 8);
    cpu.regs.Q = 0;
}

// LD SP,HL — 6 T-states: 4(M1) + 2(internal)
void handle_ld_sp_hl(Z80& cpu) {
    cpu.wait(2);
    cpu.regs.SP = cpu.regs.HL();
    cpu.regs.Q  = 0;
}

// ============================================================
// Stack operations
// ============================================================

void handle_push_rr(Z80& cpu) {
    int      reg = (cpu.current_opcode >> 4) & 3;
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = cpu.regs.HL(); break;
        default: val = cpu.regs.AF(); break;
    }
    cpu.wait(1);
    cpu.write(--cpu.regs.SP, val >> 8);
    cpu.write(--cpu.regs.SP, val & 0xFF);
    cpu.regs.Q = 0;
}

void handle_pop_rr(Z80& cpu) {
    uint8_t  lo  = cpu.read(cpu.regs.SP++);
    uint8_t  hi  = cpu.read(cpu.regs.SP++);
    uint16_t val = (uint16_t(hi) << 8) | lo;
    switch ((cpu.current_opcode >> 4) & 3) {
        case 0: cpu.regs.set_BC(val); break;
        case 1: cpu.regs.set_DE(val); break;
        case 2: cpu.regs.set_HL(val); break;
        default: cpu.regs.set_AF(val); break;
    }
    cpu.regs.Q = 0;
}

// ============================================================
// Exchange Instructions
// ============================================================

void handle_ex_de_hl(Z80& cpu) {
    uint16_t de = cpu.regs.DE();
    cpu.regs.set_DE(cpu.regs.HL());
    cpu.regs.set_HL(de);
    cpu.regs.Q = 0;
}

void handle_ex_af_afp(Z80& cpu) {
    cpu.regs.swap_af();
    cpu.regs.Q = 0;
}

// EXX — swap BC↔BC', DE↔DE', HL↔HL'
// BUG FIX: delegates to the corrected swap_all() in registers.h
void handle_exx(Z80& cpu) {
    cpu.regs.swap_all();
    cpu.regs.Q = 0;
}

// EX (SP),HL — 19 T-states: 4(M1)+3(rd lo)+3(rd hi)+1(int)+3(wr hi)+3(wr lo)+2(int)
void handle_ex_sp_hl(Z80& cpu) {
    uint16_t hl  = cpu.regs.HL();
    uint16_t sp  = cpu.regs.SP;
    uint8_t  lo  = cpu.read(sp);
    uint8_t  hi  = cpu.read(sp + 1);
    cpu.wait(1);
    uint16_t val = (uint16_t(hi) << 8) | lo;
    cpu.write(sp + 1, hl >> 8);
    cpu.write(sp,     hl & 0xFF);
    cpu.wait(2);
    cpu.regs.set_HL(val);
    cpu.regs.MEMPTR = val;
    cpu.regs.Q = 0;
}

} // namespace z80
