#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

// ============================================================
// I/O Instructions
// ============================================================

// ------------------------------------------------
// IN A, (n) - 11 T-states
// ------------------------------------------------
void handle_in_a_n(Z80& cpu) {
    uint8_t port = cpu.fetch();
    uint8_t val = cpu.in((cpu.regs.A << 8) | port);
    cpu.regs.A = val;
}

// ------------------------------------------------
// OUT (n), A - 11 T-states
// ------------------------------------------------
void handle_out_n_a(Z80& cpu) {
    uint8_t port = cpu.fetch();
    cpu.out((cpu.regs.A << 8) | port, cpu.regs.A);
}

// ------------------------------------------------
// IN r, (C) - 12 T-states
// ------------------------------------------------
void handle_in_r_c(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 3) & 7;
    uint8_t val = cpu.in(cpu.regs.BC());
    
    if (reg == 6) {
        // IN (C) - flags only
        cpu.regs.F = FlagTables::SZP_TABLE[val] | Flags::H | Flags::N;
    } else {
        cpu.write_reg8(reg, val);
        cpu.regs.F = FlagTables::SZP_TABLE[val] | Flags::H | Flags::N;
    }
}

// ------------------------------------------------
// OUT (C), r - 12 T-states
// ------------------------------------------------
void handle_out_c_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = (opcode >> 3) & 7;
    uint8_t val = (reg == 6) ? 0 : cpu.read_reg8(reg);
    cpu.out(cpu.regs.BC(), val);
}

// ------------------------------------------------
// INI - 12 T-states (B != 0) / 12 T-states (B = 0)
// ------------------------------------------------
void handle_ini(Z80& cpu) {
    uint8_t val = cpu.in(cpu.regs.BC());
    cpu.write(cpu.regs.HL(), val);
    cpu.regs.set_HL(cpu.regs.HL() + 1);
    cpu.regs.B--;
    
    // Flags: Z is set if B becomes 0
    cpu.regs.F = (cpu.regs.B == 0) ? Flags::Z : 0;
    cpu.regs.F |= Flags::N;
    // H and C are undefined
}

// ------------------------------------------------
// INIR - 12 + 16 T-states (B != 0) / 12 T-states (B = 0)
// ------------------------------------------------
void handle_inir(Z80& cpu) {
    handle_ini(cpu);
    if (cpu.regs.B != 0) {
        cpu.add_cycles(16);
    }
}

// ------------------------------------------------
// IND - 12 T-states
// ------------------------------------------------
void handle_ind(Z80& cpu) {
    uint8_t val = cpu.in(cpu.regs.BC());
    cpu.write(cpu.regs.HL(), val);
    cpu.regs.set_HL(cpu.regs.HL() - 1);
    cpu.regs.B--;
    
    cpu.regs.F = (cpu.regs.B == 0) ? Flags::Z : 0;
    cpu.regs.F |= Flags::N;
}

// ------------------------------------------------
// INDR - 12 + 16 T-states
// ------------------------------------------------
void handle_indr(Z80& cpu) {
    handle_ind(cpu);
    if (cpu.regs.B != 0) {
        cpu.add_cycles(16);
    }
}

// ------------------------------------------------
// OUTI - 12 T-states
// ------------------------------------------------
void handle_outi(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    cpu.out(cpu.regs.BC(), val);
    cpu.regs.set_HL(cpu.regs.HL() + 1);
    cpu.regs.B--;
    
    cpu.regs.F = (cpu.regs.B == 0) ? Flags::Z : 0;
    cpu.regs.F |= Flags::N;
}

// ------------------------------------------------
// OTIR - 12 + 16 T-states
// ------------------------------------------------
void handle_otir(Z80& cpu) {
    handle_outi(cpu);
    if (cpu.regs.B != 0) {
        cpu.add_cycles(16);
    }
}

// ------------------------------------------------
// OUTD - 12 T-states
// ------------------------------------------------
void handle_outd(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    cpu.out(cpu.regs.BC(), val);
    cpu.regs.set_HL(cpu.regs.HL() - 1);
    cpu.regs.B--;
    
    cpu.regs.F = (cpu.regs.B == 0) ? Flags::Z : 0;
    cpu.regs.F |= Flags::N;
}

// ------------------------------------------------
// OTDR - 12 + 16 T-states
// ------------------------------------------------
void handle_otdr(Z80& cpu) {
    handle_outd(cpu);
    if (cpu.regs.B != 0) {
        cpu.add_cycles(16);
    }
}

// ============================================================
// Rotate Instructions
// ============================================================

// ------------------------------------------------
// RLC r - 8 T-states
// ------------------------------------------------
void handle_rlc_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = opcode & 7;
    uint8_t val = cpu.read_reg8(reg);
    
    uint8_t carry = (val >> 7) & 1;
    val = (val << 1) | carry;
    
    cpu.write_reg8(reg, val);
    cpu.regs.F = FlagTables::SZP_TABLE[val] | carry;
}

// ------------------------------------------------
// RRC r - 8 T-states
// ------------------------------------------------
void handle_rrc_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = opcode & 7;
    uint8_t val = cpu.read_reg8(reg);
    
    uint8_t carry = val & 1;
    val = (val >> 1) | (carry << 7);
    
    cpu.write_reg8(reg, val);
    cpu.regs.F = FlagTables::SZP_TABLE[val] | carry;
}

// ------------------------------------------------
// RL r - 8 T-states
// ------------------------------------------------
void handle_rl_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = opcode & 7;
    uint8_t val = cpu.read_reg8(reg);
    
    uint8_t carry = (cpu.regs.F & Flags::C) ? 1 : 0;
    uint8_t new_carry = (val >> 7) & 1;
    val = (val << 1) | carry;
    
    cpu.write_reg8(reg, val);
    cpu.regs.F = FlagTables::SZP_TABLE[val] | new_carry;
}

// ------------------------------------------------
// RR r - 8 T-states
// ------------------------------------------------
void handle_rr_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = opcode & 7;
    uint8_t val = cpu.read_reg8(reg);
    
    uint8_t carry = (cpu.regs.F & Flags::C) ? 0x80 : 0;
    uint8_t new_carry = val & 1;
    val = (val >> 1) | carry;
    
    cpu.write_reg8(reg, val);
    cpu.regs.F = FlagTables::SZP_TABLE[val] | new_carry;
}

// ------------------------------------------------
// SLA r - 8 T-states
// ------------------------------------------------
void handle_sla_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = opcode & 7;
    uint8_t val = cpu.read_reg8(reg);
    
    uint8_t carry = (val >> 7) & 1;
    val = val << 1;
    
    cpu.write_reg8(reg, val);
    cpu.regs.F = FlagTables::SZP_TABLE[val] | carry;
}

// ------------------------------------------------
// SRA r - 8 T-states
// ------------------------------------------------
void handle_sra_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = opcode & 7;
    uint8_t val = cpu.read_reg8(reg);
    
    uint8_t carry = val & 1;
    val = (val >> 1) | (val & 0x80);  // Preserve sign bit
    
    cpu.write_reg8(reg, val);
    cpu.regs.F = FlagTables::SZP_TABLE[val] | carry;
}

// ------------------------------------------------
// SRL r - 8 T-states
// ------------------------------------------------
void handle_srl_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = opcode & 7;
    uint8_t val = cpu.read_reg8(reg);
    
    uint8_t carry = val & 1;
    val = val >> 1;
    
    cpu.write_reg8(reg, val);
    cpu.regs.F = FlagTables::SZP_TABLE[val] | carry;
}

// ------------------------------------------------
// RLA - 4 T-states
// ------------------------------------------------
void handle_rla(Z80& cpu) {
    uint8_t carry = (cpu.regs.F & Flags::C) ? 1 : 0;
    uint8_t new_carry = (cpu.regs.A >> 7) & 1;
    cpu.regs.A = (cpu.regs.A << 1) | carry;
    cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) | new_carry;
}

// ------------------------------------------------
// RRA - 4 T-states
// ------------------------------------------------
void handle_rra(Z80& cpu) {
    uint8_t carry = (cpu.regs.F & Flags::C) ? 0x80 : 0;
    uint8_t new_carry = cpu.regs.A & 1;
    cpu.regs.A = (cpu.regs.A >> 1) | carry;
    cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) | new_carry;
}

// ------------------------------------------------
// RLCA - 4 T-states
// ------------------------------------------------
void handle_rlca(Z80& cpu) {
    uint8_t carry = (cpu.regs.A >> 7) & 1;
    cpu.regs.A = (cpu.regs.A << 1) | carry;
    cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) | carry;
}

// ------------------------------------------------
// RRCA - 4 T-states
// ------------------------------------------------
void handle_rrca(Z80& cpu) {
    uint8_t carry = cpu.regs.A & 1;
    cpu.regs.A = (cpu.regs.A >> 1) | (carry << 7);
    cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) | carry;
}

} // namespace z80