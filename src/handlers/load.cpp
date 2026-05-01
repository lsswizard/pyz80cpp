#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

    // ============================================================
    // Load and Exchange Instructions
    // ============================================================

    void handle_ld_r_r(Z80& cpu)  { cpu.write_reg8((cpu.current_opcode >> 3) & 7, cpu.read_reg8(cpu.current_opcode & 7)); }
    void handle_ld_r_n(Z80& cpu)  { cpu.write_reg8((cpu.current_opcode >> 3) & 7, cpu.read(cpu.regs.PC++)); }
    void handle_ld_r_hl(Z80& cpu) { cpu.write_reg8((cpu.current_opcode >> 3) & 7, cpu.read(cpu.regs.HL())); }
    void handle_ld_hl_r(Z80& cpu) { cpu.write(cpu.regs.HL(), cpu.read_reg8(cpu.current_opcode & 7)); }
    void handle_ld_hl_n(Z80& cpu) { cpu.write(cpu.regs.HL(), cpu.read(cpu.regs.PC++)); }

    void handle_ld_ixhl_r(Z80& cpu) {
        uint8_t op = cpu.current_opcode;

        // Remap: dest register = (op >> 3) & 7 for r, IXH=8, IXL=9
        // Source register = op & 7 for r, or special case for A

        // Handle LD IXH, r (0x60-0x67) - dest IXH
        if (op >= 0x60 && op <= 0x67) {
            cpu.write_reg8(8, cpu.read_reg8(op & 7));  // IXH = 8
        }
        // Handle LD IXL, r (0x68-0x6F) - dest IXL
        else if (op >= 0x68 && op <= 0x6F) {
            cpu.write_reg8(9, cpu.read_reg8(op & 7));  // IXL = 9
        }
        // Handle LD r, IXH (0x44, 0x4C, 0x54, 0x5C, 0x64, 0x6C)
        else if ((op & 0x0C) == 0x04) {
            cpu.write_reg8((op >> 3) & 7, cpu.read_reg8(8));  // IXH = 8
        }
        // Handle LD r, IXL (0x45, 0x4D, 0x55, 0x5D, 0x65, 0x6D)
        else if ((op & 0x0C) == 0x0C) {
            cpu.write_reg8((op >> 3) & 7, cpu.read_reg8(9));  // IXL = 9
        }
        // Handle LD A, IXH (0x7C)
        else if (op == 0x7C) {
            cpu.regs.A = cpu.regs.IXh();
        }
        // Handle LD A, IXL (0x7D)
        else if (op == 0x7D) {
            cpu.regs.A = cpu.regs.IXl();
        }
        // Handle LD IXH, A (0x7F) - note: 0x7E is LD (IX+d),A
        else if (op == 0x7F) {
            cpu.regs.set_IXh(cpu.regs.A);
        }
        // Handle LD IXL, A - wait, there's no such opcode! 0x7F above handles this?
    }

    void handle_ld_ixhl_n(Z80& cpu) { cpu.write_reg8((cpu.current_opcode == 0x26) ? 8 : 9, cpu.read(cpu.regs.PC++)); }

    void handle_ld_a_bc(Z80& cpu) { cpu.regs.A = cpu.read(cpu.regs.BC()); cpu.regs.MEMPTR = (cpu.regs.BC() + 1) & 0xFFFF; }
    void handle_ld_a_de(Z80& cpu) { cpu.regs.A = cpu.read(cpu.regs.DE()); cpu.regs.MEMPTR = (cpu.regs.DE() + 1) & 0xFFFF; }
    void handle_ld_a_nn(Z80& cpu) {
        uint16_t lo = cpu.read(cpu.regs.PC++);
        uint16_t hi = cpu.read(cpu.regs.PC++);
        uint16_t addr = (hi << 8) | lo;
        cpu.regs.A = cpu.read(addr);
        cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    }

    void handle_ld_bc_a(Z80& cpu) { cpu.write(cpu.regs.BC(), cpu.regs.A); cpu.regs.MEMPTR = (cpu.regs.A << 8) | ((cpu.regs.BC() + 1) & 0xFF); }
    void handle_ld_de_a(Z80& cpu) { cpu.write(cpu.regs.DE(), cpu.regs.A); cpu.regs.MEMPTR = (cpu.regs.A << 8) | ((cpu.regs.DE() + 1) & 0xFF); }
    void handle_ld_nn_a(Z80& cpu) {
        uint16_t lo = cpu.read(cpu.regs.PC++);
        uint16_t hi = cpu.read(cpu.regs.PC++);
        uint16_t addr = (hi << 8) | lo;
        cpu.write(addr, cpu.regs.A);
        cpu.regs.MEMPTR = (cpu.regs.A << 8) | ((addr + 1) & 0xFF);
    }

    void handle_ld_rr_nn(Z80& cpu) {
        uint16_t lo = cpu.read(cpu.regs.PC++);
        uint16_t hi = cpu.read(cpu.regs.PC++);
        uint16_t val = (hi << 8) | lo;
        switch ((cpu.current_opcode >> 4) & 3) {
            case 0: cpu.regs.set_BC(val); break;
            case 1: cpu.regs.set_DE(val); break;
            case 2: cpu.regs.set_HL(val); break;
            case 3: cpu.regs.SP = val; break;
        }
    }

    void handle_ld_hl_nn(Z80& cpu) {
        uint16_t lo = cpu.read(cpu.regs.PC++);
        uint16_t hi = cpu.read(cpu.regs.PC++);
        uint16_t addr = (hi << 8) | lo;
        cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
        cpu.regs.set_HL(cpu.read(addr) | (cpu.read(addr + 1) << 8));
    }

    void handle_ld_nn_hl(Z80& cpu) {
        uint16_t lo = cpu.read(cpu.regs.PC++);
        uint16_t hi = cpu.read(cpu.regs.PC++);
        uint16_t addr = (hi << 8) | lo;
        cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
        cpu.write(addr, cpu.regs.L);
        cpu.write(addr + 1, cpu.regs.H);
    }

    void handle_ld_rr_nn_ind(Z80& cpu) {
        uint16_t lo = cpu.read(cpu.regs.PC++);
        uint16_t hi = cpu.read(cpu.regs.PC++);
        uint16_t addr = (hi << 8) | lo;
        cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
        uint16_t val = cpu.read(addr) | (cpu.read(addr + 1) << 8);
        switch ((cpu.current_opcode >> 4) & 3) {
            case 0: cpu.regs.set_BC(val); break;
            case 1: cpu.regs.set_DE(val); break;
            case 2: cpu.regs.set_HL(val); break;
            case 3: cpu.regs.SP = val; break;
        }
    }

    void handle_ld_nn_rr_ind(Z80& cpu) {
        uint16_t lo = cpu.read(cpu.regs.PC++);
        uint16_t hi = cpu.read(cpu.regs.PC++);
        uint16_t addr = (hi << 8) | lo;
        cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
        int reg = (cpu.current_opcode >> 4) & 3;
        uint16_t val = (reg == 0) ? cpu.regs.BC() : (reg == 1) ? cpu.regs.DE() : (reg == 2) ? cpu.regs.HL() : cpu.regs.SP;
        cpu.write(addr, val & 0xFF);
        cpu.write(addr + 1, val >> 8);
    }

    void handle_ld_sp_hl(Z80& cpu) { cpu.wait(2); cpu.regs.SP = cpu.regs.HL(); }

    void handle_push_rr(Z80& cpu) {
        int reg = (cpu.current_opcode >> 4) & 3;
        uint16_t val = (reg == 0) ? cpu.regs.BC() : (reg == 1) ? cpu.regs.DE() : (reg == 2) ? cpu.regs.HL() : cpu.regs.AF();
        cpu.wait(1);
        cpu.write(--cpu.regs.SP, val >> 8);
        cpu.write(--cpu.regs.SP, val & 0xFF);
    }

    void handle_pop_rr(Z80& cpu) {
        uint16_t lo = cpu.read(cpu.regs.SP++);
        uint16_t hi = cpu.read(cpu.regs.SP++);
        uint16_t val = (hi << 8) | lo;
        switch ((cpu.current_opcode >> 4) & 3) {
            case 0: cpu.regs.set_BC(val); break;
            case 1: cpu.regs.set_DE(val); break;
            case 2: cpu.regs.set_HL(val); break;
            default: cpu.regs.set_AF(val); break;
        }
    }

    void handle_ex_de_hl(Z80& cpu) { uint16_t de = cpu.regs.DE(); cpu.regs.set_DE(cpu.regs.HL()); cpu.regs.set_HL(de); }
    void handle_ex_af_afp(Z80& cpu) { cpu.regs.swap_af(); }
    void handle_exx(Z80& cpu) { cpu.regs.swap_all(); }

    void handle_ex_sp_hl(Z80& cpu) {
        uint16_t hl = cpu.regs.HL();
        uint16_t sp = cpu.regs.SP;
        uint16_t val = cpu.read(sp) | (cpu.read(sp + 1) << 8);
        cpu.wait(1);
        cpu.write(sp + 1, hl >> 8);
        cpu.write(sp, hl & 0xFF);
        cpu.wait(2);
        cpu.regs.set_HL(val);
        cpu.regs.MEMPTR = val;
    }

} // namespace z80
