#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

// ============================================================
// Arithmetic and Logic Instructions (ALU)
// ============================================================

void handle_add_a(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val = cpu.read_reg8(opcode & 7);
    uint16_t res = cpu.regs.A + val;
    uint8_t a = cpu.regs.A;
    uint8_t r = res & 0xFF;
    uint8_t h = ((a & 0x0F) + (val & 0x0F)) > 0x0F ? Flags::H : 0;
    uint8_t pv = (~(a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.A = r;
    cpu.regs.F = (r & (Flags::S | Flags::F5 | Flags::F3)) |
                 (r == 0 ? Flags::Z : 0) | h | pv |
                 (res > 0xFF ? Flags::C : 0);
    cpu.regs.Q = cpu.regs.F;
}

void handle_adc_a(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val    = cpu.read_reg8(opcode & 7);
    uint8_t carry  = (cpu.regs.F & Flags::C) ? 1 : 0;
    uint16_t res   = cpu.regs.A + val + carry;
    uint8_t a      = cpu.regs.A;
    uint8_t r      = res & 0xFF;
    uint8_t h      = ((a & 0x0F) + (val & 0x0F) + carry) > 0x0F ? Flags::H : 0;
    uint8_t pv     = (~(a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.A     = r;
    cpu.regs.F     = (r & (Flags::S | Flags::F5 | Flags::F3)) |
                     (r == 0 ? Flags::Z : 0) | h | pv |
                     (res > 0xFF ? Flags::C : 0);
    cpu.regs.Q = cpu.regs.F;
}

void handle_sub(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val    = cpu.read_reg8(opcode & 7);
    uint16_t res   = cpu.regs.A - val;
    uint8_t a      = cpu.regs.A;
    uint8_t r      = res & 0xFF;
    uint8_t h      = ((a & 0x0F) < (val & 0x0F)) ? Flags::H : 0;
    uint8_t pv     = ((a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.A     = r;
    cpu.regs.F     = Flags::N | (r & (Flags::S | Flags::F5 | Flags::F3)) |
                     (r == 0 ? Flags::Z : 0) | h | pv |
                     (res > 0xFF ? Flags::C : 0);
    cpu.regs.Q = cpu.regs.F;
}

void handle_sbc_a(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val    = cpu.read_reg8(opcode & 7);
    uint8_t carry  = (cpu.regs.F & Flags::C) ? 1 : 0;
    uint16_t res   = cpu.regs.A - val - carry;
    uint8_t a      = cpu.regs.A;
    uint8_t r      = res & 0xFF;
    uint8_t h      = ((a & 0x0F) < ((val & 0x0F) + carry)) ? Flags::H : 0;
    uint8_t pv     = ((a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.A     = r;
    cpu.regs.F     = Flags::N | (r & (Flags::S | Flags::F5 | Flags::F3)) |
                     (r == 0 ? Flags::Z : 0) | h | pv |
                     (res > 0xFF ? Flags::C : 0);
}

void handle_and(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val    = cpu.read_reg8(opcode & 7);
    cpu.regs.A    &= val;
    cpu.regs.F     = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                     (cpu.regs.A == 0 ? Flags::Z : 0) | Flags::H |
                     (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0);
}

void handle_or(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val    = cpu.read_reg8(opcode & 7);
    cpu.regs.A    |= val;
    cpu.regs.F     = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                     (cpu.regs.A == 0 ? Flags::Z : 0) |
                     (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0);
}

void handle_xor(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val    = cpu.read_reg8(opcode & 7);
    cpu.regs.A    ^= val;
    cpu.regs.F     = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                     (cpu.regs.A == 0 ? Flags::Z : 0) |
                     (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0);
}

void handle_cp(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val    = cpu.read_reg8(opcode & 7);
    uint16_t res   = cpu.regs.A - val;
    uint8_t a      = cpu.regs.A;
    uint8_t r      = res & 0xFF;
    uint8_t h      = ((a & 0x0F) < (val & 0x0F)) ? Flags::H : 0;
    uint8_t pv     = ((a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.F     = Flags::N | (r & Flags::S) | (val & (Flags::F5 | Flags::F3)) |
                     (r == 0 ? Flags::Z : 0) | h | pv |
                     (res > 0xFF ? Flags::C : 0);
}

void handle_add_a_n(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.PC++);
    uint16_t res = cpu.regs.A + val;
    uint8_t a = cpu.regs.A;
    uint8_t r = res & 0xFF;
    uint8_t h = ((a & 0x0F) + (val & 0x0F)) > 0x0F ? Flags::H : 0;
    uint8_t pv = (~(a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.A = r;
    cpu.regs.F = (r & (Flags::S | Flags::F5 | Flags::F3)) |
                 (r == 0 ? Flags::Z : 0) | h | pv |
                 (res > 0xFF ? Flags::C : 0);
}

void handle_adc_a_n(Z80& cpu) {
    uint8_t val   = cpu.read(cpu.regs.PC++);
    uint8_t carry = (cpu.regs.F & Flags::C) ? 1 : 0;
    uint16_t res  = cpu.regs.A + val + carry;
    uint8_t a     = cpu.regs.A;
    uint8_t r     = res & 0xFF;
    uint8_t h     = ((a & 0x0F) + (val & 0x0F) + carry) > 0x0F ? Flags::H : 0;
    uint8_t pv    = (~(a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.A    = r;
    cpu.regs.F    = (r & (Flags::S | Flags::F5 | Flags::F3)) |
                    (r == 0 ? Flags::Z : 0) | h | pv |
                    (res > 0xFF ? Flags::C : 0);
}

void handle_sub_n(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.PC++);
    uint16_t res = cpu.regs.A - val;
    uint8_t a = cpu.regs.A;
    uint8_t r = res & 0xFF;
    uint8_t h = ((a & 0x0F) < (val & 0x0F)) ? Flags::H : 0;
    uint8_t pv = ((a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.A = r;
    cpu.regs.F = Flags::N | (r & (Flags::S | Flags::F5 | Flags::F3)) |
                 (r == 0 ? Flags::Z : 0) | h | pv |
                 (res > 0xFF ? Flags::C : 0);
}

void handle_sbc_a_n(Z80& cpu) {
    uint8_t val   = cpu.read(cpu.regs.PC++);
    uint8_t carry = (cpu.regs.F & Flags::C) ? 1 : 0;
    uint16_t res  = cpu.regs.A - val - carry;
    uint8_t a     = cpu.regs.A;
    uint8_t r     = res & 0xFF;
    uint8_t h     = ((a & 0x0F) < ((val & 0x0F) + carry)) ? Flags::H : 0;
    uint8_t pv    = ((a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.A    = r;
    cpu.regs.F    = Flags::N | (r & (Flags::S | Flags::F5 | Flags::F3)) |
                    (r == 0 ? Flags::Z : 0) | h | pv |
                    (res > 0xFF ? Flags::C : 0);
}

void handle_and_n(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.PC++);
    cpu.regs.A &= val;
    cpu.regs.F  = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                  (cpu.regs.A == 0 ? Flags::Z : 0) | Flags::H |
                  (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0);
}

void handle_or_n(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.PC++);
    cpu.regs.A |= val;
    cpu.regs.F  = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                  (cpu.regs.A == 0 ? Flags::Z : 0) |
                  (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0);
}

void handle_xor_n(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.PC++);
    cpu.regs.A ^= val;
    cpu.regs.F  = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                  (cpu.regs.A == 0 ? Flags::Z : 0) |
                  (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0);
}

void handle_cp_n(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.PC++);
    uint16_t res = cpu.regs.A - val;
    uint8_t a = cpu.regs.A;
    uint8_t r = res & 0xFF;
    uint8_t h = ((a & 0x0F) < (val & 0x0F)) ? Flags::H : 0;
    uint8_t pv = ((a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.F = Flags::N | (r & Flags::S) | (val & (Flags::F5 | Flags::F3)) |
                 (r == 0 ? Flags::Z : 0) | h | pv |
                 (res > 0xFF ? Flags::C : 0);
}

void handle_add_a_hl(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    uint16_t res = cpu.regs.A + val;
    uint8_t a = cpu.regs.A;
    uint8_t r = res & 0xFF;
    uint8_t h = ((a & 0x0F) + (val & 0x0F)) > 0x0F ? Flags::H : 0;
    uint8_t pv = (~(a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.A = r;
    cpu.regs.F = (r & (Flags::S | Flags::F5 | Flags::F3)) |
                 (r == 0 ? Flags::Z : 0) | h | pv |
                 (res > 0xFF ? Flags::C : 0);
}

void handle_inc_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 3) & 7;
    uint8_t val = cpu.read_reg8(reg);
    uint8_t res = (val + 1) & 0xFF;
    cpu.write_reg8(reg, res);
    
    uint8_t f = cpu.regs.F & Flags::C; // Preserve Carry
    f |= (res & Flags::S);
    if (res == 0) f |= Flags::Z;
    if ((val & 0x0F) == 0x0F) f |= Flags::H;
    if (val == 0x7F) f |= Flags::PV;
    f |= (res & (Flags::F5 | Flags::F3));
    cpu.regs.F = f;
    cpu.regs.Q = f;
}

void handle_dec_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 3) & 7;
    uint8_t val = cpu.read_reg8(reg);
    uint8_t res = (val - 1) & 0xFF;
    cpu.write_reg8(reg, res);
    
    uint8_t f = (cpu.regs.F & Flags::C) | Flags::N; // Preserve Carry, set N
    f |= (res & Flags::S);
    if (res == 0) f |= Flags::Z;
    if ((val & 0x0F) == 0x00) f |= Flags::H;
    if (val == 0x80) f |= Flags::PV;
    f |= (res & (Flags::F5 | Flags::F3));
    cpu.regs.F = f;
    cpu.regs.Q = f;
}

void handle_inc_hl(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    cpu.wait(1);
    uint8_t res = (val + 1) & 0xFF;
    cpu.write(cpu.regs.HL(), res);
    
    uint8_t f = cpu.regs.F & Flags::C;
    f |= (res & Flags::S);
    if (res == 0) f |= Flags::Z;
    if ((val & 0x0F) == 0x0F) f |= Flags::H;
    if (val == 0x7F) f |= Flags::PV;
    f |= (res & (Flags::F5 | Flags::F3));
    cpu.regs.F = f;
    cpu.regs.Q = f;
}

void handle_dec_hl(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    cpu.wait(1);
    uint8_t res = (val - 1) & 0xFF;
    cpu.write(cpu.regs.HL(), res);
    
    uint8_t f = (cpu.regs.F & Flags::C) | Flags::N;
    f |= (res & Flags::S);
    if (res == 0) f |= Flags::Z;
    if ((val & 0x0F) == 0x00) f |= Flags::H;
    if (val == 0x80) f |= Flags::PV;
    f |= (res & (Flags::F5 | Flags::F3));
    cpu.regs.F = f;
    cpu.regs.Q = f;
}

void handle_add_hl_rr(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 4) & 3;
    uint16_t hl = cpu.regs.HL();
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = hl;            break;
        default: val = cpu.regs.SP;  break;
    }
    cpu.wait(7);
    uint32_t result = hl + val;
    uint8_t h = ((hl & 0x0FFF) + (val & 0x0FFF)) > 0x0FFF ? Flags::H : 0;
    cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) |
                 ((result >> 8) & (Flags::F5 | Flags::F3)) | h |
                 (result > 0xFFFF ? Flags::C : 0);
    cpu.regs.set_HL(result & 0xFFFF);
    cpu.regs.MEMPTR = hl + 1;
}

void handle_inc_rr(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    cpu.wait(2);
    switch ((opcode >> 4) & 3) {
        case 0: cpu.regs.set_BC(cpu.regs.BC() + 1); break;
        case 1: cpu.regs.set_DE(cpu.regs.DE() + 1); break;
        case 2: cpu.regs.set_HL(cpu.regs.HL() + 1); break;
        case 3: cpu.regs.SP++; break;
    }
}

void handle_dec_rr(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    cpu.wait(2);
    switch ((opcode >> 4) & 3) {
        case 0: cpu.regs.set_BC(cpu.regs.BC() - 1); break;
        case 1: cpu.regs.set_DE(cpu.regs.DE() - 1); break;
        case 2: cpu.regs.set_HL(cpu.regs.HL() - 1); break;
        case 3: cpu.regs.SP--; break;
    }
}

void handle_daa(Z80& cpu) {
    uint8_t a = cpu.regs.A;
    uint8_t f = cpu.regs.F;
    uint8_t correction = 0;
    uint8_t new_c = f & Flags::C;
    uint8_t h = f & Flags::H;

    if ((a & 0x0F) > 9 || (f & Flags::H)) {
        correction |= 0x06;
    }
    if (a > 0x99 || (f & Flags::C)) {
        correction |= 0x60;
        new_c = Flags::C;
    }

    if (f & Flags::N) {
        cpu.regs.A -= correction;
    } else {
        cpu.regs.A += correction;
    }

    uint8_t res = cpu.regs.A;
    cpu.regs.F = (res & (Flags::S | Flags::F5 | Flags::F3)) |
                 (res == 0 ? Flags::Z : 0) |
                 ((f & Flags::N) ? ((h && (correction & 0x06) < 6) ? Flags::H : 0) : ((a & 0x0F) > 9 ? Flags::H : 0)) |
                 (FlagTables::PARITY_TABLE[res] ? Flags::PV : 0) |
                 (f & Flags::N) | new_c;
}

void handle_cpl(Z80& cpu) {
    cpu.regs.A = ~cpu.regs.A;
    cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV | Flags::C)) |
                 (cpu.regs.A & (Flags::F5 | Flags::F3)) | Flags::N | Flags::H;
}

void handle_ccf(Z80& cpu) {
    uint8_t old_c = cpu.regs.F & Flags::C;
    uint8_t q_xor_f = cpu.regs.Q ^ cpu.regs.F;
    cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) |
                 (q_xor_f | cpu.regs.A) & (Flags::F5 | Flags::F3) |
                 (old_c ? Flags::H : 0) |
                 (old_c ? 0 : Flags::C);
    // CCF sets Q = 0 (it's a non-flag-modifying instruction)
    cpu.regs.Q = 0;
}

void handle_scf(Z80& cpu) {
    uint8_t q_xor_f = cpu.regs.Q ^ cpu.regs.F;
    cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) |
                 (q_xor_f | cpu.regs.A) & (Flags::F5 | Flags::F3) |
                 Flags::C;
    // SCF sets Q = 0 (it's a non-flag-modifying instruction)
    cpu.regs.Q = 0;
}

void handle_nop(Z80& cpu) { 
    // NOP is a non-flag-modifying instruction, so Q = 0
    cpu.regs.Q = 0; 
}

void handle_halt(Z80& cpu) {
    cpu.halted = true;
    cpu.regs.PC = (cpu.regs.PC - 1) & 0xFFFF;
}

void handle_di(Z80& cpu) {
    cpu.regs.IFF1 = cpu.regs.IFF2 = false;
    cpu.regs.EI_PENDING = false;
    cpu.regs.EI_JUST_RESOLVED = false;
}

void handle_ei(Z80& cpu) {
    cpu.regs.EI_PENDING = true;
    cpu.regs.EI_JUST_RESOLVED = false;
}

void handle_adc_hl_rr(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 4) & 3;
    uint16_t hl = cpu.regs.HL();
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = hl;            break;
        default: val = cpu.regs.SP;  break;
    }
    cpu.wait(7);
    uint8_t carry   = (cpu.regs.F & Flags::C) ? 1 : 0;
    uint32_t result = hl + val + carry;
    uint8_t h  = ((hl & 0x0FFF) + (val & 0x0FFF) + carry) > 0x0FFF ? Flags::H : 0;
    uint8_t pv = (~(hl ^ val) & (hl ^ (uint16_t)result) & 0x8000) ? Flags::PV : 0;
    uint8_t r8 = (result >> 8) & 0xFF;
    cpu.regs.F = (r8 & (Flags::S | Flags::F5 | Flags::F3)) |
                 ((result & 0xFFFF) == 0 ? Flags::Z : 0) | h | pv |
                 (result > 0xFFFF ? Flags::C : 0);
    cpu.regs.set_HL(result & 0xFFFF);
    cpu.regs.MEMPTR = hl + 1;
}

void handle_sbc_hl_rr(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 4) & 3;
    uint16_t hl = cpu.regs.HL();
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = hl;            break;
        default: val = cpu.regs.SP;  break;
    }
    cpu.wait(7);
    uint8_t carry   = (cpu.regs.F & Flags::C) ? 1 : 0;
    uint32_t result = hl - val - carry;
    uint8_t h  = ((hl & 0x0FFF) < ((val & 0x0FFF) + carry)) ? Flags::H : 0;
    uint8_t pv = ((hl ^ val) & (hl ^ (uint16_t)result) & 0x8000) ? Flags::PV : 0;
    uint8_t r8 = (result >> 8) & 0xFF;
    cpu.regs.F = Flags::N |
                 (r8 & (Flags::S | Flags::F5 | Flags::F3)) |
                 ((result & 0xFFFF) == 0 ? Flags::Z : 0) | h | pv |
                 (result > 0xFFFF ? Flags::C : 0);
    cpu.regs.set_HL(result & 0xFFFF);
    cpu.regs.MEMPTR = hl + 1;
}

void handle_rld(Z80& cpu) {
    uint16_t addr = cpu.regs.HL();
    uint8_t val   = cpu.read(addr);
    cpu.wait(4);
    uint8_t acc   = cpu.regs.A;
    cpu.regs.A    = (acc & 0xF0) | (val >> 4);
    cpu.write(addr, (val << 4) | (acc & 0x0F));
    cpu.regs.F    = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                    (cpu.regs.A == 0 ? Flags::Z : 0) |
                    (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0) |
                    (cpu.regs.F & Flags::C);
    cpu.regs.MEMPTR = addr + 1;
}

void handle_rrd(Z80& cpu) {
    uint16_t addr = cpu.regs.HL();
    uint8_t val   = cpu.read(addr);
    cpu.wait(4);
    uint8_t acc   = cpu.regs.A;
    cpu.regs.A    = (acc & 0xF0) | (val & 0x0F);
    cpu.write(addr, (val >> 4) | (acc << 4));
    cpu.regs.F    = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                    (cpu.regs.A == 0 ? Flags::Z : 0) |
                    (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0) |
                    (cpu.regs.F & Flags::C);
    cpu.regs.MEMPTR = addr + 1;
}

void handle_neg(Z80& cpu) {
    uint8_t a = cpu.regs.A;
    uint16_t res = 0 - a;
    uint8_t r = res & 0xFF;
    cpu.regs.A = r;
    cpu.regs.F = Flags::N | (r & (Flags::S | Flags::F5 | Flags::F3)) |
                 (r == 0 ? Flags::Z : 0) |
                 ((0 & 0x0F) < (a & 0x0F) ? Flags::H : 0) |
                 (a == 0x80 ? Flags::PV : 0) |
                 (a != 0 ? Flags::C : 0);
}

void handle_reti(Z80& cpu) {
    cpu.regs.IFF1 = cpu.regs.IFF2;
    uint16_t lo = cpu.read(cpu.regs.SP++);
    uint16_t hi = cpu.read(cpu.regs.SP++);
    cpu.regs.PC = (hi << 8) | lo;
    cpu.regs.MEMPTR = cpu.regs.PC;
}

void handle_retn(Z80& cpu) {
    cpu.regs.IFF1 = cpu.regs.IFF2;
    uint16_t lo = cpu.read(cpu.regs.SP++);
    uint16_t hi = cpu.read(cpu.regs.SP++);
    cpu.regs.PC = (hi << 8) | lo;
    cpu.regs.MEMPTR = cpu.regs.PC;
}

void handle_im(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    cpu.regs.IM = (opcode == 0x56) ? 1 : ((opcode == 0x5E) ? 2 : 0);
}

void handle_ld_a_i(Z80& cpu) {
    cpu.wait(1);
    cpu.regs.A = cpu.regs.I;
    cpu.regs.F = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                 (cpu.regs.A == 0 ? Flags::Z : 0) |
                 (cpu.regs.IFF2 ? Flags::PV : 0) |
                 (cpu.regs.F & Flags::C);
}

void handle_ld_a_r(Z80& cpu) {
    cpu.wait(1);
    cpu.regs.A = cpu.regs.R;
    cpu.regs.F = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                 (cpu.regs.A == 0 ? Flags::Z : 0) |
                 (cpu.regs.IFF2 ? Flags::PV : 0) |
                 (cpu.regs.F & Flags::C);
}

void handle_ld_i_a(Z80& cpu) {
    cpu.wait(1);
    cpu.regs.I = cpu.regs.A;
}

void handle_ld_r_a(Z80& cpu) {
    cpu.wait(1);
    cpu.regs.R = cpu.regs.A;
}

void handle_dd_fd_ld_r_ixd(Z80& cpu) {
    uint8_t opcode  = cpu.current_opcode;
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset   = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    cpu.write_reg8((opcode >> 3) & 7, cpu.read(addr));
}

void handle_dd_fd_ld_ixd_r(Z80& cpu) {
    uint8_t opcode  = cpu.current_opcode;
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset   = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    cpu.write(addr, cpu.read_reg8(opcode & 7));
}

void handle_dd_fd_ld_ixd_n(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset   = (int8_t)cpu.read(cpu.regs.PC++);
    uint8_t val     = cpu.read(cpu.regs.PC++);
    cpu.wait(2);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    cpu.write(addr, val);
}

// ADD A, IXH/IXL (DD prefix)
void handle_dd_fd_add_a_ixhl(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode == 0x84) ? 8 : 9;  // 0x84=ADD A,IXH, 0x85=ADD A,IXL
    uint8_t val = cpu.read_reg8(reg);
    uint8_t a = cpu.regs.A;
    uint16_t res = a + val;
    uint8_t r = res & 0xFF;
    uint8_t h = ((a & 0x0F) + (val & 0x0F)) > 0x0F ? Flags::H : 0;
    uint8_t pv = (~(a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.A = r;
    cpu.regs.F = (r & (Flags::S | Flags::F5 | Flags::F3)) |
                 (r == 0 ? Flags::Z : 0) | h | pv |
                 (res > 0xFF ? Flags::C : 0);
}

// SUB A, IXH/IXL
void handle_dd_fd_sub_ixhl(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode == 0x94) ? 8 : 9;
    uint8_t val = cpu.read_reg8(reg);
    uint8_t a = cpu.regs.A;
    uint16_t res = a - val;
    uint8_t r = res & 0xFF;
    uint8_t h = (a & 0x0F) < (val & 0x0F) ? Flags::H : 0;
    uint8_t pv = (a ^ val) & (a ^ r) & 0x80 ? Flags::PV : 0;
    cpu.regs.A = r;
    cpu.regs.F = Flags::N | (r & (Flags::S | Flags::F5 | Flags::F3)) |
                 (r == 0 ? Flags::Z : 0) | h | pv |
                 (val > a ? Flags::C : 0);
}

// AND A, IXH/IXL  
void handle_dd_fd_and_ixhl(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode == 0xA4) ? 8 : 9;
    uint8_t val = cpu.read_reg8(reg);
    cpu.regs.A &= val;
    uint8_t r = cpu.regs.A;
    cpu.regs.F = Flags::H | (r & (Flags::S | Flags::F5 | Flags::F3)) |
                 (r == 0 ? Flags::Z : 0) | FlagTables::PARITY_TABLE[r];
}

// OR A, IXH/IXL
void handle_dd_fd_or_ixhl(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode == 0xB4) ? 8 : 9;
    uint8_t val = cpu.read_reg8(reg);
    cpu.regs.A |= val;
    uint8_t r = cpu.regs.A;
    cpu.regs.F = (r & (Flags::S | Flags::F5 | Flags::F3)) |
                 (r == 0 ? Flags::Z : 0) | FlagTables::PARITY_TABLE[r];
}

// XOR A, IXH/IXL
void handle_dd_fd_xor_ixhl(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode == 0xAC) ? 8 : 9;
    uint8_t val = cpu.read_reg8(reg);
    cpu.regs.A ^= val;
    uint8_t r = cpu.regs.A;
    cpu.regs.F = (r & (Flags::S | Flags::F5 | Flags::F3)) |
                 (r == 0 ? Flags::Z : 0) | FlagTables::PARITY_TABLE[r];
}

// CP A, IXH/IXL  
void handle_dd_fd_cp_ixhl(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode == 0xBC) ? 8 : 9;
    uint8_t val = cpu.read_reg8(reg);
    uint8_t a = cpu.regs.A;
    uint16_t res = a - val;
    uint8_t r = res & 0xFF;
    uint8_t h = (a & 0x0F) < (val & 0x0F) ? Flags::H : 0;
    uint8_t pv = (a ^ val) & (a ^ r) & 0x80 ? Flags::PV : 0;
    cpu.regs.F = Flags::N | (r & (Flags::S | Flags::F5 | Flags::F3)) |
                 (r == 0 ? Flags::Z : 0) | h | pv |
                 (val > a ? Flags::C : 0);
}

void handle_dd_fd_add_a_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset   = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    uint8_t val     = cpu.read(addr);
    uint16_t res    = cpu.regs.A + val;
    uint8_t a       = cpu.regs.A;
    uint8_t r       = res & 0xFF;
    uint8_t h       = ((a & 0x0F) + (val & 0x0F)) > 0x0F ? Flags::H : 0;
    uint8_t pv      = (~(a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.A      = r;
    cpu.regs.F      = (r & (Flags::S | Flags::F5 | Flags::F3)) |
                      (r == 0 ? Flags::Z : 0) | h | pv |
                      (res > 0xFF ? Flags::C : 0);
}

void handle_dd_fd_inc_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset   = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    uint8_t val     = cpu.read(addr);
    cpu.wait(1);
    uint8_t res     = (val + 1) & 0xFF;
    cpu.write(addr, res);
    uint8_t f = cpu.regs.F & Flags::C;
    f |= (res & Flags::S);
    if (res == 0) f |= Flags::Z;
    if ((val & 0x0F) == 0x0F) f |= Flags::H;
    if (val == 0x7F) f |= Flags::PV;
    f |= (res & (Flags::F5 | Flags::F3));
    cpu.regs.F = f;
}

void handle_dd_fd_dec_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset   = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    uint8_t val     = cpu.read(addr);
    cpu.wait(1);
    uint8_t res     = (val - 1) & 0xFF;
    cpu.write(addr, res);
    uint8_t f = (cpu.regs.F & Flags::C) | Flags::N;
    f |= (res & Flags::S);
    if (res == 0) f |= Flags::Z;
    if ((val & 0x0F) == 0x00) f |= Flags::H;
    if (val == 0x80) f |= Flags::PV;
    f |= (res & (Flags::F5 | Flags::F3));
    cpu.regs.F = f;
}

void handle_dd_fd_sub_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset   = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    uint8_t val     = cpu.read(addr);
    uint16_t res    = cpu.regs.A - val;
    uint8_t a       = cpu.regs.A;
    uint8_t r       = res & 0xFF;
    uint8_t h       = ((a & 0x0F) < (val & 0x0F)) ? Flags::H : 0;
    uint8_t pv      = ((a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.A      = r;
    cpu.regs.F      = Flags::N | (r & (Flags::S | Flags::F5 | Flags::F3)) |
                      (r == 0 ? Flags::Z : 0) | h | pv |
                      (res > 0xFF ? Flags::C : 0);
}

void handle_dd_fd_and_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset   = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    uint8_t val     = cpu.read(addr);
    cpu.regs.A     &= val;
    cpu.regs.F      = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                      (cpu.regs.A == 0 ? Flags::Z : 0) | Flags::H |
                      (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0);
}

void handle_dd_fd_or_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset   = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    uint8_t val     = cpu.read(addr);
    cpu.regs.A     |= val;
    cpu.regs.F      = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                      (cpu.regs.A == 0 ? Flags::Z : 0) |
                      (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0);
}

void handle_dd_fd_xor_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset   = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    uint8_t val     = cpu.read(addr);
    cpu.regs.A     ^= val;
    cpu.regs.F      = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                      (cpu.regs.A == 0 ? Flags::Z : 0) |
                      (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0);
}

void handle_dd_fd_cp_ixd(Z80& cpu) {
    uint16_t ix     = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset   = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);
    uint16_t addr   = (ix + offset) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    uint8_t val     = cpu.read(addr);
    uint16_t res    = cpu.regs.A - val;
    uint8_t a       = cpu.regs.A;
    uint8_t r       = res & 0xFF;
    uint8_t h       = ((a & 0x0F) < (val & 0x0F)) ? Flags::H : 0;
    uint8_t pv      = ((a ^ val) & (a ^ r) & 0x80) ? Flags::PV : 0;
    cpu.regs.F      = Flags::N | (r & Flags::S) | (val & (Flags::F5 | Flags::F3)) |
                      (r == 0 ? Flags::Z : 0) | h | pv |
                      (res > 0xFF ? Flags::C : 0);
}

void handle_dd_fd_ld_ix_nn(Z80& cpu) {
    uint8_t lo  = cpu.read(cpu.regs.PC++);
    uint8_t hi  = cpu.read(cpu.regs.PC++);
    uint16_t v  = (hi << 8) | lo;
    if (cpu.prefix_ix) cpu.regs.IX = v;
    else               cpu.regs.IY = v;
}

void handle_dd_fd_ld_nn_ix(Z80& cpu) {
    uint8_t lo   = cpu.read(cpu.regs.PC++);
    uint8_t hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (hi << 8) | lo;
    uint16_t val  = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    cpu.write(addr,     val & 0xFF);
    cpu.write(addr + 1, (val >> 8) & 0xFF);
    cpu.regs.MEMPTR = addr + 1;
}

void handle_dd_fd_ld_ix_nn_ind(Z80& cpu) {
    uint8_t lo   = cpu.read(cpu.regs.PC++);
    uint8_t hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (hi << 8) | lo;
    uint16_t val  = cpu.read(addr) | ((uint16_t)cpu.read(addr + 1) << 8);
    cpu.regs.MEMPTR = addr + 1;
    if (cpu.prefix_ix) cpu.regs.IX = val;
    else               cpu.regs.IY = val;
}

void handle_dd_fd_inc_ix(Z80& cpu) {
    cpu.wait(2);
    if (cpu.prefix_ix) cpu.regs.IX++;
    else               cpu.regs.IY++;
}

void handle_dd_fd_dec_ix(Z80& cpu) {
    cpu.wait(2);
    if (cpu.prefix_ix) cpu.regs.IX--;
    else               cpu.regs.IY--;
}

// INC IXH / INC IXL (DD prefix)
void handle_dd_fd_inc_ixhl(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    // opcode: 0x24 = INC IXH, 0x2C = INC IXL
    int reg = (opcode == 0x24) ? 8 : 9;  // 8=IXH, 9=IXL
    uint8_t val = cpu.read_reg8(reg);
    cpu.wait(1);
    uint8_t res = (val + 1) & 0xFF;
    cpu.write_reg8(reg, res);
    // Set flags
    uint8_t f = cpu.regs.F & Flags::C;
    f |= (res & (Flags::S | Flags::F5 | Flags::F3));
    if (res == 0) f |= Flags::Z;
    if (val == 0x7F) f |= Flags::PV;  // overflow
    if ((val & 0x0F) == 0x0F) f |= Flags::H;  // half carry
    cpu.regs.F = f;
    cpu.regs.Q = f;
}

// DEC IXH / DEC IXL
void handle_dd_fd_dec_ixhl(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode == 0x25) ? 8 : 9;  // 0x25=DEC IXH, 0x2D=DEC IXL
    uint8_t val = cpu.read_reg8(reg);
    cpu.wait(1);
    uint8_t res = (val - 1) & 0xFF;
    cpu.write_reg8(reg, res);
    // Set flags
    uint8_t f = (cpu.regs.F & Flags::C) | Flags::N;
    f |= (res & (Flags::S | Flags::F5 | Flags::F3));
    if (res == 0) f |= Flags::Z;
    if (val == 0x80) f |= Flags::PV;  // overflow
    if ((val & 0x0F) == 0x00) f |= Flags::H;  // half borrow
    cpu.regs.F = f;
    cpu.regs.Q = f;
}

void handle_dd_fd_add_ix_rr(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint16_t ix    = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    uint16_t val;
    switch ((opcode >> 4) & 3) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = ix;            break;
        default: val = cpu.regs.SP;  break;
    }
    cpu.wait(7);
    uint32_t result = ix + val;
    uint8_t h = ((ix & 0x0FFF) + (val & 0x0FFF)) > 0x0FFF ? Flags::H : 0;
    if (cpu.prefix_ix) cpu.regs.IX = result & 0xFFFF;
    else               cpu.regs.IY = result & 0xFFFF;
    cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) |
                 ((result >> 8) & (Flags::F5 | Flags::F3)) | h |
                 (result > 0xFFFF ? Flags::C : 0);
    cpu.regs.MEMPTR = ix + 1;
}

void handle_dd_fd_ld_sp_ix(Z80& cpu) {
    cpu.wait(2);
    cpu.regs.SP = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
}

void handle_dd_fd_push_ix(Z80& cpu) {
    uint16_t val = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    cpu.wait(1);
    cpu.write(--cpu.regs.SP, (val >> 8) & 0xFF);
    cpu.write(--cpu.regs.SP,  val & 0xFF);
}

void handle_dd_fd_pop_ix(Z80& cpu) {
    uint16_t lo = cpu.read(cpu.regs.SP++);
    uint16_t hi = cpu.read(cpu.regs.SP++);
    uint16_t v  = (hi << 8) | lo;
    if (cpu.prefix_ix) cpu.regs.IX = v;
    else               cpu.regs.IY = v;
}

void handle_dd_fd_ex_sp_ix(Z80& cpu) {
    uint16_t ix  = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    uint16_t sp  = cpu.regs.SP;
    uint16_t lo  = cpu.read(sp);
    uint16_t hi  = cpu.read(sp + 1);
    cpu.wait(1);
    uint16_t val = (hi << 8) | lo;
    cpu.write(sp + 1, (ix >> 8) & 0xFF);
    cpu.write(sp,      ix & 0xFF);
    cpu.wait(2);
    if (cpu.prefix_ix) cpu.regs.IX = val;
    else               cpu.regs.IY = val;
    cpu.regs.MEMPTR = val;
}

void handle_dd_fd_jp_ix(Z80& cpu) {
    cpu.regs.PC = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
}

} // namespace z80
