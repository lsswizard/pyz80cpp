#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

// ============================================================
// Arithmetic and Logic Instructions (ALU)
// ============================================================

// ------------------------------------------------
// ADD A, r - 4 T-states
// ------------------------------------------------
void handle_add_a(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val = cpu.read_reg8(opcode & 7);
    uint16_t result = cpu.regs.A + val;
    
    cpu.regs.F = FlagTables::ADD_FLAGS[(cpu.regs.A << 8) | val];
    cpu.regs.A = result & 0xFF;
}

// ------------------------------------------------
// ADC A, r - 4 T-states
// ------------------------------------------------
void handle_adc_a(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val = cpu.read_reg8(opcode & 7);
    uint16_t result = cpu.regs.A + val + (cpu.regs.F & Flags::C);
    
    cpu.regs.F = FlagTables::ADC_FLAGS[(cpu.regs.A << 8) | val | ((cpu.regs.F & Flags::C) << 8)];
    cpu.regs.A = result & 0xFF;
}

// ------------------------------------------------
// SUB r - 4 T-states
// ------------------------------------------------
void handle_sub(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val = cpu.read_reg8(opcode & 7);
    uint16_t result = cpu.regs.A - val;
    
    cpu.regs.F = FlagTables::SUB_FLAGS[(cpu.regs.A << 8) | val];
    cpu.regs.A = result & 0xFF;
}

// ------------------------------------------------
// SBC A, r - 4 T-states
// ------------------------------------------------
void handle_sbc_a(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val = cpu.read_reg8(opcode & 7);
    uint16_t result = cpu.regs.A - val - (cpu.regs.F & Flags::C);
    
    cpu.regs.F = FlagTables::SBC_FLAGS[(cpu.regs.A << 8) | val | ((cpu.regs.F & Flags::C) << 8)];
    cpu.regs.A = result & 0xFF;
}

// ------------------------------------------------
// AND r - 4 T-states
// ------------------------------------------------
void handle_and(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val = cpu.read_reg8(opcode & 7);
    cpu.regs.A &= val;
    cpu.regs.F = FlagTables::SZP_TABLE[cpu.regs.A] | Flags::H;
}

// ------------------------------------------------
// OR r - 4 T-states
// ------------------------------------------------
void handle_or(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val = cpu.read_reg8(opcode & 7);
    cpu.regs.A |= val;
    cpu.regs.F = FlagTables::SZP_TABLE[cpu.regs.A];
}

// ------------------------------------------------
// XOR r - 4 T-states
// ------------------------------------------------
void handle_xor(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val = cpu.read_reg8(opcode & 7);
    cpu.regs.A ^= val;
    cpu.regs.F = FlagTables::SZP_TABLE[cpu.regs.A];
}

// ------------------------------------------------
// CP r - 4 T-states
// ------------------------------------------------
void handle_cp(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t val = cpu.read_reg8(opcode & 7);
    
    cpu.regs.F = FlagTables::SUB_FLAGS[(cpu.regs.A << 8) | val];
}

// ------------------------------------------------
// ADD A, n - 7 T-states
// ------------------------------------------------
void handle_add_a_n(Z80& cpu) {
    uint8_t val = cpu.fetch();
    cpu.regs.F = FlagTables::ADD_FLAGS[(cpu.regs.A << 8) | val];
    cpu.regs.A = (cpu.regs.A + val) & 0xFF;
}

// ------------------------------------------------
// SUB n - 7 T-states
// ------------------------------------------------
void handle_sub_n(Z80& cpu) {
    uint8_t val = cpu.fetch();
    cpu.regs.F = FlagTables::SUB_FLAGS[(cpu.regs.A << 8) | val];
    cpu.regs.A = (cpu.regs.A - val) & 0xFF;
}

// ------------------------------------------------
// CP n - 7 T-states
// ------------------------------------------------
void handle_cp_n(Z80& cpu) {
    uint8_t val = cpu.fetch();
    cpu.regs.F = FlagTables::SUB_FLAGS[(cpu.regs.A << 8) | val];
}

// ------------------------------------------------
// ADD A, (HL) - 7 T-states
// ------------------------------------------------
void handle_add_a_hl(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    cpu.regs.F = FlagTables::ADD_FLAGS[(cpu.regs.A << 8) | val];
    cpu.regs.A = (cpu.regs.A + val) & 0xFF;
}

// ------------------------------------------------
// INC r - 4 T-states
// ------------------------------------------------
void handle_inc_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 3) & 7;
    uint8_t val = cpu.read_reg8(reg);
    val++;
    cpu.write_reg8(reg, val);
    cpu.regs.F = (cpu.regs.F & Flags::C) | FlagTables::INC_FLAGS[val];
}

// ------------------------------------------------
// DEC r - 4 T-states
// ------------------------------------------------
void handle_dec_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 3) & 7;
    uint8_t val = cpu.read_reg8(reg);
    val--;
    cpu.write_reg8(reg, val);
    cpu.regs.F = (cpu.regs.F & Flags::C) | FlagTables::DEC_FLAGS[val];
}

// ------------------------------------------------
// INC (HL) - 11 T-states
// ------------------------------------------------
void handle_inc_hl(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    val++;
    cpu.write(cpu.regs.HL(), val);
    cpu.regs.F = (cpu.regs.F & Flags::C) | FlagTables::INC_FLAGS[val];
}

// ------------------------------------------------
// DEC (HL) - 11 T-states
// ------------------------------------------------
void handle_dec_hl(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    val--;
    cpu.write(cpu.regs.HL(), val);
    cpu.regs.F = (cpu.regs.F & Flags::C) | FlagTables::DEC_FLAGS[val];
}

// ------------------------------------------------
// ADD HL, rr - 11 T-states
// ------------------------------------------------
void handle_add_hl_rr(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 4) & 3;
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = cpu.regs.HL(); break;
        case 3: val = cpu.regs.SP; break;
    }
    
    uint32_t result = cpu.regs.HL() + val;
    cpu.regs.F = (cpu.regs.F & 0xC4) | 
                 ((result >> 8) & 0x28) |
                 ((result >> 16) & 1);
    cpu.regs.set_HL(result & 0xFFFF);
}

// ------------------------------------------------
// INC rr - 6 T-states
// ------------------------------------------------
void handle_inc_rr(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 4) & 3;
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC() + 1; cpu.regs.set_BC(val); break;
        case 1: val = cpu.regs.DE() + 1; cpu.regs.set_DE(val); break;
        case 2: val = cpu.regs.HL() + 1; cpu.regs.set_HL(val); break;
        case 3: val = cpu.regs.SP + 1; cpu.regs.SP = val; break;
    }
}

// ------------------------------------------------
// DEC rr - 6 T-states
// ------------------------------------------------
void handle_dec_rr(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 4) & 3;
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC() - 1; cpu.regs.set_BC(val); break;
        case 1: val = cpu.regs.DE() - 1; cpu.regs.set_DE(val); break;
        case 2: val = cpu.regs.HL() - 1; cpu.regs.set_HL(val); break;
        case 3: val = cpu.regs.SP - 1; cpu.regs.SP = val; break;
    }
}

// ------------------------------------------------
// DAA - Decimal Adjust Accumulator - 4 T-states
// ------------------------------------------------
void handle_daa(Z80& cpu) {
    uint16_t a = cpu.regs.A;
    uint16_t correction = 0;
    
    if ((cpu.regs.F & Flags::H) || ((a & 0x0F) > 9)) {
        correction |= 0x06;
    }
    if ((cpu.regs.F & Flags::C) || (a > 0x99)) {
        correction |= 0x60;
        cpu.regs.F |= Flags::C;
    }
    
    if (cpu.regs.F & Flags::N) {
        a -= correction;
    } else {
        a += correction;
    }
    
    cpu.regs.A = a & 0xFF;
    cpu.regs.F = (cpu.regs.F & (Flags::C | Flags::N)) | 
                 FlagTables::SZP_TABLE[cpu.regs.A] |
                 ((a ^ correction) & Flags::H);
}

// ------------------------------------------------
// CPL - Complement A - 4 T-states
// ------------------------------------------------
void handle_cpl(Z80& cpu) {
    cpu.regs.A = ~cpu.regs.A;
    cpu.regs.F |= Flags::N | Flags::H;
}

// ------------------------------------------------
// CCF - Complement Carry Flag - 4 T-states
// ------------------------------------------------
void handle_ccf(Z80& cpu) {
    uint8_t c = cpu.regs.F & Flags::C;
    cpu.regs.F = (cpu.regs.F & 0xC4) | (c ? 0 : Flags::C) | (c ? Flags::H : 0);
}

// ------------------------------------------------
// SCF - Set Carry Flag - 4 T-states
// ------------------------------------------------
void handle_scf(Z80& cpu) {
    cpu.regs.F = (cpu.regs.F & 0xC4) | Flags::C;
}

// ------------------------------------------------
// NOP - 4 T-states
// ------------------------------------------------
void handle_nop(Z80& cpu) {
    (void)cpu;
}

// ------------------------------------------------
// HALT - 4 T-states
// ------------------------------------------------
void handle_halt(Z80& cpu) {
    cpu.halted = true;
    cpu.regs.PC = (cpu.regs.PC - 1) & 0xFFFF;
}

// ------------------------------------------------
// DI - Disable Interrupts - 4 T-states
// ------------------------------------------------
void handle_di(Z80& cpu) {
    cpu.regs.IFF1 = cpu.regs.IFF2 = false;
    cpu.regs.EI_PENDING = false;
}

// ------------------------------------------------
// EI - Enable Interrupts - 4 T-states
// ------------------------------------------------
void handle_ei(Z80& cpu) {
    cpu.regs.EI_PENDING = true;
    cpu.regs.EI_JUST_RESOLVED = false;
}

// ------------------------------------------------
// ADC HL, rr - Add with carry 16-bit - 15 T-states
// ------------------------------------------------
void handle_adc_hl_rr(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 4) & 3;
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = cpu.regs.HL(); break;
        case 3: val = cpu.regs.SP; break;
    }
    
    uint32_t result = cpu.regs.HL() + val + (cpu.regs.F & Flags::C);
    uint8_t h = ((cpu.regs.HL() & 0x0FFF) + (val & 0x0FFF) + (cpu.regs.F & Flags::C)) > 0x0FFF;
    uint8_t c = result > 0xFFFF;
    uint8_t pv = (~(cpu.regs.HL() ^ val) & (cpu.regs.HL() ^ result) & 0x8000) ? Flags::PV : 0;
    
    cpu.regs.F = ((result >> 8) & Flags::S) | 
                 ((result & 0xFFFF) == 0 ? Flags::Z : 0) |
                 ((result >> 8) & Flags::F5) |
                 (h ? Flags::H : 0) |
                 ((result >> 8) & Flags::F3) |
                 pv |
                 (c ? Flags::C : 0);
    cpu.regs.set_HL(result & 0xFFFF);
}

// ------------------------------------------------
// SBC HL, rr - Subtract with carry 16-bit - 15 T-states
// ------------------------------------------------
void handle_sbc_hl_rr(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 4) & 3;
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = cpu.regs.HL(); break;
        case 3: val = cpu.regs.SP; break;
    }
    
    uint32_t result = cpu.regs.HL() - val - (cpu.regs.F & Flags::C);
    uint8_t h = ((cpu.regs.HL() & 0x0FFF) < ((val & 0x0FFF) + (cpu.regs.F & Flags::C))) ? 1 : 0;
    uint8_t c = (result & 0x10000) ? Flags::C : 0;
    uint8_t pv = ((cpu.regs.HL() ^ val) & (cpu.regs.HL() ^ result) & 0x8000) ? Flags::PV : 0;
    
    cpu.regs.F = Flags::N | ((result >> 8) & Flags::S) | 
                 ((result & 0xFFFF) == 0 ? Flags::Z : 0) |
                 ((result >> 8) & Flags::F5) |
                 (h ? Flags::H : 0) |
                 ((result >> 8) & Flags::F3) |
                 pv |
                 c;
    cpu.regs.set_HL(result & 0xFFFF);
}

// ------------------------------------------------
// RLD - Rotate Left Digit - 18 T-states
// ------------------------------------------------
void handle_rld(Z80& cpu) {
    uint16_t addr = cpu.regs.HL();
    uint8_t val = cpu.read(addr);
    
    uint8_t acc = cpu.regs.A;
    cpu.regs.A = (acc & 0xF0) | (val >> 4);
    cpu.write(addr, (val << 4) | (acc & 0x0F));
    
    cpu.regs.F = FlagTables::SZP_TABLE[cpu.regs.A] | Flags::H | Flags::N;
}

// ------------------------------------------------
// RRD - Rotate Right Digit - 18 T-states
// ------------------------------------------------
void handle_rrd(Z80& cpu) {
    uint16_t addr = cpu.regs.HL();
    uint8_t val = cpu.read(addr);
    
    uint8_t acc = cpu.regs.A;
    cpu.regs.A = (acc & 0xF0) | (val & 0x0F);
    cpu.write(addr, (val >> 4) | (acc << 4));
    
    cpu.regs.F = FlagTables::SZP_TABLE[cpu.regs.A] | Flags::H | Flags::N;
}

// ------------------------------------------------
// NEG - Negate A - 8 T-states
// ------------------------------------------------
void handle_neg(Z80& cpu) {
    uint8_t a = cpu.regs.A;
    uint8_t result = 0 - a;
    
    cpu.regs.F = FlagTables::SUB_FLAGS[(0 << 8) | a];
    cpu.regs.A = result;
}

// ------------------------------------------------
// RETI - Return from Interrupt - 14 T-states
// ------------------------------------------------
void handle_reti(Z80& cpu) {
    cpu.regs.IFF1 = cpu.regs.IFF2;
    uint16_t lo = cpu.read(cpu.regs.SP++);
    uint16_t hi = cpu.read(cpu.regs.SP++);
    cpu.regs.PC = (hi << 8) | lo;
}

// ------------------------------------------------
// RETN - Return from NMI - 14 T-states
// ------------------------------------------------
void handle_retn(Z80& cpu) {
    cpu.regs.IFF1 = cpu.regs.IFF2;
    uint16_t lo = cpu.read(cpu.regs.SP++);
    uint16_t hi = cpu.read(cpu.regs.SP++);
    cpu.regs.PC = (hi << 8) | lo;
}

// ------------------------------------------------
// IM 0/1/2 - Set Interrupt Mode - 8 T-states
// ------------------------------------------------
void handle_im(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    cpu.regs.IM = (opcode == 0x56) ? 1 : ((opcode == 0x5E) ? 2 : 0);
}

// ------------------------------------------------
// LD A, I - Load A from I - 9 T-states
// ------------------------------------------------
void handle_ld_a_i(Z80& cpu) {
    cpu.regs.A = cpu.regs.I;
    cpu.regs.F = FlagTables::SZP_TABLE[cpu.regs.A] | 
                 (cpu.regs.IFF2 ? Flags::C : 0) | Flags::H | Flags::N;
}

// ------------------------------------------------
// LD A, R - Load A from R - 9 T-states
// ------------------------------------------------
void handle_ld_a_r(Z80& cpu) {
    cpu.regs.A = cpu.regs.R;
    cpu.regs.F = FlagTables::SZP_TABLE[cpu.regs.A] | 
                 (cpu.regs.IFF2 ? Flags::C : 0) | Flags::H | Flags::N;
}

// ------------------------------------------------
// LD I, A - Load I from A - 9 T-states
// ------------------------------------------------
void handle_ld_i_a(Z80& cpu) {
    cpu.regs.I = cpu.regs.A;
}

// ------------------------------------------------
// LD R, A - Load R from A - 9 T-states
// ------------------------------------------------
void handle_ld_r_a(Z80& cpu) {
    cpu.regs.R = cpu.regs.A;
}

// ============================================================
// DD/FD Indexed Operations (IX/IY + d)
// ============================================================

// ------------------------------------------------
// LD r, (IX+d) - 19 T-states
// ------------------------------------------------
void handle_dd_fd_ld_r_ixd(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint16_t ix = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (ix + offset) & 0xFFFF;
    
    int reg = (opcode >> 3) & 7;
    if (reg == 6) {
        // LD (IX+d), A handled differently - actually it's LD A, (IX+d)
        cpu.regs.A = cpu.read(addr);
    } else {
        cpu.write_reg8(reg, cpu.read(addr));
    }
}

// ------------------------------------------------
// LD (IX+d), r - 19 T-states
// ------------------------------------------------
void handle_dd_fd_ld_ixd_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint16_t ix = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (ix + offset) & 0xFFFF;
    
    int reg = opcode & 7;
    uint8_t val = cpu.read_reg8(reg);
    cpu.write(addr, val);
}

// ------------------------------------------------
// LD (IX+d), n - 19 T-states
// ------------------------------------------------
void handle_dd_fd_ld_ixd_n(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint16_t ix = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (ix + offset) & 0xFFFF;
    
    uint8_t val = cpu.fetch();
    cpu.write(addr, val);
}

// ------------------------------------------------
// ADD A, (IX+d) - 19 T-states
// ------------------------------------------------
void handle_dd_fd_add_a_ixd(Z80& cpu) {
    uint16_t ix = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (ix + offset) & 0xFFFF;
    
    uint8_t val = cpu.read(addr);
    uint16_t result = cpu.regs.A + val;
    cpu.regs.F = FlagTables::ADD_FLAGS[(cpu.regs.A << 8) | val];
    cpu.regs.A = result & 0xFF;
}

// ------------------------------------------------
// INC (IX+d) - 23 T-states
// ------------------------------------------------
void handle_dd_fd_inc_ixd(Z80& cpu) {
    uint16_t ix = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (ix + offset) & 0xFFFF;
    
    uint8_t val = cpu.read(addr);
    val++;
    cpu.write(addr, val);
    cpu.regs.F = (cpu.regs.F & Flags::C) | FlagTables::INC_FLAGS[val];
}

// ------------------------------------------------
// DEC (IX+d) - 23 T-states
// ------------------------------------------------
void handle_dd_fd_dec_ixd(Z80& cpu) {
    uint16_t ix = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (ix + offset) & 0xFFFF;
    
    uint8_t val = cpu.read(addr);
    val--;
    cpu.write(addr, val);
    cpu.regs.F = (cpu.regs.F & Flags::C) | FlagTables::DEC_FLAGS[val];
}

// ------------------------------------------------
// SUB (IX+d) - 19 T-states
// ------------------------------------------------
void handle_dd_fd_sub_ixd(Z80& cpu) {
    uint16_t ix = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (ix + offset) & 0xFFFF;
    
    uint8_t val = cpu.read(addr);
    cpu.regs.F = FlagTables::SUB_FLAGS[(cpu.regs.A << 8) | val];
    cpu.regs.A = (cpu.regs.A - val) & 0xFF;
}

// ------------------------------------------------
// AND (IX+d) - 19 T-states
// ------------------------------------------------
void handle_dd_fd_and_ixd(Z80& cpu) {
    uint16_t ix = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (ix + offset) & 0xFFFF;
    
    uint8_t val = cpu.read(addr);
    cpu.regs.A &= val;
    cpu.regs.F = FlagTables::SZP_TABLE[cpu.regs.A] | Flags::H;
}

// ------------------------------------------------
// OR (IX+d) - 19 T-states
// ------------------------------------------------
void handle_dd_fd_or_ixd(Z80& cpu) {
    uint16_t ix = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (ix + offset) & 0xFFFF;
    
    uint8_t val = cpu.read(addr);
    cpu.regs.A |= val;
    cpu.regs.F = FlagTables::SZP_TABLE[cpu.regs.A];
}

// ------------------------------------------------
// XOR (IX+d) - 19 T-states
// ------------------------------------------------
void handle_dd_fd_xor_ixd(Z80& cpu) {
    uint16_t ix = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (ix + offset) & 0xFFFF;
    
    uint8_t val = cpu.read(addr);
    cpu.regs.A ^= val;
    cpu.regs.F = FlagTables::SZP_TABLE[cpu.regs.A];
}

// ------------------------------------------------
// CP (IX+d) - 19 T-states
// ------------------------------------------------
void handle_dd_fd_cp_ixd(Z80& cpu) {
    uint16_t ix = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (ix + offset) & 0xFFFF;
    
    uint8_t val = cpu.read(addr);
    cpu.regs.F = FlagTables::SUB_FLAGS[(cpu.regs.A << 8) | val];
}

// ------------------------------------------------
// LD IX, nn - 14 T-states
// ------------------------------------------------
void handle_dd_fd_ld_ix_nn(Z80& cpu) {
    uint8_t lo = cpu.fetch();
    uint8_t hi = cpu.fetch();
    uint16_t val = (hi << 8) | lo;
    if (cpu.prefix_ix) {
        cpu.regs.IX = val;
    } else {
        cpu.regs.IY = val;
    }
}

// ------------------------------------------------
// LD (nn), IX - 20 T-states
// ------------------------------------------------
void handle_dd_fd_ld_nn_ix(Z80& cpu) {
    uint8_t lo = cpu.fetch();
    uint8_t hi = cpu.fetch();
    uint16_t addr = (hi << 8) | lo;
    uint16_t val = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    cpu.write(addr, val & 0xFF);
    cpu.write(addr + 1, (val >> 8) & 0xFF);
}

// ------------------------------------------------
// LD IX, (nn) - 20 T-states
// ------------------------------------------------
void handle_dd_fd_ld_ix_nn_ind(Z80& cpu) {
    uint8_t lo = cpu.fetch();
    uint8_t hi = cpu.fetch();
    uint16_t addr = (hi << 8) | lo;
    uint16_t val = cpu.read(addr) | (cpu.read(addr + 1) << 8);
    if (cpu.prefix_ix) {
        cpu.regs.IX = val;
    } else {
        cpu.regs.IY = val;
    }
}

// ------------------------------------------------
// INC IX - 10 T-states
// ------------------------------------------------
void handle_dd_fd_inc_ix(Z80& cpu) {
    if (cpu.prefix_ix) {
        cpu.regs.IX++;
    } else {
        cpu.regs.IY++;
    }
}

// ------------------------------------------------
// DEC IX - 10 T-states
// ------------------------------------------------
void handle_dd_fd_dec_ix(Z80& cpu) {
    if (cpu.prefix_ix) {
        cpu.regs.IX--;
    } else {
        cpu.regs.IY--;
    }
}

// ------------------------------------------------
// ADD IX, rr - 15 T-states
// ------------------------------------------------
void handle_dd_fd_add_ix_rr(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 4) & 3;
    uint16_t val;
    switch (reg) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY; break;
        case 3: val = cpu.regs.SP; break;
    }
    
    uint32_t result = (cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY) + val;
    if (cpu.prefix_ix) {
        cpu.regs.IX = result & 0xFFFF;
    } else {
        cpu.regs.IY = result & 0xFFFF;
    }
    uint8_t c = result > 0xFFFF;
    cpu.regs.F = (cpu.regs.F & 0xC4) | 
                 ((result >> 8) & 0x28) | c;
}

// ------------------------------------------------
// LD SP, IX - 10 T-states
// ------------------------------------------------
void handle_dd_fd_ld_sp_ix(Z80& cpu) {
    cpu.regs.SP = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
}

// ------------------------------------------------
// PUSH IX - 15 T-states
// ------------------------------------------------
void handle_dd_fd_push_ix(Z80& cpu) {
    uint16_t val = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    cpu.write(--cpu.regs.SP, (val >> 8) & 0xFF);
    cpu.write(--cpu.regs.SP, val & 0xFF);
}

// ------------------------------------------------
// POP IX - 14 T-states
// ------------------------------------------------
void handle_dd_fd_pop_ix(Z80& cpu) {
    uint16_t val = cpu.read(cpu.regs.SP++);
    val |= (cpu.read(cpu.regs.SP++) << 8);
    if (cpu.prefix_ix) {
        cpu.regs.IX = val;
    } else {
        cpu.regs.IY = val;
    }
}

// ------------------------------------------------
// EX (SP), IX - 23 T-states
// ------------------------------------------------
void handle_dd_fd_ex_sp_ix(Z80& cpu) {
    uint16_t temp = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    uint16_t sp = cpu.regs.SP;
    
    uint16_t val = cpu.read(sp) | (cpu.read(sp + 1) << 8);
    cpu.write(sp, temp & 0xFF);
    cpu.write(sp + 1, (temp >> 8) & 0xFF);
    
    if (cpu.prefix_ix) {
        cpu.regs.IX = val;
    } else {
        cpu.regs.IY = val;
    }
}

// ------------------------------------------------
// JP (IX) - 8 T-states
// ------------------------------------------------
void handle_dd_fd_jp_ix(Z80& cpu) {
    cpu.regs.PC = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
}

} // namespace z80