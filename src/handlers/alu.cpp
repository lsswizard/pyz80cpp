#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

// ============================================================
// Helper: compute ADC flags for 8-bit a + b + carry
// ============================================================
static inline uint8_t adc8_flags(uint8_t a, uint8_t b, uint8_t c) {
    int res = a + b + c;
    uint8_t r = res & 0xFF;
    uint8_t f = 0;
    if (r == 0)                               f |= Flags::Z;
    if (r & 0x80)                             f |= Flags::S;
    if (res > 0xFF)                           f |= Flags::C;
    if ((a ^ b ^ res) & 0x10)                f |= Flags::H;
    if (~(a ^ b) & (a ^ res) & 0x80)         f |= Flags::PV;
    f |= r & (Flags::F5 | Flags::F3);
    return f;
}

// ============================================================
// Helper: compute SBC flags for 8-bit a - b - carry
// ============================================================
static inline uint8_t sbc8_flags(uint8_t a, uint8_t b, uint8_t c) {
    int res = a - b - c;
    uint8_t r = res & 0xFF;
    uint8_t f = Flags::N;
    if (r == 0)                               f |= Flags::Z;
    if (r & 0x80)                             f |= Flags::S;
    if (res < 0)                              f |= Flags::C;
    if ((a ^ b ^ res) & 0x10)                f |= Flags::H;
    if ((a ^ b) & (a ^ res) & 0x80)          f |= Flags::PV;
    f |= r & (Flags::F5 | Flags::F3);
    return f;
}

// ============================================================
// ALU — register / (HL) source
// ============================================================

void handle_add_a(Z80& cpu) {
    uint8_t val = cpu.read_reg8(cpu.current_opcode & 7);
    uint8_t a   = cpu.regs.A;
    cpu.regs.A  = (a + val) & 0xFF;
    cpu.regs.F  = calc_add_flags(a, val);
    cpu.regs.Q  = cpu.regs.F;
}

void handle_adc_a(Z80& cpu) {
    uint8_t val = cpu.read_reg8(cpu.current_opcode & 7);
    uint8_t a   = cpu.regs.A;
    uint8_t c   = (cpu.regs.F & Flags::C) ? 1 : 0;
    cpu.regs.A  = (a + val + c) & 0xFF;
    cpu.regs.F  = adc8_flags(a, val, c);
    cpu.regs.Q  = cpu.regs.F;
}

void handle_sub(Z80& cpu) {
    uint8_t val = cpu.read_reg8(cpu.current_opcode & 7);
    uint8_t a   = cpu.regs.A;
    cpu.regs.A  = (a - val) & 0xFF;
    cpu.regs.F  = calc_sub_flags(a, val);
    cpu.regs.Q  = cpu.regs.F;
}

void handle_sbc_a(Z80& cpu) {
    uint8_t val = cpu.read_reg8(cpu.current_opcode & 7);
    uint8_t a   = cpu.regs.A;
    uint8_t c   = (cpu.regs.F & Flags::C) ? 1 : 0;
    cpu.regs.A  = (a - val - c) & 0xFF;
    cpu.regs.F  = sbc8_flags(a, val, c);
    cpu.regs.Q  = cpu.regs.F;
}

void handle_and(Z80& cpu) {
    cpu.regs.A &= cpu.read_reg8(cpu.current_opcode & 7);
    uint8_t r   = cpu.regs.A;
    cpu.regs.F  = Flags::H
                | (r & (Flags::S | Flags::F5 | Flags::F3))
                | (r == 0 ? Flags::Z : 0)
                | FlagTables::PARITY_TABLE[r];
    cpu.regs.Q  = cpu.regs.F;
}

void handle_or(Z80& cpu) {
    cpu.regs.A |= cpu.read_reg8(cpu.current_opcode & 7);
    uint8_t r   = cpu.regs.A;
    cpu.regs.F  = (r & (Flags::S | Flags::F5 | Flags::F3))
                | (r == 0 ? Flags::Z : 0)
                | FlagTables::PARITY_TABLE[r];
    cpu.regs.Q  = cpu.regs.F;
}

void handle_xor(Z80& cpu) {
    cpu.regs.A ^= cpu.read_reg8(cpu.current_opcode & 7);
    uint8_t r   = cpu.regs.A;
    cpu.regs.F  = (r & (Flags::S | Flags::F5 | Flags::F3))
                | (r == 0 ? Flags::Z : 0)
                | FlagTables::PARITY_TABLE[r];
    cpu.regs.Q  = cpu.regs.F;
}

// CP: compare without storing result; F3/F5 come from the *operand* (not result)
void handle_cp(Z80& cpu) {
    uint8_t val = cpu.read_reg8(cpu.current_opcode & 7);
    uint8_t a   = cpu.regs.A;
    uint8_t f   = calc_sub_flags(a, val);
    // CP: F3/F5 come from operand, not result
    f &= ~(Flags::F5 | Flags::F3);
    f |= val & (Flags::F5 | Flags::F3);
    cpu.regs.F  = f;
    cpu.regs.Q  = cpu.regs.F;
}

// ============================================================
// ALU — immediate source
// ============================================================

void handle_add_a_n(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.PC++);
    uint8_t a   = cpu.regs.A;
    cpu.regs.A  = (a + val) & 0xFF;
    cpu.regs.F  = calc_add_flags(a, val);
    cpu.regs.Q  = cpu.regs.F;
}

void handle_adc_a_n(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.PC++);
    uint8_t a   = cpu.regs.A;
    uint8_t c   = (cpu.regs.F & Flags::C) ? 1 : 0;
    cpu.regs.A  = (a + val + c) & 0xFF;
    cpu.regs.F  = adc8_flags(a, val, c);
    cpu.regs.Q  = cpu.regs.F;
}

void handle_sub_n(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.PC++);
    uint8_t a   = cpu.regs.A;
    cpu.regs.A  = (a - val) & 0xFF;
    cpu.regs.F  = calc_sub_flags(a, val);
    cpu.regs.Q  = cpu.regs.F;
}

void handle_sbc_a_n(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.PC++);
    uint8_t a   = cpu.regs.A;
    uint8_t c   = (cpu.regs.F & Flags::C) ? 1 : 0;
    cpu.regs.A  = (a - val - c) & 0xFF;
    cpu.regs.F  = sbc8_flags(a, val, c);
    cpu.regs.Q  = cpu.regs.F;
}

void handle_and_n(Z80& cpu) {
    cpu.regs.A &= cpu.read(cpu.regs.PC++);
    uint8_t r   = cpu.regs.A;
    cpu.regs.F  = Flags::H
                | (r & (Flags::S | Flags::F5 | Flags::F3))
                | (r == 0 ? Flags::Z : 0)
                | FlagTables::PARITY_TABLE[r];
    cpu.regs.Q  = cpu.regs.F;
}

void handle_or_n(Z80& cpu) {
    cpu.regs.A |= cpu.read(cpu.regs.PC++);
    uint8_t r   = cpu.regs.A;
    cpu.regs.F  = (r & (Flags::S | Flags::F5 | Flags::F3))
                | (r == 0 ? Flags::Z : 0)
                | FlagTables::PARITY_TABLE[r];
    cpu.regs.Q  = cpu.regs.F;
}

void handle_xor_n(Z80& cpu) {
    cpu.regs.A ^= cpu.read(cpu.regs.PC++);
    uint8_t r   = cpu.regs.A;
    cpu.regs.F  = (r & (Flags::S | Flags::F5 | Flags::F3))
                | (r == 0 ? Flags::Z : 0)
                | FlagTables::PARITY_TABLE[r];
    cpu.regs.Q  = cpu.regs.F;
}

void handle_cp_n(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.PC++);
    uint8_t a   = cpu.regs.A;
    uint8_t f   = calc_sub_flags(a, val);
    // CP: F3/F5 from operand
    f &= ~(Flags::F5 | Flags::F3);
    f |= val & (Flags::F5 | Flags::F3);
    cpu.regs.F  = f;
    cpu.regs.Q  = cpu.regs.F;
}

// ADD A,(HL) — explicit memory read
void handle_add_a_hl(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    uint8_t a   = cpu.regs.A;
    cpu.regs.A  = (a + val) & 0xFF;
    cpu.regs.F  = calc_add_flags(a, val);
    cpu.regs.Q  = cpu.regs.F;
}

// ============================================================
// INC / DEC
// ============================================================

void handle_inc_r(Z80& cpu) {
    int     reg = (cpu.current_opcode >> 3) & 7;
    uint8_t val = cpu.read_reg8(reg);
    uint8_t res = (val + 1) & 0xFF;
    cpu.write_reg8(reg, res);
    cpu.regs.F  = (cpu.regs.F & Flags::C) | calc_inc_flags(val);
    cpu.regs.Q  = cpu.regs.F;
}

void handle_dec_r(Z80& cpu) {
    int     reg = (cpu.current_opcode >> 3) & 7;
    uint8_t val = cpu.read_reg8(reg);
    uint8_t res = (val - 1) & 0xFF;
    cpu.write_reg8(reg, res);
    cpu.regs.F  = (cpu.regs.F & Flags::C) | calc_dec_flags(val);
    cpu.regs.Q  = cpu.regs.F;
}

// INC (HL) — 11 T-states: 4(M1) + 3(read) + 1(internal) + 3(write)
void handle_inc_hl(Z80& cpu) {
    uint16_t addr = cpu.regs.HL();
    uint8_t  val  = cpu.read(addr);
    cpu.wait(1);
    uint8_t res = (val + 1) & 0xFF;
    cpu.write(addr, res);
    cpu.regs.F  = (cpu.regs.F & Flags::C) | calc_inc_flags(val);
    cpu.regs.Q  = cpu.regs.F;
}

// DEC (HL) — 11 T-states
void handle_dec_hl(Z80& cpu) {
    uint16_t addr = cpu.regs.HL();
    uint8_t  val  = cpu.read(addr);
    cpu.wait(1);
    uint8_t res = (val - 1) & 0xFF;
    cpu.write(addr, res);
    cpu.regs.F  = (cpu.regs.F & Flags::C) | calc_dec_flags(val);
    cpu.regs.Q  = cpu.regs.F;
}

// INC rr — 6 T-states: 4(M1) + 2(internal)
void handle_inc_rr(Z80& cpu) {
    cpu.wait(2);
    switch ((cpu.current_opcode >> 4) & 3) {
        case 0: cpu.regs.set_BC(cpu.regs.BC() + 1); break;
        case 1: cpu.regs.set_DE(cpu.regs.DE() + 1); break;
        case 2: cpu.regs.set_HL(cpu.regs.HL() + 1); break;
        case 3: cpu.regs.SP++; break;
    }
}

// DEC rr — 6 T-states
void handle_dec_rr(Z80& cpu) {
    cpu.wait(2);
    switch ((cpu.current_opcode >> 4) & 3) {
        case 0: cpu.regs.set_BC(cpu.regs.BC() - 1); break;
        case 1: cpu.regs.set_DE(cpu.regs.DE() - 1); break;
        case 2: cpu.regs.set_HL(cpu.regs.HL() - 1); break;
        case 3: cpu.regs.SP--; break;
    }
}

// ADD HL,rr — 11 T-states: 4(M1) + 7(internal)
void handle_add_hl_rr(Z80& cpu) {
    uint16_t hl  = cpu.regs.HL();
    int      reg = (cpu.current_opcode >> 4) & 3;
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = hl;            break;
        default: val = cpu.regs.SP;  break;
    }
    cpu.wait(7);
    uint32_t res = hl + val;
    uint8_t  h   = ((hl & 0x0FFF) + (val & 0x0FFF)) > 0x0FFF ? Flags::H : 0;
    // ADD HL,rr preserves S, Z, PV; clears N; updates H, C, F5, F3
    cpu.regs.F   = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV))
                 | ((res >> 8) & (Flags::F5 | Flags::F3))
                 | h
                 | (res > 0xFFFF ? Flags::C : 0);
    cpu.regs.set_HL(res & 0xFFFF);
    cpu.regs.MEMPTR = hl + 1;
    cpu.regs.Q  = cpu.regs.F;
}

// ============================================================
// 16-bit ED-prefix ALU
// ============================================================

// ADC HL,rr — 15 T-states: 4(M1) + 4(M1 ED) + 7(internal)
void handle_adc_hl_rr(Z80& cpu) {
    int      reg   = (cpu.current_opcode >> 4) & 3;
    uint16_t hl    = cpu.regs.HL();
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = hl;            break;
        default: val = cpu.regs.SP;  break;
    }
    cpu.wait(7);
    uint8_t  c   = (cpu.regs.F & Flags::C) ? 1 : 0;
    uint32_t res = hl + val + c;
    uint8_t  h   = ((hl & 0x0FFF) + (val & 0x0FFF) + c) > 0x0FFF ? Flags::H : 0;
    uint8_t  pv  = (~(hl ^ val) & (hl ^ (uint16_t)res) & 0x8000) ? Flags::PV : 0;
    uint8_t  r8  = (res >> 8) & 0xFF;
    cpu.regs.F   = (r8 & (Flags::S | Flags::F5 | Flags::F3))
                 | ((res & 0xFFFF) == 0 ? Flags::Z : 0)
                 | h | pv
                 | (res > 0xFFFF ? Flags::C : 0);
    cpu.regs.set_HL(res & 0xFFFF);
    cpu.regs.MEMPTR = hl + 1;
    cpu.regs.Q  = cpu.regs.F;
}

// SBC HL,rr — 15 T-states
void handle_sbc_hl_rr(Z80& cpu) {
    int      reg   = (cpu.current_opcode >> 4) & 3;
    uint16_t hl    = cpu.regs.HL();
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = hl;            break;
        default: val = cpu.regs.SP;  break;
    }
    cpu.wait(7);
    uint8_t  c   = (cpu.regs.F & Flags::C) ? 1 : 0;
    int32_t  res = (int32_t)hl - val - c;
    uint8_t  h   = ((hl & 0x0FFF) < ((val & 0x0FFF) + c)) ? Flags::H : 0;
    uint8_t  pv  = ((hl ^ val) & (hl ^ (uint16_t)res) & 0x8000) ? Flags::PV : 0;
    uint8_t  r8  = (res >> 8) & 0xFF;
    cpu.regs.F   = Flags::N
                 | (r8 & (Flags::S | Flags::F5 | Flags::F3))
                 | ((res & 0xFFFF) == 0 ? Flags::Z : 0)
                 | h | pv
                 | (res < 0 ? Flags::C : 0);
    cpu.regs.set_HL(res & 0xFFFF);
    cpu.regs.MEMPTR = hl + 1;
    cpu.regs.Q  = cpu.regs.F;
}

// ============================================================
// Special ALU
// ============================================================

// DAA — BCD adjustment
void handle_daa(Z80& cpu) {
    uint8_t a    = cpu.regs.A;
    uint8_t f    = cpu.regs.F;
    uint8_t corr = 0;
    uint8_t new_c = f & Flags::C;
    uint8_t new_h = 0;

    if (f & Flags::N) {
        // After subtraction
        if ((f & Flags::H) || (a & 0x0F) > 9)  { corr |= 0x06; }
        if ((f & Flags::C) || a > 0x99)          { corr |= 0x60; new_c = Flags::C; }
        // H flag after subtraction: set if borrow from bit 4
        if ((f & Flags::H) && (a & 0x0F) < 6)   new_h = Flags::H;
        a -= corr;
    } else {
        // After addition
        if ((f & Flags::H) || (a & 0x0F) > 9)   { corr |= 0x06; }
        if ((f & Flags::C) || a > 0x99)          { corr |= 0x60; new_c = Flags::C; }
        // H flag after addition: set if carry from bit 3
        if ((a & 0x0F) > 9)                       new_h = Flags::H;
        a += corr;
    }

    cpu.regs.A  = a;
    cpu.regs.F  = (a & (Flags::S | Flags::F5 | Flags::F3))
                | (a == 0 ? Flags::Z : 0)
                | new_h
                | FlagTables::PARITY_TABLE[a]
                | (f & Flags::N)
                | new_c;
    cpu.regs.Q  = cpu.regs.F;
}

// CPL — complement accumulator (sets H and N, preserves others)
void handle_cpl(Z80& cpu) {
    cpu.regs.A  = ~cpu.regs.A;
    cpu.regs.F  = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV | Flags::C))
                | (cpu.regs.A & (Flags::F5 | Flags::F3))
                | Flags::N | Flags::H;
    cpu.regs.Q  = cpu.regs.F;
}

// NEG — negate accumulator (0 - A)
void handle_neg(Z80& cpu) {
    uint8_t a   = cpu.regs.A;
    uint8_t res = (0 - a) & 0xFF;
    cpu.regs.A  = res;
    cpu.regs.F  = Flags::N
                | (res & (Flags::S | Flags::F5 | Flags::F3))
                | (res == 0 ? Flags::Z : 0)
                | ((a & 0x0F) != 0 ? Flags::H : 0)  // H set if borrow from bit 4
                | (a == 0x80 ? Flags::PV : 0)
                | (a != 0 ? Flags::C : 0);
    cpu.regs.Q  = cpu.regs.F;
}

// CCF — complement carry flag
// F3/F5 are tricky: set from (Q XOR F) | A (per documented Z80 behaviour)
void handle_ccf(Z80& cpu) {
    uint8_t old_c   = cpu.regs.F & Flags::C;
    uint8_t f35_src = (cpu.regs.Q ^ cpu.regs.F) | cpu.regs.A;
    cpu.regs.F  = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV))
                | (f35_src & (Flags::F5 | Flags::F3))
                | (old_c ? Flags::H : 0)          // old carry → H
                | (old_c ? 0 : Flags::C);          // new carry = ~old
    cpu.regs.Q  = cpu.regs.F;
}

// SCF — set carry flag
void handle_scf(Z80& cpu) {
    uint8_t f35_src = (cpu.regs.Q ^ cpu.regs.F) | cpu.regs.A;
    cpu.regs.F  = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV))
                | (f35_src & (Flags::F5 | Flags::F3))
                | Flags::C;
    cpu.regs.Q  = cpu.regs.F;
}

// NOP
void handle_nop(Z80& cpu) {
    cpu.regs.Q = 0;
}

// HALT
void handle_halt(Z80& cpu) {
    cpu.halted  = true;
    // PC is decremented so that on each HALT tick the same opcode is re-fetched
    cpu.regs.PC = (cpu.regs.PC - 1) & 0xFFFF;
}

// ============================================================
// Rotate accumulator (A) — 4 T-states each; flags: S/Z/PV preserved; H/N cleared
// BUG FIX: original code kept old C in the preserved mask, causing carry corruption
// ============================================================

// RLCA — rotate A left; bit 7 goes to bit 0 and to C
void handle_rlca(Z80& cpu) {
    uint8_t c   = cpu.regs.A >> 7;
    cpu.regs.A  = (cpu.regs.A << 1) | c;
    cpu.regs.F  = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV))
                | (cpu.regs.A & (Flags::F5 | Flags::F3))
                | c;    // new C only
    cpu.regs.Q  = cpu.regs.F;
}

// RRCA — rotate A right; bit 0 goes to bit 7 and to C
void handle_rrca(Z80& cpu) {
    uint8_t c   = cpu.regs.A & 1;
    cpu.regs.A  = (cpu.regs.A >> 1) | (c << 7);
    cpu.regs.F  = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV))
                | (cpu.regs.A & (Flags::F5 | Flags::F3))
                | c;
    cpu.regs.Q  = cpu.regs.F;
}

// RLA — rotate A left through carry
void handle_rla(Z80& cpu) {
    uint8_t new_c = cpu.regs.A >> 7;
    cpu.regs.A    = (cpu.regs.A << 1) | ((cpu.regs.F & Flags::C) ? 1 : 0);
    cpu.regs.F    = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV))
                  | (cpu.regs.A & (Flags::F5 | Flags::F3))
                  | new_c;   // new C only — old C was consumed
    cpu.regs.Q    = cpu.regs.F;
}

// RRA — rotate A right through carry
void handle_rra(Z80& cpu) {
    uint8_t new_c = cpu.regs.A & 1;
    cpu.regs.A    = (cpu.regs.A >> 1) | ((cpu.regs.F & Flags::C) ? 0x80 : 0);
    cpu.regs.F    = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV))
                  | (cpu.regs.A & (Flags::F5 | Flags::F3))
                  | new_c;
    cpu.regs.Q    = cpu.regs.F;
}

// ============================================================
// ED-prefix special loads
// ============================================================

void handle_ld_a_i(Z80& cpu) {
    cpu.wait(1);
    cpu.regs.A  = cpu.regs.I;
    // PV = IFF2 at the moment of the instruction (not after any pending interrupt)
    cpu.regs.F  = (cpu.regs.I & (Flags::S | Flags::F5 | Flags::F3))
                | (cpu.regs.I == 0 ? Flags::Z : 0)
                | (cpu.regs.IFF2 ? Flags::PV : 0)
                | (cpu.regs.F & Flags::C);
    cpu.regs.Q  = cpu.regs.F;
}

void handle_ld_a_r(Z80& cpu) {
    cpu.wait(1);
    cpu.regs.A  = cpu.regs.R;
    cpu.regs.F  = (cpu.regs.R & (Flags::S | Flags::F5 | Flags::F3))
                | (cpu.regs.R == 0 ? Flags::Z : 0)
                | (cpu.regs.IFF2 ? Flags::PV : 0)
                | (cpu.regs.F & Flags::C);
    cpu.regs.Q  = cpu.regs.F;
}

void handle_ld_i_a(Z80& cpu) {
    cpu.wait(1);
    cpu.regs.I  = cpu.regs.A;
    cpu.regs.Q  = 0;
}

void handle_ld_r_a(Z80& cpu) {
    cpu.wait(1);
    cpu.regs.R  = cpu.regs.A;
    cpu.regs.Q  = 0;
}

// ============================================================
// RLD / RRD — rotate through (HL) and accumulator nibbles
// ============================================================

void handle_rld(Z80& cpu) {
    uint16_t addr = cpu.regs.HL();
    uint8_t  mem  = cpu.read(addr);
    cpu.wait(4);
    uint8_t  a    = cpu.regs.A;
    cpu.regs.A    = (a & 0xF0) | (mem >> 4);
    cpu.write(addr, (mem << 4) | (a & 0x0F));
    cpu.regs.F    = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3))
                  | (cpu.regs.A == 0 ? Flags::Z : 0)
                  | FlagTables::PARITY_TABLE[cpu.regs.A]
                  | (cpu.regs.F & Flags::C);
    cpu.regs.MEMPTR = addr + 1;
    cpu.regs.Q    = cpu.regs.F;
}

void handle_rrd(Z80& cpu) {
    uint16_t addr = cpu.regs.HL();
    uint8_t  mem  = cpu.read(addr);
    cpu.wait(4);
    uint8_t  a    = cpu.regs.A;
    cpu.regs.A    = (a & 0xF0) | (mem & 0x0F);
    cpu.write(addr, (mem >> 4) | (a << 4));
    cpu.regs.F    = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3))
                  | (cpu.regs.A == 0 ? Flags::Z : 0)
                  | FlagTables::PARITY_TABLE[cpu.regs.A]
                  | (cpu.regs.F & Flags::C);
    cpu.regs.MEMPTR = addr + 1;
    cpu.regs.Q    = cpu.regs.F;
}

// ============================================================
// DD/FD-prefix: IX/IY indexed loads and ALU
// ============================================================

// LD r,(IX+d) / LD r,(IY+d)  — 19 T-states: 4+4+3+5+3
void handle_dd_fd_ld_r_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t   offset = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    cpu.write_reg8((cpu.current_opcode >> 3) & 7, cpu.read(addr));
    cpu.regs.Q = 0;
}

// LD (IX+d),r  — 19 T-states
void handle_dd_fd_ld_ixd_r(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t   offset = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    cpu.write(addr, cpu.read_reg8(cpu.current_opcode & 7));
    cpu.regs.Q = 0;
}

// LD (IX+d),n  — 19 T-states
void handle_dd_fd_ld_ixd_n(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t   offset = (int8_t)cpu.read(cpu.regs.PC++);
    uint8_t  val    = cpu.read(cpu.regs.PC++);
    cpu.wait(2);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    cpu.write(addr, val);
    cpu.regs.Q = 0;
}

// LD IX,nn / LD IY,nn
void handle_dd_fd_ld_ix_nn(Z80& cpu) {
    uint8_t  lo = cpu.read(cpu.regs.PC++);
    uint8_t  hi = cpu.read(cpu.regs.PC++);
    uint16_t v  = (uint16_t(hi) << 8) | lo;
    if (cpu.prefix_ix) cpu.regs.IX = v; else cpu.regs.IY = v;
    cpu.regs.Q = 0;
}

// LD (nn),IX / LD (nn),IY
void handle_dd_fd_ld_nn_ix(Z80& cpu) {
    uint8_t  lo   = cpu.read(cpu.regs.PC++);
    uint8_t  hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (uint16_t(hi) << 8) | lo;
    uint16_t val  = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    cpu.write(addr,     val & 0xFF);
    cpu.write(addr + 1, val >> 8);
    cpu.regs.MEMPTR = addr + 1;
    cpu.regs.Q = 0;
}

// LD IX,(nn) / LD IY,(nn)
void handle_dd_fd_ld_ix_nn_ind(Z80& cpu) {
    uint8_t  lo   = cpu.read(cpu.regs.PC++);
    uint8_t  hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (uint16_t(hi) << 8) | lo;
    uint16_t val  = cpu.read(addr) | (uint16_t(cpu.read(addr + 1)) << 8);
    cpu.regs.MEMPTR = addr + 1;
    if (cpu.prefix_ix) cpu.regs.IX = val; else cpu.regs.IY = val;
    cpu.regs.Q = 0;
}

// INC IX / INC IY  — 10 T-states: 4(DD M1) + 4(IX M1) + 2(internal)
void handle_dd_fd_inc_ix(Z80& cpu) {
    cpu.wait(2);
    if (cpu.prefix_ix) cpu.regs.IX++; else cpu.regs.IY++;
    cpu.regs.Q = 0;
}

// DEC IX / DEC IY
void handle_dd_fd_dec_ix(Z80& cpu) {
    cpu.wait(2);
    if (cpu.prefix_ix) cpu.regs.IX--; else cpu.regs.IY--;
    cpu.regs.Q = 0;
}

// INC IXH/IXL/IYH/IYL  — 8 T-states: 4(DD M1) + 4(op M1)
// BUG FIX: removed spurious cpu.wait(1) which added an extra T-state
void handle_dd_fd_inc_ixhl(Z80& cpu) {
    int     reg = (cpu.current_opcode == 0x24) ? 8 : 9;
    uint8_t val = cpu.read_reg8(reg);
    uint8_t res = (val + 1) & 0xFF;
    cpu.write_reg8(reg, res);
    cpu.regs.F  = (cpu.regs.F & Flags::C) | calc_inc_flags(val);
    cpu.regs.Q  = cpu.regs.F;
}

// DEC IXH/IXL/IYH/IYL  — 8 T-states
// BUG FIX: removed spurious cpu.wait(1)
void handle_dd_fd_dec_ixhl(Z80& cpu) {
    int     reg = (cpu.current_opcode == 0x25) ? 8 : 9;
    uint8_t val = cpu.read_reg8(reg);
    uint8_t res = (val - 1) & 0xFF;
    cpu.write_reg8(reg, res);
    cpu.regs.F  = (cpu.regs.F & Flags::C) | calc_dec_flags(val);
    cpu.regs.Q  = cpu.regs.F;
}

// ADD IX,rr / ADD IY,rr  — 15 T-states
void handle_dd_fd_add_ix_rr(Z80& cpu) {
    uint16_t ix  = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int      reg = (cpu.current_opcode >> 4) & 3;
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = ix;            break;
        default: val = cpu.regs.SP;  break;
    }
    cpu.wait(7);
    uint32_t res = ix + val;
    uint8_t  h   = ((ix & 0x0FFF) + (val & 0x0FFF)) > 0x0FFF ? Flags::H : 0;
    if (cpu.prefix_ix) cpu.regs.IX = res & 0xFFFF;
    else               cpu.regs.IY = res & 0xFFFF;
    cpu.regs.F  = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV))
                | ((res >> 8) & (Flags::F5 | Flags::F3))
                | h
                | (res > 0xFFFF ? Flags::C : 0);
    cpu.regs.MEMPTR = ix + 1;
    cpu.regs.Q  = cpu.regs.F;
}

// LD SP,IX / LD SP,IY
void handle_dd_fd_ld_sp_ix(Z80& cpu) {
    cpu.wait(2);
    cpu.regs.SP = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    cpu.regs.Q  = 0;
}

// PUSH IX / PUSH IY  — 15 T-states: 4+4+1+3+3
void handle_dd_fd_push_ix(Z80& cpu) {
    uint16_t val = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    cpu.wait(1);
    cpu.write(--cpu.regs.SP, val >> 8);
    cpu.write(--cpu.regs.SP, val & 0xFF);
    cpu.regs.Q = 0;
}

// POP IX / POP IY  — 14 T-states: 4+4+3+3
void handle_dd_fd_pop_ix(Z80& cpu) {
    uint16_t lo = cpu.read(cpu.regs.SP++);
    uint16_t hi = cpu.read(cpu.regs.SP++);
    uint16_t v  = (hi << 8) | lo;
    if (cpu.prefix_ix) cpu.regs.IX = v; else cpu.regs.IY = v;
    cpu.regs.Q = 0;
}

// EX (SP),IX / EX (SP),IY  — 23 T-states
void handle_dd_fd_ex_sp_ix(Z80& cpu) {
    uint16_t ix   = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    uint16_t sp   = cpu.regs.SP;
    uint8_t  lo   = cpu.read(sp);
    uint8_t  hi   = cpu.read(sp + 1);
    cpu.wait(1);
    uint16_t val  = (uint16_t(hi) << 8) | lo;
    cpu.write(sp + 1, ix >> 8);
    cpu.write(sp,     ix & 0xFF);
    cpu.wait(2);
    if (cpu.prefix_ix) cpu.regs.IX = val; else cpu.regs.IY = val;
    cpu.regs.MEMPTR = val;
    cpu.regs.Q = 0;
}

// JP (IX) / JP (IY)
void handle_dd_fd_jp_ix(Z80& cpu) {
    cpu.regs.PC = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    cpu.regs.Q  = 0;
}

// ============================================================
// DD/FD undocumented: unified LD r,IXH/L and LD IXH/L,r
//
// Opcode map (after DD/FD prefix):
//   0x44 LD B,IXH    0x45 LD B,IXL    0x4C LD C,IXH    0x4D LD C,IXL
//   0x54 LD D,IXH    0x55 LD D,IXL    0x5C LD E,IXH    0x5D LD E,IXL
//   0x60 LD IXH,B    0x61 LD IXH,C    0x62 LD IXH,D    0x63 LD IXH,E
//   0x64 LD IXH,IXH  0x65 LD IXH,IXL  0x67 LD IXH,A
//   0x68 LD IXL,B    0x69 LD IXL,C    0x6A LD IXL,D    0x6B LD IXL,E
//   0x6C LD IXL,IXH  0x6D LD IXL,IXL  0x6F LD IXL,A
//   0x26 LD IXH,n    0x2E LD IXL,n
//   0x7C LD A,IXH    0x7D LD A,IXL
// ============================================================
void handle_dd_fd_ld_ixhl(Z80& cpu) {
    uint8_t op  = cpu.current_opcode;
    int     dst = (op >> 3) & 7;   // bits 5-3
    int     src = op & 7;           // bits 2-0

    // Remap H(4) and L(5) in the operand fields to IXH(8)/IXL(9)
    auto remap = [](int r) { return (r == 4) ? 8 : (r == 5) ? 9 : r; };

    cpu.write_reg8(remap(dst), cpu.read_reg8(remap(src)));
    cpu.regs.Q = 0;
}

void handle_dd_fd_ld_ixhl_n(Z80& cpu) {
    // 0x26 = LD IXH,n ; 0x2E = LD IXL,n
    int     reg = (cpu.current_opcode == 0x26) ? 8 : 9;
    uint8_t val = cpu.read(cpu.regs.PC++);
    cpu.write_reg8(reg, val);
    cpu.regs.Q = 0;
}

// ============================================================
// DD/FD undocumented: unified ALU with IXH/IXL
//
// All eight ALU operations (ADD/ADC/SUB/SBC/AND/XOR/OR/CP)
// share the same flag logic as their register-operand forms.
// Opcode bits 5-3 select the operation; bit 2-0 = 4(IXH) or 5(IXL).
// ============================================================
void handle_dd_fd_alu_ixhl(Z80& cpu) {
    uint8_t op  = cpu.current_opcode;
    int     src = (op & 1) ? 9 : 8;  // 0x?4 → IXH(8), 0x?5 → IXL(9)
    uint8_t val = cpu.read_reg8(src);
    uint8_t a   = cpu.regs.A;
    int     alu = (op >> 3) & 7;

    switch (alu) {
        case 0: // ADD A,IXH/L
            cpu.regs.A = (a + val) & 0xFF;
            cpu.regs.F = calc_add_flags(a, val);
            break;
        case 1: { // ADC A,IXH/L
            uint8_t c  = (cpu.regs.F & Flags::C) ? 1 : 0;
            cpu.regs.A = (a + val + c) & 0xFF;
            cpu.regs.F = adc8_flags(a, val, c);
            break;
        }
        case 2: // SUB IXH/L
            cpu.regs.A = (a - val) & 0xFF;
            cpu.regs.F = calc_sub_flags(a, val);
            break;
        case 3: { // SBC A,IXH/L
            uint8_t c  = (cpu.regs.F & Flags::C) ? 1 : 0;
            cpu.regs.A = (a - val - c) & 0xFF;
            cpu.regs.F = sbc8_flags(a, val, c);
            break;
        }
        case 4: // AND IXH/L
            cpu.regs.A &= val;
            cpu.regs.F  = Flags::H
                        | (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3))
                        | (cpu.regs.A == 0 ? Flags::Z : 0)
                        | FlagTables::PARITY_TABLE[cpu.regs.A];
            break;
        case 5: // XOR IXH/L
            cpu.regs.A ^= val;
            cpu.regs.F  = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3))
                        | (cpu.regs.A == 0 ? Flags::Z : 0)
                        | FlagTables::PARITY_TABLE[cpu.regs.A];
            break;
        case 6: // OR IXH/L
            cpu.regs.A |= val;
            cpu.regs.F  = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3))
                        | (cpu.regs.A == 0 ? Flags::Z : 0)
                        | FlagTables::PARITY_TABLE[cpu.regs.A];
            break;
        case 7: { // CP IXH/L — F3/F5 from operand
            uint8_t f = calc_sub_flags(a, val);
            f &= ~(Flags::F5 | Flags::F3);
            f |= val & (Flags::F5 | Flags::F3);
            cpu.regs.F = f;
            break;
        }
    }
    cpu.regs.Q = cpu.regs.F;
}

// ============================================================
// DD/FD: ALU with (IX+d)/(IY+d)  — 19 T-states each
// ============================================================
void handle_dd_fd_alu_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t   offset = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    uint8_t  val    = cpu.read(addr);
    uint8_t  a      = cpu.regs.A;
    int      alu    = (cpu.current_opcode >> 3) & 7;

    switch (alu) {
        case 0: // ADD
            cpu.regs.A = (a + val) & 0xFF;
            cpu.regs.F = calc_add_flags(a, val);
            break;
        case 1: { // ADC
            uint8_t c  = (cpu.regs.F & Flags::C) ? 1 : 0;
            cpu.regs.A = (a + val + c) & 0xFF;
            cpu.regs.F = adc8_flags(a, val, c);
            break;
        }
        case 2: // SUB
            cpu.regs.A = (a - val) & 0xFF;
            cpu.regs.F = calc_sub_flags(a, val);
            break;
        case 3: { // SBC
            uint8_t c  = (cpu.regs.F & Flags::C) ? 1 : 0;
            cpu.regs.A = (a - val - c) & 0xFF;
            cpu.regs.F = sbc8_flags(a, val, c);
            break;
        }
        case 4: // AND
            cpu.regs.A &= val;
            cpu.regs.F  = Flags::H
                        | (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3))
                        | (cpu.regs.A == 0 ? Flags::Z : 0)
                        | FlagTables::PARITY_TABLE[cpu.regs.A];
            break;
        case 5: // XOR
            cpu.regs.A ^= val;
            cpu.regs.F  = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3))
                        | (cpu.regs.A == 0 ? Flags::Z : 0)
                        | FlagTables::PARITY_TABLE[cpu.regs.A];
            break;
        case 6: // OR
            cpu.regs.A |= val;
            cpu.regs.F  = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3))
                        | (cpu.regs.A == 0 ? Flags::Z : 0)
                        | FlagTables::PARITY_TABLE[cpu.regs.A];
            break;
        case 7: { // CP — F3/F5 from operand
            uint8_t f = calc_sub_flags(a, val);
            f &= ~(Flags::F5 | Flags::F3);
            f |= val & (Flags::F5 | Flags::F3);
            cpu.regs.F = f;
            break;
        }
    }
    cpu.regs.Q = cpu.regs.F;
}

// INC (IX+d)  — 23 T-states: 4+4+3+5+3+1+3
void handle_dd_fd_inc_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t   offset = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    uint8_t  val    = cpu.read(addr);
    cpu.wait(1);
    uint8_t  res    = (val + 1) & 0xFF;
    cpu.write(addr, res);
    cpu.regs.F  = (cpu.regs.F & Flags::C) | calc_inc_flags(val);
    cpu.regs.Q  = cpu.regs.F;
}

// DEC (IX+d)  — 23 T-states
void handle_dd_fd_dec_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t   offset = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    uint8_t  val    = cpu.read(addr);
    cpu.wait(1);
    uint8_t  res    = (val - 1) & 0xFF;
    cpu.write(addr, res);
    cpu.regs.F  = (cpu.regs.F & Flags::C) | calc_dec_flags(val);
    cpu.regs.Q  = cpu.regs.F;
}

// ============================================================
// ADC A, (IX+d) / ADC A, (IY+d) — DD/FD 8E / FD 8E
// ============================================================
void handle_dd_fd_adc_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t   offset = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    uint8_t  val    = cpu.read(addr);
    uint8_t  a      = cpu.regs.A;
    uint8_t  c      = (cpu.regs.F & Flags::C) ? 1 : 0;
    cpu.regs.A       = (a + val + c) & 0xFF;
    cpu.regs.F       = adc8_flags(a, val, c);
    cpu.regs.Q       = cpu.regs.F;
}

// ============================================================
// SBC A, (IX+d) / SBC A, (IY+d) — DD/FD 9E / FD 9E
// ============================================================
void handle_dd_fd_sbc_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t   offset = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    uint8_t  val    = cpu.read(addr);
    uint8_t  a      = cpu.regs.A;
    uint8_t  c      = (cpu.regs.F & Flags::C) ? 1 : 0;
    cpu.regs.A       = (a - val - c) & 0xFF;
    cpu.regs.F       = sbc8_flags(a, val, c);
    cpu.regs.Q       = cpu.regs.F;
}

} // namespace z80
