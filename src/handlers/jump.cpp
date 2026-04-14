#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

    // ============================================================
    // Jump Instructions
    // ============================================================

    void handle_jp_nn(Z80& cpu) {
        uint16_t lo = cpu.read(cpu.regs.PC++);
        uint16_t hi = cpu.read(cpu.regs.PC++);
        uint16_t addr = (hi << 8) | lo;
        cpu.regs.PC = cpu.regs.MEMPTR = addr;
    }

    void handle_jp_cc_nn(Z80& cpu) {
        uint16_t lo = cpu.read(cpu.regs.PC++);
        uint16_t hi = cpu.read(cpu.regs.PC++);
        uint16_t addr = (hi << 8) | lo;
        cpu.regs.MEMPTR = addr;
        if (cpu.check_condition((cpu.current_opcode >> 3) & 7)) {
            cpu.regs.PC = addr;
        }
    }

    void handle_jp_hl(Z80& cpu) { cpu.regs.PC = cpu.regs.HL(); }

    void handle_jr_e(Z80& cpu) {
        int8_t offset = (int8_t)cpu.read(cpu.regs.PC++);
        cpu.wait(5);
        cpu.regs.PC = cpu.regs.MEMPTR = (cpu.regs.PC + offset) & 0xFFFF;
    }

    void handle_jr_cc_e(Z80& cpu) {
        int8_t offset = (int8_t)cpu.read(cpu.regs.PC++);
        if (cpu.check_condition((cpu.current_opcode >> 3) & 3)) {
            cpu.wait(5);
            cpu.regs.PC = cpu.regs.MEMPTR = (cpu.regs.PC + offset) & 0xFFFF;
        }
    }

    void handle_djnz_e(Z80& cpu) {
        cpu.wait(1);
        int8_t offset = (int8_t)cpu.read(cpu.regs.PC++);
        if (--cpu.regs.B != 0) {
            cpu.wait(5);
            cpu.regs.PC = cpu.regs.MEMPTR = (cpu.regs.PC + offset) & 0xFFFF;
        }
    }

    // ============================================================
    // Call and Return Instructions
    // ============================================================

    void handle_call_nn(Z80& cpu) {
        uint16_t lo = cpu.read(cpu.regs.PC++);
        uint16_t hi = cpu.read(cpu.regs.PC++);
        uint16_t addr = (hi << 8) | lo;
        cpu.regs.MEMPTR = addr;
        cpu.wait(1);
        cpu.write(--cpu.regs.SP, cpu.regs.PC >> 8);
        cpu.write(--cpu.regs.SP, cpu.regs.PC & 0xFF);
        cpu.regs.PC = addr;
    }

    void handle_call_cc_nn(Z80& cpu) {
        uint16_t lo = cpu.read(cpu.regs.PC++);
        uint16_t hi = cpu.read(cpu.regs.PC++);
        uint16_t addr = (hi << 8) | lo;
        cpu.regs.MEMPTR = addr;
        if (cpu.check_condition((cpu.current_opcode >> 3) & 7)) {
            cpu.wait(1);
            cpu.write(--cpu.regs.SP, cpu.regs.PC >> 8);
            cpu.write(--cpu.regs.SP, cpu.regs.PC & 0xFF);
            cpu.regs.PC = addr;
        }
    }

    void handle_ret(Z80& cpu) {
        uint16_t lo = cpu.read(cpu.regs.SP++);
        uint16_t hi = cpu.read(cpu.regs.SP++);
        cpu.regs.PC = cpu.regs.MEMPTR = (hi << 8) | lo;
    }

    void handle_ret_cc(Z80& cpu) {
        cpu.wait(1);
        if (cpu.check_condition((cpu.current_opcode >> 3) & 7)) {
            uint16_t lo = cpu.read(cpu.regs.SP++);
            uint16_t hi = cpu.read(cpu.regs.SP++);
            cpu.regs.PC = cpu.regs.MEMPTR = (hi << 8) | lo;
        }
    }

    void handle_rst(Z80& cpu) {
        uint8_t vector = cpu.current_opcode & 0x38;
        cpu.wait(1);
        cpu.write(--cpu.regs.SP, cpu.regs.PC >> 8);
        cpu.write(--cpu.regs.SP, cpu.regs.PC & 0xFF);
        cpu.regs.PC = cpu.regs.MEMPTR = vector;
    }

} // namespace z80
