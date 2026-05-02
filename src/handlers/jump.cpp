#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

// ============================================================
// Jump Instructions
// ============================================================

void handle_jp_nn(Z80& cpu) {
    uint8_t  lo   = cpu.read(cpu.regs.PC++);
    uint8_t  hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (uint16_t(hi) << 8) | lo;
    cpu.regs.PC     = addr;
    cpu.regs.MEMPTR = addr;
    cpu.regs.Q = 0;
}

void handle_jp_cc_nn(Z80& cpu) {
    uint8_t  lo   = cpu.read(cpu.regs.PC++);
    uint8_t  hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (uint16_t(hi) << 8) | lo;
    cpu.regs.MEMPTR = addr;   // MEMPTR updated regardless of condition
    if (cpu.check_condition((cpu.current_opcode >> 3) & 7))
        cpu.regs.PC = addr;
    cpu.regs.Q = 0;
}

void handle_jp_hl(Z80& cpu) {
    cpu.regs.PC = cpu.regs.HL();
    cpu.regs.Q  = 0;
}

// JR e — 12 T-states taken, 7 not taken
void handle_jr_e(Z80& cpu) {
    int8_t offset = (int8_t)cpu.read(cpu.regs.PC++);
    cpu.wait(5);   // extra 5 T-states for the jump (total: 4+3+5 = 12)
    cpu.regs.PC     = (cpu.regs.PC + offset) & 0xFFFF;
    cpu.regs.MEMPTR = cpu.regs.PC;
    cpu.regs.Q = 0;
}

// JR cc,e — 12 T-states taken, 7 not taken
void handle_jr_cc_e(Z80& cpu) {
    int8_t offset = (int8_t)cpu.read(cpu.regs.PC++);
    if (cpu.check_condition((cpu.current_opcode >> 3) & 3)) {
        cpu.wait(5);
        cpu.regs.PC     = (cpu.regs.PC + offset) & 0xFFFF;
        cpu.regs.MEMPTR = cpu.regs.PC;
    }
    cpu.regs.Q = 0;
}

// DJNZ e — 13 T-states if taken (4+1+3+5), 8 not taken (4+1+3)
void handle_djnz_e(Z80& cpu) {
    cpu.wait(1);   // extra M1 internal wait
    int8_t offset = (int8_t)cpu.read(cpu.regs.PC++);
    if (--cpu.regs.B != 0) {
        cpu.wait(5);
        cpu.regs.PC     = (cpu.regs.PC + offset) & 0xFFFF;
        cpu.regs.MEMPTR = cpu.regs.PC;
    }
    cpu.regs.Q = 0;
}

// ============================================================
// Call / Return
// ============================================================

// CALL nn — 17 T-states
void handle_call_nn(Z80& cpu) {
    uint8_t  lo   = cpu.read(cpu.regs.PC++);
    uint8_t  hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (uint16_t(hi) << 8) | lo;
    cpu.regs.MEMPTR = addr;
    cpu.wait(1);
    cpu.write(--cpu.regs.SP, cpu.regs.PC >> 8);
    cpu.write(--cpu.regs.SP, cpu.regs.PC & 0xFF);
    cpu.regs.PC = addr;
    cpu.regs.Q  = 0;
}

// CALL cc,nn — 17 if taken, 10 if not
void handle_call_cc_nn(Z80& cpu) {
    uint8_t  lo   = cpu.read(cpu.regs.PC++);
    uint8_t  hi   = cpu.read(cpu.regs.PC++);
    uint16_t addr = (uint16_t(hi) << 8) | lo;
    cpu.regs.MEMPTR = addr;
    if (cpu.check_condition((cpu.current_opcode >> 3) & 7)) {
        cpu.wait(1);
        cpu.write(--cpu.regs.SP, cpu.regs.PC >> 8);
        cpu.write(--cpu.regs.SP, cpu.regs.PC & 0xFF);
        cpu.regs.PC = addr;
    }
    cpu.regs.Q = 0;
}

// RET — 10 T-states
void handle_ret(Z80& cpu) {
    uint8_t lo = cpu.read(cpu.regs.SP++);
    uint8_t hi = cpu.read(cpu.regs.SP++);
    cpu.regs.PC     = (uint16_t(hi) << 8) | lo;
    cpu.regs.MEMPTR = cpu.regs.PC;
    cpu.regs.Q = 0;
}

// RET cc — 11 if taken, 5 if not
void handle_ret_cc(Z80& cpu) {
    cpu.wait(1);
    if (cpu.check_condition((cpu.current_opcode >> 3) & 7)) {
        uint8_t lo = cpu.read(cpu.regs.SP++);
        uint8_t hi = cpu.read(cpu.regs.SP++);
        cpu.regs.PC     = (uint16_t(hi) << 8) | lo;
        cpu.regs.MEMPTR = cpu.regs.PC;
    }
    cpu.regs.Q = 0;
}

// RST p — 11 T-states: 4(M1) + 1(int) + 3+3(push)
void handle_rst(Z80& cpu) {
    uint8_t vec = cpu.current_opcode & 0x38;
    cpu.wait(1);
    cpu.write(--cpu.regs.SP, cpu.regs.PC >> 8);
    cpu.write(--cpu.regs.SP, cpu.regs.PC & 0xFF);
    cpu.regs.PC     = vec;
    cpu.regs.MEMPTR = vec;
    cpu.regs.Q = 0;
}

// ============================================================
// Interrupt control
// ============================================================

void handle_di(Z80& cpu) {
    cpu.regs.IFF1 = cpu.regs.IFF2 = false;
    cpu.regs.Q = 0;
}

void handle_ei(Z80& cpu) {
    // Two-phase enable: IFF1/IFF2 set after the NEXT instruction
    cpu.regs.EI_PENDING = true;
    cpu.regs.Q = 0;
}

// IM 0 / IM 1 / IM 2
// Multiple ED opcodes map to the same mode (documented and undocumented variants)
void handle_im(Z80& cpu) {
    switch (cpu.current_opcode) {
        case 0x46: case 0x4E: case 0x66: case 0x6E:
            cpu.regs.IM = 0; break;
        case 0x56: case 0x76:
            cpu.regs.IM = 1; break;
        case 0x5E: case 0x7E:
            cpu.regs.IM = 2; break;
        default: break;
    }
    cpu.regs.Q = 0;
}

// RETN — return from NMI; restores IFF1 from IFF2
void handle_retn(Z80& cpu) {
    cpu.regs.IFF1 = cpu.regs.IFF2;
    uint8_t lo    = cpu.read(cpu.regs.SP++);
    uint8_t hi    = cpu.read(cpu.regs.SP++);
    cpu.regs.PC     = (uint16_t(hi) << 8) | lo;
    cpu.regs.MEMPTR = cpu.regs.PC;
    cpu.regs.Q = 0;
}

// RETI — return from maskable interrupt; also restores IFF1 from IFF2
// (on real hardware RETI signals the daisy-chain peripheral to release the bus;
//  here we just restore IFF1 which is correct for software behaviour)
void handle_reti(Z80& cpu) {
    cpu.regs.IFF1 = cpu.regs.IFF2;
    uint8_t lo    = cpu.read(cpu.regs.SP++);
    uint8_t hi    = cpu.read(cpu.regs.SP++);
    cpu.regs.PC     = (uint16_t(hi) << 8) | lo;
    cpu.regs.MEMPTR = cpu.regs.PC;
    cpu.regs.Q = 0;
}

} // namespace z80
