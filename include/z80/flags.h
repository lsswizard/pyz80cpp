#pragma once

#include <cstdint>
#include <cstddef>

namespace z80 {

// ============================================================
// Flag Bit Definitions
// ============================================================
namespace Flags {
    constexpr uint8_t S  = 0x80;  // Sign
    constexpr uint8_t Z  = 0x40;  // Zero
    constexpr uint8_t F5 = 0x20;  // Undocumented flag 5
    constexpr uint8_t H  = 0x10;  // Half carry
    constexpr uint8_t F3 = 0x08;  // Undocumented flag 3
    constexpr uint8_t PV = 0x04;  // Parity/Overflow
    constexpr uint8_t N  = 0x02;  // Add/Subtract
    constexpr uint8_t C  = 0x01;  // Carry
}

// ============================================================
// Flag Tables (defined in flags.cpp)
// ============================================================
namespace FlagTables {
    extern uint8_t PARITY_TABLE[256];
    extern uint8_t SZ_TABLE[256];
    extern uint8_t SZ53_TABLE[256];
    extern uint8_t SZP_TABLE[256];
    extern uint8_t SZ53P_TABLE[256];
    extern uint8_t SZHZP_TABLE[256];

    // Rotate/shift: [op][256], op: 0=RLC,1=RRC,2=RL,3=RR,4=SLA,5=SRA,6=SLL,7=SRL
    extern uint8_t ROT_RESULT[8][256];
    extern uint8_t ROT_CARRY[8][256];
    extern uint8_t RL_CARRY_0[256];
    extern uint8_t RL_CARRY_1[256];
    extern uint8_t RR_CARRY_0[256];
    extern uint8_t RR_CARRY_1[256];

    // 8-bit ALU
    extern uint8_t ADD_FLAGS[65536];
    extern uint8_t ADC_FLAGS[65536];
    extern uint8_t SUB_FLAGS[65536];
    extern uint8_t SBC_FLAGS[65536];
    extern uint8_t INC_FLAGS[256];
    extern uint8_t DEC_FLAGS[256];

    // DAA: index = (N << 10) | (H << 9) | (C << 8) | A
    extern uint8_t DAA_FULL_FLAGS[4096];

    // Condition: [flags << 3 | cc]
    extern bool COND_TABLE[2048];

    // Bit operations
    extern uint8_t BIT_MASK[8];
    extern uint8_t RES_MASK[8];

    // Initialize all tables
    void init();
}

// ============================================================
// Condition Codes
// ============================================================
inline bool check_condition(uint8_t flags, int cc) {
    // cc: 0=NZ, 1=Z, 2=NC, 3=C, 4=PO, 5=PE, 6=P, 7=M
    return FlagTables::COND_TABLE[(flags << 3) | (cc & 0x07)];
}

} // namespace z80