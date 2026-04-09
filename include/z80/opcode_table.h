#pragma once

#include "z80.h"
#include <array>

namespace z80 {

// ============================================================
// Opcode Tables - static tables for instruction execution
// ============================================================

class OpcodeTable {
public:
    static constexpr size_t TABLE_SIZE = 256;

    // Get instruction for main opcode table
    static const Instruction& get_main(uint8_t opcode) {
        return main_table[opcode];
    }

    // Get instruction for CB prefix opcodes
    static const Instruction& get_cb(uint8_t opcode) {
        return cb_table[opcode];
    }

    // Get instruction for ED prefix opcodes
    static const Instruction& get_ed(uint8_t opcode) {
        return ed_table[opcode];
    }

    // Get instruction for DD (IX) prefix opcodes
    static const Instruction& get_dd(uint8_t opcode) {
        return dd_table[opcode];
    }

    // Get instruction for FD (IY) prefix opcodes
    static const Instruction& get_fd(uint8_t opcode) {
        return fd_table[opcode];
    }

    // Get instruction for DDCB prefix opcodes
    static const Instruction& get_ddcb(uint8_t opcode) {
        return ddcb_table[opcode];
    }

    // Get instruction for FDCB prefix opcodes
    static const Instruction& get_fdcb(uint8_t opcode) {
        return fdcb_table[opcode];
    }

    // Initialize all tables (setup handler pointers)
    static void init();

private:
    static std::array<Instruction, TABLE_SIZE> main_table;
    static std::array<Instruction, TABLE_SIZE> cb_table;
    static std::array<Instruction, TABLE_SIZE> ed_table;
    static std::array<Instruction, TABLE_SIZE> dd_table;
    static std::array<Instruction, TABLE_SIZE> fd_table;
    static std::array<Instruction, TABLE_SIZE> ddcb_table;
    static std::array<Instruction, TABLE_SIZE> fdcb_table;
};

// ============================================================
// Instruction Timings (T-states)
// ============================================================
namespace Timing {
    // Base timings
    constexpr uint8_t NOP = 4;
    constexpr uint8_t LD_R_R = 4;
    constexpr uint8_t LD_R_N = 7;
    constexpr uint8_t LD_R_HL = 7;
    constexpr uint8_t LD_HL_R = 7;
    constexpr uint8_t LD_HL_N = 10;
    constexpr uint8_t LD_A_BC = 7;
    constexpr uint8_t LD_A_DE = 7;
    constexpr uint8_t LD_A_NN = 13;
    constexpr uint8_t LD_BC_A = 7;
    constexpr uint8_t LD_DE_A = 7;
    constexpr uint8_t LD_NN_A = 13;
    constexpr uint8_t LD_RR_NN = 10;
    constexpr uint8_t LD_HL_NN = 16;
    constexpr uint8_t LD_NN_HL = 16;
    constexpr uint8_t LD_SP_HL = 6;
    constexpr uint8_t PUSH_RR = 11;
    constexpr uint8_t POP_RR = 10;
    constexpr uint8_t EX_DE_HL = 4;
    constexpr uint8_t EX_AF_AFP = 4;
    constexpr uint8_t EXX = 4;
    constexpr uint8_t EX_SP_HL = 19;  // CRITICAL: was 15, should be 19
    constexpr uint8_t ADD_A_R = 4;
    constexpr uint8_t ADD_A_N = 7;
    constexpr uint8_t ADD_A_HL = 7;
    constexpr uint8_t ADC_A_R = 4;
    constexpr uint8_t SUB_R = 4;
    constexpr uint8_t SBC_A_R = 4;
    constexpr uint8_t AND_R = 4;
    constexpr uint8_t OR_R = 4;
    constexpr uint8_t XOR_R = 4;
    constexpr uint8_t CP_R = 4;
    constexpr uint8_t INC_R = 4;
    constexpr uint8_t DEC_R = 4;
    constexpr uint8_t INC_HL = 11;
    constexpr uint8_t DEC_HL = 11;
    constexpr uint8_t ADD_HL_RR = 11;
    constexpr uint8_t INC_RR = 6;
    constexpr uint8_t DEC_RR = 6;
    constexpr uint8_t DAA = 4;
    constexpr uint8_t CPL = 4;
    constexpr uint8_t CCF = 4;
    constexpr uint8_t SCF = 4;
    constexpr uint8_t RLCA = 4;
    constexpr uint8_t RRCA = 4;
    constexpr uint8_t RLA = 4;
    constexpr uint8_t RRA = 4;
    constexpr uint8_t JP_NN = 10;
    constexpr uint8_t JP_CC_NN = 10;
    constexpr uint8_t JP_HL = 4;
    constexpr uint8_t JR_E = 12;
    constexpr uint8_t JR_CC_E = 12;
    constexpr uint8_t DJNZ_E = 13;
    constexpr uint8_t CALL_NN = 17;
    constexpr uint8_t CALL_CC_NN = 17;
    constexpr uint8_t RET = 10;
    constexpr uint8_t RET_CC = 11;
    constexpr uint8_t RST = 11;
    constexpr uint8_t IN_A_N = 11;
    constexpr uint8_t OUT_N_A = 11;
    constexpr uint8_t HALT = 4;

    // Block transfer instructions
    constexpr uint8_t LDI = 16;
    constexpr uint8_t LDIR = 21;     // First iteration
    constexpr uint8_t LDIR_CONT = 16; // Subsequent iterations
    constexpr uint8_t LDD = 16;
    constexpr uint8_t LDDR = 21;
    constexpr uint8_t CPI = 16;
    constexpr uint8_t CPIR = 21;
    constexpr uint8_t CPD = 16;
    constexpr uint8_t CPDR = 21;

    // I/O block instructions
    constexpr uint8_t INI = 12;
    constexpr uint8_t INIR = 12;     // First + loop
    constexpr uint8_t IND = 12;
    constexpr uint8_t INDR = 12;
    constexpr uint8_t OUTI = 12;
    constexpr uint8_t OTIR = 12;
    constexpr uint8_t OUTD = 12;
    constexpr uint8_t OTDR = 12;

    // ED prefix instructions
    constexpr uint8_t NEG = 8;
    constexpr uint8_t RETI = 14;
    constexpr uint8_t RETN = 14;
    constexpr uint8_t IM_0 = 8;
    constexpr uint8_t IM_1 = 8;
    constexpr uint8_t IM_2 = 8;
    constexpr uint8_t IN_R_C = 12;
    constexpr uint8_t OUT_C_R = 12;
    constexpr uint8_t LD_I_A = 9;
    constexpr uint8_t LD_R_A = 9;
    constexpr uint8_t LD_A_I = 9;
    constexpr uint8_t LD_A_R = 9;
    constexpr uint8_t RLD = 18;
    constexpr uint8_t RRD = 18;

    // 16-bit loads (ED prefix)
    constexpr uint8_t LD_BC_NN = 20;
    constexpr uint8_t LD_DE_NN = 20;
    constexpr uint8_t LD_HL_NN_ED = 20;
    constexpr uint8_t LD_NN_BC = 20;
    constexpr uint8_t LD_NN_DE = 20;
    constexpr uint8_t LD_NN_HL_ED = 20;
    constexpr uint8_t ADC_HL_RR = 15;
    constexpr uint8_t SBC_HL_RR = 15;

    // Indexed instructions
    constexpr uint8_t LD_IX_NN = 14;
    constexpr uint8_t LD_NN_IX = 20;
    constexpr uint8_t INC_IX = 10;
    constexpr uint8_t DEC_IX = 10;
    constexpr uint8_t ADD_IX_BC = 15;
    constexpr uint8_t LD_SP_IX = 10;
    constexpr uint8_t PUSH_IX = 15;
    constexpr uint8_t POP_IX = 14;
    constexpr uint8_t EX_SP_IX = 23;
    constexpr uint8_t JP_IX = 8;
    constexpr uint8_t LD_IXD_R = 19;
    constexpr uint8_t LD_R_IXD = 19;
    constexpr uint8_t INC_IXD = 23;
    constexpr uint8_t DEC_IXD = 23;
    constexpr uint8_t ADD_A_IXD = 19;

    // DDCB/FDCB instructions
    constexpr uint8_t DDCB_LD = 23;
    constexpr uint8_t DDCB_ROT = 23;
    constexpr uint8_t DDCB_BIT = 20;
    constexpr uint8_t DDCB_SET = 23;
    constexpr uint8_t DDCB_RES = 23;
}

} // namespace z80