#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

// ============================================================
// Jump Instructions
// ============================================================

// ------------------------------------------------
// JP nn - 10 T-states
// ------------------------------------------------
void handle_jp_nn(Z80& cpu) {
    uint16_t addr = cpu.fetch();
    addr |= (cpu.fetch() << 8);
    cpu.regs.PC = addr;
    cpu.regs.MEMPTR = addr;
}

// ------------------------------------------------
// JP cc, nn - 10 T-states
// ------------------------------------------------
void handle_jp_cc_nn(Z80& cpu) {
    uint8_t cc = (cpu.current_opcode >> 3) & 7;
    uint16_t addr = cpu.fetch();
    addr |= (cpu.fetch() << 8);
    
    if (cpu.check_condition(cc)) {
        cpu.regs.PC = addr;
        cpu.regs.MEMPTR = addr;
    }
}

// ------------------------------------------------
// JP (HL) - 4 T-states
// ------------------------------------------------
void handle_jp_hl(Z80& cpu) {
    cpu.regs.PC = cpu.regs.HL();
}

// ------------------------------------------------
// JR e - 12 T-states
// ------------------------------------------------
void handle_jr_e(Z80& cpu) {
    int8_t offset = (int8_t)cpu.fetch();
    cpu.regs.PC = (cpu.regs.PC + offset) & 0xFFFF;
    cpu.regs.MEMPTR = cpu.regs.PC;
}

// ------------------------------------------------
// JR cc, e - 12 T-states (taken) / 7 T-states (not taken)
// ------------------------------------------------
void handle_jr_cc_e(Z80& cpu) {
    uint8_t cc = (cpu.current_opcode >> 3) & 3;
    if (cpu.check_condition(cc)) {
        int8_t offset = (int8_t)cpu.fetch();
        cpu.regs.PC = (cpu.regs.PC + offset) & 0xFFFF;
        cpu.regs.MEMPTR = cpu.regs.PC;
    } else {
        cpu.fetch();  // Skip offset byte
    }
}

// ------------------------------------------------
// DJNZ e - 13 T-states (B != 0) / 8 T-states (B = 0)
// ------------------------------------------------
void handle_djnz_e(Z80& cpu) {
    cpu.regs.B--;
    if (cpu.regs.B != 0) {
        int8_t offset = (int8_t)cpu.fetch();
        cpu.regs.PC = (cpu.regs.PC + offset) & 0xFFFF;
        cpu.regs.MEMPTR = cpu.regs.PC;
    } else {
        cpu.fetch();  // Skip offset byte
    }
}

// ============================================================
// Call and Return Instructions
// ============================================================

// ------------------------------------------------
// CALL nn - 17 T-states
// ------------------------------------------------
void handle_call_nn(Z80& cpu) {
    uint16_t addr = cpu.fetch();
    addr |= (cpu.fetch() << 8);
    cpu.regs.MEMPTR = addr;
    
    cpu.push(cpu.regs.PC);
    cpu.regs.PC = addr;
}

// ------------------------------------------------
// CALL cc, nn - 17 T-states (taken) / 10 T-states (not taken)
// ------------------------------------------------
void handle_call_cc_nn(Z80& cpu) {
    uint8_t cc = (cpu.current_opcode >> 3) & 7;
    uint16_t addr = cpu.fetch();
    addr |= (cpu.fetch() << 8);
    cpu.regs.MEMPTR = addr;
    
    if (cpu.check_condition(cc)) {
        cpu.push(cpu.regs.PC);
        cpu.regs.PC = addr;
    }
}

// ------------------------------------------------
// RET - 10 T-states
// ------------------------------------------------
void handle_ret(Z80& cpu) {
    cpu.regs.PC = cpu.pop();
}

// ------------------------------------------------
// RET cc - 11 T-states (taken) / 5 T-states (not taken)
// ------------------------------------------------
void handle_ret_cc(Z80& cpu) {
    uint8_t cc = (cpu.current_opcode >> 3) & 7;
    if (cpu.check_condition(cc)) {
        cpu.regs.PC = cpu.pop();
    }
}

// ------------------------------------------------
// RST n - 11 T-states
// ------------------------------------------------
void handle_rst(Z80& cpu) {
    uint8_t vector = cpu.current_opcode & 0x38;
    cpu.push(cpu.regs.PC);
    cpu.regs.PC = vector;
    cpu.regs.MEMPTR = vector;
}

} // namespace z80