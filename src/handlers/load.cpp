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
        if (op >= 0x44 && op <= 0x6D) {         // LD r, IXH/L
            cpu.write_reg8((op >> 3) & 7, cpu.read_reg8((op <= 0x6C) ? 8 : 9));
        } else if (op >= 0x60 && op <= 0x6F) {  // LD IXH/L, r
            cpu.write_reg8((op <= 0x67) ? 8 : 9, cpu.read_reg8(op & 7));
        }
    }

    void handle_ld_ixhl_n(Z80& cpu) { cpu.write_reg8((cpu.current_opcode == 0x26) ? 8 : 9, cpu.read(cpu.regs.PC++)); }

    void handle_ld_a_bc(Z80& cpu) { cpu.regs.A = cpu.read(cpu.regs.BC()); cpu.regs.MEMPTR = (cpu.regs.BC() + 1) & 0xFFFF; }
    void handle_ld_a_de(Z80& cpu) { cpu.regs.A = cpu.read(cpu.regs.DE()); cpu.regs.MEMPTR = (cpu.regs.DE() + 1) & 0xFFFF; }
    void handle_ld_a_nn(Z80& cpu) {
        uint16_t addr = cpu.read(cpu.regs.PC++) | (cpu.read(cpu.regs.PC++) << 8);
        cpu.regs.A = cpu.read(addr);
        cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    }

    void handle_ld_bc_a(Z80& cpu) { cpu.write(cpu.regs.BC(), cpu.regs.A); cpu.regs.MEMPTR = (cpu.regs.A << 8) | ((cpu.regs.BC() + 1) & 0xFF); }
    void handle_ld_de_a(Z80& cpu) { cpu.write(cpu.regs.DE(), cpu.regs.A); cpu.regs.MEMPTR = (cpu.regs.A << 8) | ((cpu.regs.DE() + 1) & 0xFF); }
    void handle_ld_nn_a(Z80& cpu) {
        uint16_t addr = cpu.read(cpu.regs.PC++) | (cpu.read(cpu.regs.PC++) << 8);
        cpu.write(addr, cpu.regs.A);
        cpu.regs.MEMPTR = (cpu.regs.A << 8) | ((addr + 1) & 0xFF);
    }

    void handle_ld_rr_nn(Z80& cpu) {
        uint16_t val = cpu.read(cpu.regs.PC++) | (cpu.read(cpu.regs.PC++) << 8);
        switch ((cpu.current_opcode >> 4) & 3) {
            case 0: cpu.regs.set_BC(val); break;
            case 1: cpu.regs.set_DE(val); break;
            case 2: cpu.regs.set_HL(val); break;
            case 3: cpu.regs.SP = val; break;
        }
    }

    void handle_ld_hl_nn(Z80& cpu) {
        uint16_t addr = cpu.read(cpu.regs.PC++) | (cpu.read(cpu.regs.PC++) << 8);
        cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
        cpu.regs.set_HL(cpu.read(addr) | (cpu.read(addr + 1) << 8));
    }

    void handle_ld_nn_hl(Z80& cpu) {
        uint16_t addr = cpu.read(cpu.regs.PC++) | (cpu.read(cpu.regs.PC++) << 8);
        cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
        cpu.write(addr, cpu.regs.L);
        cpu.write(addr + 1, cpu.regs.H);
    }

    void handle_ld_rr_nn_ind(Z80& cpu) {
        uint16_t addr = cpu.read(cpu.regs.PC++) | (cpu.read(cpu.regs.PC++) << 8);
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
        uint16_t addr = cpu.read(cpu.regs.PC++) | (cpu.read(cpu.regs.PC++) << 8);
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
        uint16_t val = cpu.read(cpu.regs.SP++) | (cpu.read(cpu.regs.SP++) << 8);
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
