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

} // namespace z80