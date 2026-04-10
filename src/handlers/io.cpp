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
// SLL r - Shift Logical Left (Undocumented) - 8 T-states
// Like SLA but shifts in 0 (not carry)
// ------------------------------------------------
void handle_sll_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int reg = opcode & 7;
    uint8_t val = cpu.read_reg8(reg);
    
    uint8_t carry = (val >> 7) & 1;
    val = (val << 1);
    
    cpu.write_reg8(reg, val);
    cpu.regs.F = FlagTables::SZP_TABLE[val] | carry;
}

// ------------------------------------------------
// BIT b, r - Test bit - 8 T-states
// ------------------------------------------------
void handle_cb_bit(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int bit = (opcode >> 3) & 7;
    int reg = opcode & 7;
    uint8_t val = cpu.read_reg8(reg);
    
    uint8_t result = val & (1 << bit);
    cpu.regs.F = (cpu.regs.F & Flags::C) | Flags::H | 
                 (result ? 0 : Flags::Z) |
                 (result ? Flags::S : 0);
    
    // For bits 3 and 5, copy bit 3 of original value to F5
    if (bit == 3 || bit == 5) {
        cpu.regs.F = (cpu.regs.F & ~Flags::F5) | (val & Flags::F5);
    }
}

// ------------------------------------------------
// RES b, r - Reset bit - 8 T-states
// ------------------------------------------------
void handle_cb_res(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int bit = (opcode >> 3) & 7;
    int reg = opcode & 7;
    uint8_t val = cpu.read_reg8(reg);
    
    val = val & ~(1 << bit);
    cpu.write_reg8(reg, val);
}

// ------------------------------------------------
// SET b, r - Set bit - 8 T-states
// ------------------------------------------------
void handle_cb_set(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    int bit = (opcode >> 3) & 7;
    int reg = opcode & 7;
    uint8_t val = cpu.read_reg8(reg);
    
    val = val | (1 << bit);
    cpu.write_reg8(reg, val);
}

// ============================================================
// DDCB/FDCB Prefix Handlers (indexed bit operations)
// ============================================================

// ------------------------------------------------
// DDCB/FDCB rotate/shift - 23 T-states
// Note: DDCB/FDCB with index+d doesn't support SLL, only index+r
// ------------------------------------------------
void handle_ddcb_fdcb_rot(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t base = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (base + offset) & 0xFFFF;
    
    uint8_t val = cpu.read(addr);
    uint8_t carry = (cpu.regs.F & Flags::C) ? 1 : 0;
    uint8_t result;
    uint8_t new_carry;
    
    switch ((opcode >> 3) & 7) {
        case 0:  // RLC
            new_carry = (val >> 7) & 1;
            result = (val << 1) | new_carry;
            break;
        case 1:  // RRC
            new_carry = val & 1;
            result = (val >> 1) | (new_carry << 7);
            break;
        case 2:  // RL
            new_carry = (val >> 7) & 1;
            result = (val << 1) | carry;
            break;
        case 3:  // RR
            new_carry = val & 1;
            result = (val >> 1) | (carry << 7);
            break;
        case 4:  // SLA
            new_carry = (val >> 7) & 1;
            result = val << 1;
            break;
        case 5:  // SRA
            new_carry = val & 1;
            result = (val >> 1) | (val & 0x80);
            break;
        case 6:  // SRL
            new_carry = val & 1;
            result = val >> 1;
            break;
        default:
            result = val;
            new_carry = 0;
    }
    
    cpu.write(addr, result);
    cpu.regs.F = FlagTables::SZP_TABLE[result] | new_carry;
}

// ------------------------------------------------
// DDCB/FDCB BIT - 20 T-states
// ------------------------------------------------
void handle_ddcb_fdcb_bit(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t base = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (base + offset) & 0xFFFF;
    
    int bit = (opcode >> 3) & 7;
    uint8_t val = cpu.read(addr);
    uint8_t result = val & (1 << bit);
    
    cpu.regs.F = (cpu.regs.F & Flags::C) | Flags::H | 
                 (result ? 0 : Flags::Z) |
                 (result ? Flags::S : 0);
}

// ------------------------------------------------
// DDCB/FDCB RES - 23 T-states
// ------------------------------------------------
void handle_ddcb_fdcb_res(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t base = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (base + offset) & 0xFFFF;
    
    int bit = (opcode >> 3) & 7;
    uint8_t val = cpu.read(addr);
    val = val & ~(1 << bit);
    cpu.write(addr, val);
}

// ------------------------------------------------
// DDCB/FDCB SET - 23 T-states
// ------------------------------------------------
void handle_ddcb_fdcb_set(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t base = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    int8_t offset = (int8_t)cpu.fetch();
    uint16_t addr = (base + offset) & 0xFFFF;
    
    int bit = (opcode >> 3) & 7;
    uint8_t val = cpu.read(addr);
    val = val | (1 << bit);
    cpu.write(addr, val);
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