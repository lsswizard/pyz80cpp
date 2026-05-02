#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

// ============================================================
// Arithmetic and Logic Instructions (ALU)
// ============================================================

void handle_add_a(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val = cpu.read_reg8(opcode & 7);
    uint8_t a = cpu.regs.A;
    uint16_t res = a + val;
    cpu.regs.A = res & 0xFF;
    // Use centralized flag calculation
    cpu.regs.F = calc_add_flags(a, val);
    cpu.regs.Q = cpu.regs.F;
}

void handle_adc_a(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val    = cpu.read_reg8(opcode & 7);
    uint8_t a      = cpu.regs.A;
    uint8_t carry  = (cpu.regs.F & Flags::C) ? 1 : 0;
    uint16_t res   = a + val + carry;
    cpu.regs.A     = res & 0xFF;
    // Use centralized flag calculation
    cpu.regs.F     = calc_adc_flags(a, val, carry, res);
    cpu.regs.Q = cpu.regs.F;
}

void handle_sub(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val    = cpu.read_reg8(opcode & 7);
    uint8_t a      = cpu.regs.A;
    uint16_t res   = a - val;
    cpu.regs.A     = res & 0xFF;
    // Use centralized flag calculation
    cpu.regs.F     = calc_sub_flags(a, val);
    cpu.regs.Q = cpu.regs.F;
}

void handle_sbc_a(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val    = cpu.read_reg8(opcode & 7);
    uint8_t a      = cpu.regs.A;
    uint8_t carry  = (cpu.regs.F & Flags::C) ? 1 : 0;
    uint16_t res   = a - val - carry;
    cpu.regs.A     = res & 0xFF;
    // Use centralized flag calculation
    cpu.regs.F     = calc_sbc_flags(a, val, carry, res);
    cpu.regs.Q = cpu.regs.F;
}

void handle_and(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val    = cpu.read_reg8(opcode & 7);
    cpu.regs.A    &= val;
    cpu.regs.Q     = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                     (cpu.regs.A == 0 ? Flags::Z : 0) | Flags::H |
                     (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0);
    cpu.regs.F = cpu.regs.Q;
}

void handle_or(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val    = cpu.read_reg8(opcode & 7);
    cpu.regs.A    |= val;
    cpu.regs.Q     = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                     (cpu.regs.A == 0 ? Flags::Z : 0) |
                     (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0);
    cpu.regs.F = cpu.regs.Q;
}

void handle_xor(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val    = cpu.read_reg8(opcode & 7);
    cpu.regs.A    ^= val;
    cpu.regs.Q     = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                     (cpu.regs.A == 0 ? Flags::Z : 0) |
                     (FlagTables::PARITY_TABLE[cpu.regs.A] ? Flags::PV : 0);
    cpu.regs.F = cpu.regs.Q;
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
    
    // Use centralized flag calculation (preserves carry)
    cpu.regs.F = (cpu.regs.F & Flags::C) | calc_inc_flags(val);
    cpu.regs.Q = cpu.regs.F;
}

void handle_dec_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 3) & 7;
    uint8_t val = cpu.read_reg8(reg);
    uint8_t res = (val - 1) & 0xFF;
    cpu.write_reg8(reg, res);
    
    // Use centralized flag calculation (preserves carry, sets N)
    cpu.regs.F = (cpu.regs.F & Flags::C) | calc_dec_flags(val);
    cpu.regs.Q = cpu.regs.F;
}

void handle_inc_hl(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    cpu.wait(1);
    uint8_t res = (val + 1) & 0xFF;
    cpu.write(cpu.regs.HL(), res);
    
    // Use centralized flag calculation (preserves carry)
    cpu.regs.F = (cpu.regs.F & Flags::C) | calc_inc_flags(val);
    cpu.regs.Q = cpu.regs.F;
}

void handle_dec_hl(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    cpu.wait(1);
    uint8_t res = (val - 1) & 0xFF;
    cpu.write(cpu.regs.HL(), res);
    
    // Use centralized flag calculation (preserves carry, sets N)
    cpu.regs.F = (cpu.regs.F & Flags::C) | calc_dec_flags(val);
    cpu.regs.Q = cpu.regs.F;
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

    // Determine if half-carry should be set after DAA
    // For addition: H is set if ((A & 0x0F) + (correction & 0x0F)) > 0x0F
    // For subtraction: H is set if ((A & 0x0F) - (correction & 0x0F)) < 0 (borrow)
    uint8_t h = 0;
    uint8_t al = a & 0x0F;

    if ((f & Flags::N) == 0) {
        // After addition
        if ((f & Flags::H) || al > 9) {
            correction |= 0x06;
        }
        if ((f & Flags::C) || a > 0x99) {
            correction |= 0x60;
            new_c = Flags::C;
        }
        // Half-carry after addition: was there a half-carry from the correction?
        if ((al + (correction & 0x0F)) > 0x0F) h = Flags::H;
    } else {
        // After subtraction
        if ((f & Flags::H) || al > 9) {
            correction |= 0x06;
        }
        if ((f & Flags::C) || a > 0x99) {
            correction |= 0x60;
            new_c = Flags::C;
        }
        // Half-carry after subtraction: was there a half-borrow from the correction?
        // H is set if lower nibble of A was < lower nibble of correction (before subtraction)
        if (al < (correction & 0x0F)) h = Flags::H;
    }

    if (f & Flags::N) {
        a -= correction;
    } else {
        a += correction;
    }

    cpu.regs.A = a;
    cpu.regs.F = (a & (Flags::S | Flags::F5 | Flags::F3)) |
                 (a == 0 ? Flags::Z : 0) | h |
                 (FlagTables::PARITY_TABLE[a] ? Flags::PV : 0) |
                 (f & Flags::N) | new_c;
    cpu.regs.Q = cpu.regs.F;
}

void handle_cpl(Z80& cpu) {
    cpu.regs.A = ~cpu.regs.A;
    cpu.regs.Q = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV | Flags::C)) |
                 (cpu.regs.A & (Flags::F5 | Flags::F3)) | Flags::N | Flags::H;
    cpu.regs.F = cpu.regs.Q;
}

    void handle_ccf(Z80& cpu) {
        uint8_t old_c = cpu.regs.F & Flags::C;
        // CCF: F3 = previous carry flag, F5 = A.5
        uint8_t f5_f3 = Flags::F5 & cpu.regs.A;  // F5 = A.5
        f5_f3 |= (old_c ? Flags::F3 : 0);  // F3 = previous carry
        
        cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) |
                     f5_f3 |
                     (old_c ? Flags::H : 0) |
                     (old_c ? 0 : Flags::C);
        // CCF sets Q = 0 (it's a non-flag-modifying instruction)
        cpu.regs.Q = 0;
    }

    void handle_scf(Z80& cpu) {
        // SCF: F5 = A.5, F3 = A.3 if previous instruction modified flags (Q != 0)
        //      Otherwise: F5 = YF (old F5), F3 = XF (old F3)
        uint8_t f5_f3;
        if (cpu.regs.Q) {
            // Previous instruction modified flags: use A.5, A.3
            f5_f3 = cpu.regs.A & (Flags::F5 | Flags::F3);
        } else {
            // Previous instruction did NOT modify flags: preserve old F5/F3
            f5_f3 = cpu.regs.F & (Flags::F5 | Flags::F3);
        }
        cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) |
                     f5_f3 |
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
    cpu.regs.Q = Flags::N | (r & (Flags::S | Flags::F5 | Flags::F3)) |
                 (r == 0 ? Flags::Z : 0) |
                 ((0 & 0x0F) < (a & 0x0F) ? Flags::H : 0) |
                 (a == 0x80 ? Flags::PV : 0) |
                 (a != 0 ? Flags::C : 0);
    cpu.regs.F = cpu.regs.Q;
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

// Unified ALU handler for IXH/IXL operations
// Handles: ADD, ADC, SUB, SBC, AND, OR, XOR, CP with IXH/IYH and IXL/IYL
// Opcodes:
//   0x84/0x85: ADD A, IXH/IXL - (opcode >> 4) & 0xF = 0x8, bit 3 = 0
//   0x8C/0x8D: ADC A, IXH/IXL - (opcode >> 4) & 0xF = 0x8, bit 3 = 1
//   0x94/0x95: SUB A, IXH/IXL - (opcode >> 4) & 0xF = 0x9, bit 3 = 0
//   0x9C/0x9D: SBC A, IXH/IXL - (opcode >> 4) & 0xF = 0x9, bit 3 = 1
//   0xA4/0xA5: AND A, IXH/IXL - (opcode >> 4) & 0xF = 0xA, bit 2 = 0
//   0xAC/0xAD: XOR A, IXH/IXL - (opcode >> 4) & 0xF = 0xA, bit 2 = 1
//   0xB4/0xB5: OR A, IXH/IXL - (opcode >> 4) & 0xF = 0xB, bit 2 = 0
//   0xBC/0xBD: CP A, IXH/IXL - (opcode >> 4) & 0xF = 0xB, bit 2 = 1
void handle_dd_fd_alu_ixhl(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;

    // Determine register: 8 = IXH/IYH, 9 = IXL/IYL (bit 0: 0=high, 1=low)
    int reg = (opcode & 1) ? 9 : 8;
    uint8_t val = cpu.read_reg8(reg);
    uint8_t a = cpu.regs.A;

    uint8_t op_group = (opcode >> 4) & 0x0F;  // 0x8, 0x9, 0xA, or 0xB

    if (op_group == 0x8) {
        // ADD or ADC (bit 3 determines which)
        if (opcode & 0x08) {
            // ADC: bit 3 = 1
            uint8_t carry = (cpu.regs.F & Flags::C) ? 1 : 0;
            uint16_t res = a + val + carry;
            cpu.regs.F = calc_adc_flags(a, val, carry, res);
            cpu.regs.A = res & 0xFF;
        } else {
            // ADD: bit 3 = 0
            uint16_t res = a + val;
            cpu.regs.F = calc_add_flags(a, val);
            cpu.regs.A = res & 0xFF;
        }
    } else if (op_group == 0x9) {
        // SUB or SBC (bit 3 determines which)
        if (opcode & 0x08) {
            // SBC: bit 3 = 1
            uint8_t carry = (cpu.regs.F & Flags::C) ? 1 : 0;
            uint16_t res = a - val - carry;
            cpu.regs.F = calc_sbc_flags(a, val, carry, res);
            cpu.regs.A = res & 0xFF;
        } else {
            // SUB: bit 3 = 0
            uint16_t res = a - val;
            cpu.regs.F = calc_sub_flags(a, val);
            cpu.regs.A = res & 0xFF;
        }
    } else if (op_group == 0xA) {
        // AND or XOR (bit 2 determines which)
        if (opcode & 0x04) {
            // XOR: bit 2 = 1
            cpu.regs.A ^= val;
        } else {
            // AND: bit 2 = 0
            cpu.regs.A &= val;
        }
        cpu.regs.F = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                     (cpu.regs.A == 0 ? Flags::Z : 0) | Flags::H |
                     FlagTables::PARITY_TABLE[cpu.regs.A];
    } else {  // op_group == 0xB
        // OR or CP (bit 2 determines which)
        if (opcode & 0x04) {
            // CP: bit 2 = 1 (doesn't modify A)
            cpu.regs.F = Flags::N | (calc_sub_flags(a, val) & ~(Flags::N)) | (val & (Flags::F5 | Flags::F3));
        } else {
            // OR: bit 2 = 0
            cpu.regs.A |= val;
            cpu.regs.F = (cpu.regs.A & (Flags::S | Flags::F5 | Flags::F3)) |
                         (cpu.regs.A == 0 ? Flags::Z : 0) |
                         FlagTables::PARITY_TABLE[cpu.regs.A];
        }
    }
    cpu.regs.Q = cpu.regs.F;
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

// Unified INC/DEC handler for IXH/IXL (and IYH/IYL)
// Handles: INC IXH, DEC IXH, INC IXL, DEC IXL
void handle_dd_fd_inc_dec_ixhl(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    // Determine register: even opcodes (0x24/0x25) = IXH, odd (0x2C/0x2D) = IXL
    int reg = (opcode & 0x08) ? 9 : 8;  // bit 3: 0 = IXH, 1 = IXL
    bool is_inc = (opcode & 1) == 0;    // bit 0: 0 = INC, 1 = DEC

    uint8_t val = cpu.read_reg8(reg);
    uint8_t res = is_inc ? (val + 1) & 0xFF : (val - 1) & 0xFF;
    cpu.write_reg8(reg, res);

    // Set flags using centralized helpers
    if (is_inc) {
        cpu.regs.F = (cpu.regs.F & Flags::C) | calc_inc_flags(val);
    } else {
        cpu.regs.F = (cpu.regs.F & Flags::C) | calc_dec_flags(val);
    }
    cpu.regs.Q = cpu.regs.F;
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
