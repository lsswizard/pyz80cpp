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
        // JR cc opcodes: 0x20(NZ), 0x28(Z), 0x30(NC), 0x38(C)
        // Bits 3-4 encode the condition: (opcode >> 3) & 3
        // Actually JR opcodes are:
        //   0x20: 0010 0000 -> cc=0 (NZ)
        //   0x28: 0010 1000 -> cc=1 (Z)  
        //   0x30: 0011 0000 -> cc=2 (NC)
        //   0x38: 0011 1000 -> cc=3 (C)
        // Mask is (opcode >> 3) & 3 = 6 & 3 = 2... wait
        // Let me recalculate: 0x30 = 0b00110000
        // (0x30 >> 3) = 0b00000110 = 6
        // 6 & 3 = 2 = NC (not Z)...
        // 
        // Actually for JR cc: opcode bits 3-4 encode cc:
        //   bit 4 (0x10): invert for Z vs NZ  
        //   bit 3 (0x08): carry vs no-carry
        // The correct decoding is: ((opcode >> 3) & 3)
        // But 0x20=0010 0000, 0x28=0010 1000, 0x30=0011 0000, 0x38=0011 1000
        // So bits: nz=0,z=1,nc=2,c=3 = (opcode >> 3) & 3
        // 
        // 0x30 >> 3 = 6, 6 & 3 = 2 = cc=2 (NC), so it DECODES as NC
        // But we WANT it to be cc=0 (NZ)
        //
        // Actually JR NZ is 0x20, so for opcode 0x30:
        // This is wrong encoding. The correct formula is:
        // cc = (opcode >> 3) - 4 (because 0x20=0010 0000 shifts to 4)
        // No wait, let's use simpler: cc = (opcode >> 4) + ((opcode >> 3) & 1)
        // Actually: the 4 conditions are in 2 bits: bit 3 (C), bit 4 (Z)
        // cc = ((opcode >> 3) & 3) for JR 
        // For 0x30: 0x30 >> 3 = 0x06 = 6, 6 & 3 = 2
        // That's CC = 2 = NC (NOT Z!)
        //
        // The bug is the opcode is 0x30 which decodes to NC, not NZ!
        // Different opcodes map to different conditions:
        // 0x20(0010 0000) = cc 0 (NZ) 
        // 0x28(0010 1000) = cc 1 (Z)
        // 0x30(0011 0000) = cc 2 (NC)  
        // 0x38(0011 1000) = cc 3 (C)
        //
        // 0x30 is JR NC (not jump if carry), not JR NZ!
        // So our test is wrong - let's test with correct opcode
        
        if (cpu.check_condition((cpu.current_opcode >> 3) & 3)) {
            // JR cc taken: 12 T-states (7 for fetch + 5 for jump)
            cpu.wait(5);
            cpu.regs.PC = cpu.regs.MEMPTR = (cpu.regs.PC + offset) & 0xFFFF;
        }
        // When NOT taken: 7 T-states total (correct - already consumed 7 in table)
        // No additional wait needed
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

    void handle_di(Z80& cpu) {
        cpu.regs.IFF1 = cpu.regs.IFF2 = false;
    }

    void handle_ei(Z80& cpu) {
        // printf("DEBUG: EI executed at PC=0x%04X\n", cpu.regs.PC-1);
        cpu.regs.EI_PENDING = true;
    }

    void handle_im(Z80& cpu) {
        switch (cpu.current_opcode) {
            case 0x46: case 0x4E: case 0x66: case 0x6E: cpu.regs.IM = 0; break;
            case 0x56: case 0x76: cpu.regs.IM = 1; break;
            case 0x5E: case 0x7E: cpu.regs.IM = 2; break;
            default: break;
        }
    }

    void handle_reti(Z80& cpu) {
        cpu.regs.IFF1 = cpu.regs.IFF2;
        uint16_t lo = cpu.read(cpu.regs.SP++);
        uint16_t hi = cpu.read(cpu.regs.SP++);
        cpu.regs.PC = cpu.regs.MEMPTR = (hi << 8) | lo;
    }

    void handle_retn(Z80& cpu) {
        cpu.regs.IFF1 = cpu.regs.IFF2;
        uint16_t lo = cpu.read(cpu.regs.SP++);
        uint16_t hi = cpu.read(cpu.regs.SP++);
        cpu.regs.PC = cpu.regs.MEMPTR = (hi << 8) | lo;
    }

    void handle_rst(Z80& cpu) {
        uint8_t vector = cpu.current_opcode & 0x38;
        cpu.wait(1);
        cpu.write(--cpu.regs.SP, cpu.regs.PC >> 8);
        cpu.write(--cpu.regs.SP, cpu.regs.PC & 0xFF);
        cpu.regs.PC = cpu.regs.MEMPTR = vector;
    }

} // namespace z80
