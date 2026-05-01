#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

    // ============================================================
    // I/O Inline Helpers
    // ============================================================

    static inline void update_in_block_flags(Z80& cpu, uint8_t val, uint8_t l_val) {
        // Calculate (value + L) for flags - uses L AFTER HL update
        uint16_t t = val + (l_val & 0xFF);
        uint8_t b_after = cpu.regs.B;
        uint8_t f = (b_after & Flags::S) | (b_after == 0 ? Flags::Z : 0);
        if (t > 0xFF) f |= (Flags::C | Flags::H);
        if (FlagTables::PARITY_TABLE[(t & 7) ^ b_after]) f |= Flags::PV;

        // F3/F5: From MEMPTR high byte (correct for block I/O)
        f |= ((cpu.regs.MEMPTR >> 8) & (Flags::F3 | Flags::F5));
        // N flag: bit 7 of the transferred value
        f |= (val & 0x80);

        cpu.regs.F = f;
    }

    static inline void set_rot_flags(Z80& cpu, uint8_t res, uint8_t carry) {
        uint8_t f = (res & (Flags::S | Flags::F5 | Flags::F3)) | carry;
        if (res == 0) f |= Flags::Z;
        if (FlagTables::PARITY_TABLE[res]) f |= Flags::PV;
        cpu.regs.F = f;
    }

    // ============================================================
    // Standard I/O
    // ============================================================

    void handle_in_a_n(Z80& cpu) {
        uint16_t full_port = (cpu.regs.A << 8) | cpu.read(cpu.regs.PC++);
        cpu.regs.A = cpu.in(full_port);
        cpu.regs.MEMPTR = (full_port + 1) & 0xFFFF;
    }

    void handle_out_n_a(Z80& cpu) {
        uint16_t full_port = (cpu.regs.A << 8) | cpu.read(cpu.regs.PC++);
        cpu.out(full_port, cpu.regs.A);
        cpu.regs.MEMPTR = ((full_port + 1) & 0xFF) | (cpu.regs.A << 8);
    }

    void handle_in_r_c(Z80& cpu) {
        int reg = (cpu.current_opcode >> 3) & 7;
        uint16_t port = (cpu.regs.A << 8) | cpu.regs.C;  // Correct 16-bit port address
        uint8_t val = cpu.in(port);
        cpu.regs.MEMPTR = (port + 1) & 0xFFFF;  // MEMPTR = port + 1
        if (reg != 6) cpu.write_reg8(reg, val);

        cpu.regs.F = (cpu.regs.F & Flags::C) | (val & Flags::S) | (val == 0 ? Flags::Z : 0) |
        (val & (Flags::F5 | Flags::F3)) | (FlagTables::PARITY_TABLE[val] ? Flags::PV : 0);
    }

    void handle_out_c_r(Z80& cpu) {
        int reg = (cpu.current_opcode >> 3) & 7;
        uint16_t port = (cpu.regs.A << 8) | cpu.regs.C;  // Correct 16-bit port address
        uint8_t val = (reg == 6) ? 0xFF : cpu.read_reg8(reg);
        cpu.out(port, val);
        cpu.regs.MEMPTR = (port + 1) & 0xFFFF;  // MEMPTR = port + 1
    }

    // ============================================================
    // Block I/O
    // ============================================================

    void handle_ini(Z80& cpu) {
        cpu.wait(1);
        uint16_t port = (cpu.regs.A << 8) | cpu.regs.C;
        uint8_t val = cpu.in(port);
        cpu.write(cpu.regs.HL(), val);
        cpu.regs.set_HL(cpu.regs.HL() + 1);
        cpu.regs.B--;
        cpu.regs.MEMPTR = (port + 1) & 0xFFFF;  // MEMPTR = port + 1
        update_in_block_flags(cpu, val, 1);
    }

    void handle_ind(Z80& cpu) {
        cpu.wait(1);
        uint16_t port = (cpu.regs.A << 8) | cpu.regs.C;
        uint8_t val = cpu.in(port);
        cpu.write(cpu.regs.HL(), val);
        cpu.regs.set_HL(cpu.regs.HL() - 1);
        cpu.regs.B--;
        cpu.regs.MEMPTR = (port - 1) & 0xFFFF;  // MEMPTR = port - 1
        update_in_block_flags(cpu, val, -1);
    }

    void handle_inir(Z80& cpu) { handle_ini(cpu); if (cpu.regs.B) { cpu.wait(5); cpu.regs.PC -= 2; } }
    void handle_indr(Z80& cpu) { handle_ind(cpu); if (cpu.regs.B) { cpu.wait(5); cpu.regs.PC -= 2; } }

    void handle_outi(Z80& cpu) {
        cpu.wait(1);
        uint16_t port = (cpu.regs.A << 8) | cpu.regs.C;
        uint8_t val = cpu.read(cpu.regs.HL());
        cpu.regs.B--;
        cpu.out(port, val);
        cpu.regs.set_HL(cpu.regs.HL() + 1);
        cpu.regs.MEMPTR = (port + 1) & 0xFFFF;  // MEMPTR = port + 1
        update_in_block_flags(cpu, val, cpu.regs.L);
    }

    void handle_outd(Z80& cpu) {
        cpu.wait(1);
        uint16_t port = (cpu.regs.A << 8) | cpu.regs.C;
        uint8_t val = cpu.read(cpu.regs.HL());
        cpu.regs.B--;
        cpu.out(port, val);
        cpu.regs.set_HL(cpu.regs.HL() - 1);
        cpu.regs.MEMPTR = (port - 1) & 0xFFFF;  // MEMPTR = port - 1
        update_in_block_flags(cpu, val, cpu.regs.L);
    }

    void handle_otir(Z80& cpu) { handle_outi(cpu); if (cpu.regs.B) { cpu.wait(5); cpu.regs.PC -= 2; } }
    void handle_otdr(Z80& cpu) { handle_outd(cpu); if (cpu.regs.B) { cpu.wait(5); cpu.regs.PC -= 2; } }

    // ============================================================
    // Bit / Shift / Rotate Instructions (CB)
    // ============================================================

    void handle_cb_bit(Z80& cpu) {
        int bit_pos = (cpu.current_opcode >> 3) & 7;
        int reg = cpu.current_opcode & 7;
        uint16_t addr;
        if (reg == 6) {
            cpu.wait(1);
            addr = cpu.regs.HL();
            cpu.regs.MEMPTR = addr;
        } else {
            addr = 0; // Not used for register BIT
        }
        uint8_t val = cpu.read_reg8(reg);
        uint8_t result = val & (1 << bit_pos);
        
        // Build flags:
        // Z: Set if tested bit is 0
        // S: Set ONLY for bit 7 (sign bit)
        // H: Always set
        // F5/F3: For (HL), from MEMPTR high byte (addr >> 8); for registers, from original value
        // PV: Always equals Z (parity of original value) for ALL bit positions
        uint8_t f = (cpu.regs.F & Flags::C) | Flags::H | (result == 0 ? Flags::Z : 0);
        
        // Sign flag: Only set if testing bit 7
        if (bit_pos == 7) f |= (result != 0) ? Flags::S : 0;
        
        // F5/F3: For (HL) come from MEMPTR high byte; for registers come from original value
        if (reg == 6) {
            // BIT (HL): F5/F3 from MEMPTR high byte
            f |= (cpu.regs.MEMPTR >> 8) & (Flags::F5 | Flags::F3);
        } else {
            // BIT r: F5/F3 from original value being tested
            f |= (val & (Flags::F5 | Flags::F3));
        }
        
        // PV: always equals Z (parity of original value) for ALL bit positions
        f |= FlagTables::PARITY_TABLE[val] ? Flags::PV : 0;
        
        cpu.regs.F = f;
    }

    void handle_cb_res(Z80& cpu) {
        int reg = cpu.current_opcode & 7;
        if (reg == 6) cpu.wait(1);
        cpu.write_reg8(reg, cpu.read_reg8(reg) & ~(1 << ((cpu.current_opcode >> 3) & 7)));
    }

    void handle_cb_set(Z80& cpu) {
        int reg = cpu.current_opcode & 7;
        if (reg == 6) cpu.wait(1);
        cpu.write_reg8(reg, cpu.read_reg8(reg) | (1 << ((cpu.current_opcode >> 3) & 7)));
    }

    #define ROT_IMPL(FUNC, EXPR) \
    void FUNC(Z80& cpu) { \
        int reg = cpu.current_opcode & 7; \
        if (reg == 6) cpu.wait(1); \
            uint8_t val = cpu.read_reg8(reg); \
            EXPR; \
            cpu.write_reg8(reg, res); \
            set_rot_flags(cpu, res, carry); \
    }

    ROT_IMPL(handle_rlc_r, uint8_t carry = val >> 7; uint8_t res = (val << 1) | carry)
    ROT_IMPL(handle_rrc_r, uint8_t carry = val & 1; uint8_t res = (val >> 1) | (carry << 7))
    ROT_IMPL(handle_rl_r,  uint8_t carry = (val >> 7); uint8_t res = (val << 1) | ((cpu.regs.F & Flags::C) ? 1 : 0))
    ROT_IMPL(handle_rr_r,  uint8_t carry = val & 1; uint8_t res = (val >> 1) | ((cpu.regs.F & Flags::C) ? 0x80 : 0))
    ROT_IMPL(handle_sla_r, uint8_t carry = val >> 7; uint8_t res = val << 1)
    ROT_IMPL(handle_sra_r, uint8_t carry = val & 1; uint8_t res = (val >> 1) | (val & 0x80))
    ROT_IMPL(handle_srl_r, uint8_t carry = val & 1; uint8_t res = val >> 1)
    ROT_IMPL(handle_sll_r, uint8_t carry = val >> 7; uint8_t res = (val << 1) | 1)

    // ============================================================
    // Bit / Shift / Rotate Indexed (DDCB/FDCB)
    // ============================================================

    void handle_ddcb_fdcb_rot(Z80& cpu) {
        uint16_t addr = ((cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY) + cpu.ddcb_displacement) & 0xFFFF;
        cpu.regs.MEMPTR = addr;
        uint8_t val = cpu.read(addr);
        cpu.wait(3);
        uint8_t carry_flag = (cpu.regs.F & Flags::C) ? 1 : 0;
        uint8_t res, new_c;

        switch ((cpu.ddcb_opcode >> 3) & 7) {
            case 0: new_c = val >> 7; res = (val << 1) | new_c; break;
            case 1: new_c = val & 1;  res = (val >> 1) | (new_c << 7); break;
            case 2: new_c = val >> 7; res = (val << 1) | carry_flag; break;
            case 3: new_c = val & 1;  res = (val >> 1) | (carry_flag << 7); break;
            case 4: new_c = val >> 7; res = val << 1; break;
            case 5: new_c = val & 1;  res = (val >> 1) | (val & 0x80); break;
            case 6: new_c = val >> 7; res = (val << 1) | 1; break;
            default:new_c = val & 1;  res = val >> 1; break;
        }

        cpu.write(addr, res);
        int reg = cpu.ddcb_opcode & 7;
        if (reg != 6) cpu.write_reg8(reg, res);
        set_rot_flags(cpu, res, new_c);
    }

    void handle_ddcb_fdcb_bit(Z80& cpu) {
        uint16_t addr = ((cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY) + cpu.ddcb_displacement) & 0xFFFF;
        cpu.regs.MEMPTR = addr;
        uint8_t val = cpu.read(addr);
        cpu.wait(3);  // Additional processing time for BIT
        int bit_pos = (cpu.ddcb_opcode >> 3) & 7;
        uint8_t result = val & (1 << bit_pos);
        
        uint8_t f = (cpu.regs.F & Flags::C) | Flags::H;
        if (result == 0) f |= (Flags::Z | Flags::PV);
        if (bit_pos == 7 && result) f |= Flags::S;
        
        // For indexed BIT instructions, F5/F3 are taken from the 
        // high byte of the effective address (MEMPTR).
        f |= ((addr >> 8) & (Flags::F5 | Flags::F3));
        cpu.regs.F = f;
    }

    void handle_ddcb_fdcb_res(Z80& cpu) {
        uint16_t addr = ((cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY) + cpu.ddcb_displacement) & 0xFFFF;
        cpu.regs.MEMPTR = addr;
        uint8_t val = cpu.read(addr);
        cpu.wait(3);
        val &= ~(1 << ((cpu.ddcb_opcode >> 3) & 7));
        cpu.write(addr, val);
        if ((cpu.ddcb_opcode & 7) != 6) cpu.write_reg8(cpu.ddcb_opcode & 7, val);
    }

    void handle_ddcb_fdcb_set(Z80& cpu) {
        uint16_t addr = ((cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY) + cpu.ddcb_displacement) & 0xFFFF;
        cpu.regs.MEMPTR = addr;
        uint8_t val = cpu.read(addr);
        cpu.wait(3);
        val |= (1 << ((cpu.ddcb_opcode >> 3) & 7));
        cpu.write(addr, val);
        if ((cpu.ddcb_opcode & 7) != 6) cpu.write_reg8(cpu.ddcb_opcode & 7, val);
    }

    // Accumulator Rotates
    void handle_rla(Z80& cpu)  {
        uint8_t c = (cpu.regs.A >> 7);
        cpu.regs.A = (cpu.regs.A << 1) | ((cpu.regs.F & Flags::C) ? 1 : 0);
        cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) | (cpu.regs.A & (Flags::F5 | Flags::F3)) | c;
        cpu.regs.Q = cpu.regs.F;
    }
    void handle_rra(Z80& cpu)  {
        uint8_t c = (cpu.regs.A & 1);
        cpu.regs.A = (cpu.regs.A >> 1) | ((cpu.regs.F & Flags::C) ? 0x80 : 0);
        cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) | (cpu.regs.A & (Flags::F5 | Flags::F3)) | c;
        cpu.regs.Q = cpu.regs.F;
    }
    void handle_rlca(Z80& cpu) {
        uint8_t c = (cpu.regs.A >> 7);
        cpu.regs.A = (cpu.regs.A << 1) | c;
        cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) | (cpu.regs.A & (Flags::F5 | Flags::F3)) | c;
        cpu.regs.Q = cpu.regs.F;
    }
    void handle_rrca(Z80& cpu) {
        uint8_t c = (cpu.regs.A & 1);
        cpu.regs.A = (cpu.regs.A >> 1) | (c << 7);
        cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::PV)) | (cpu.regs.A & (Flags::F5 | Flags::F3)) | c;
        cpu.regs.Q = cpu.regs.F;
    }

} // namespace z80
