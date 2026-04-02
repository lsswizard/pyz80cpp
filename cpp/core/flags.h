#pragma once

#include <cstdint>
#include <cstddef>

namespace z80flags {

// Flag bit constants
constexpr uint8_t FLAG_S  = 0x80;
constexpr uint8_t FLAG_Z  = 0x40;
constexpr uint8_t FLAG_F5 = 0x20;
constexpr uint8_t FLAG_H  = 0x10;
constexpr uint8_t FLAG_F3 = 0x08;
constexpr uint8_t FLAG_PV = 0x04;
constexpr uint8_t FLAG_N  = 0x02;
constexpr uint8_t FLAG_C  = 0x01;

// Lookup tables (defined in flags.cpp)
extern uint8_t PARITY_TABLE[256];
extern uint8_t SZ_TABLE[256];
extern uint8_t SZ53_TABLE[256];
extern uint8_t SZP_TABLE[256];
extern uint8_t SZ53P_TABLE[256];
extern uint8_t SZHZP_TABLE[256];

// Rotate/shift tables: [op][256], op: 0=RLC,1=RRC,2=RL,3=RR,4=SLA,5=SRA,6=SLL,7=SRL
extern uint8_t ROT_RESULT[8][256];
extern uint8_t ROT_CARRY[8][256];
extern uint8_t RL_CARRY_0[256];
extern uint8_t RL_CARRY_1[256];
extern uint8_t RR_CARRY_0[256];
extern uint8_t RR_CARRY_1[256];

// 8-bit ALU flag tables (64KB each)
extern uint8_t ADD_FLAGS[65536];
extern uint8_t ADC_FLAGS[65536];
extern uint8_t SUB_FLAGS[65536];
extern uint8_t SBC_FLAGS[65536];
extern uint8_t INC_FLAGS[256];
extern uint8_t DEC_FLAGS[256];

// 16-bit flag helpers
uint8_t add16_flags(uint16_t hl, uint16_t reg, uint8_t current_f);
uint8_t adc16_flags(uint16_t hl, uint16_t reg, uint8_t carry);
uint8_t sbc16_flags(uint16_t hl, uint16_t reg, uint8_t carry);

// DAA table: index = (N << 10) | (H << 9) | (C << 8) | A
// Each entry: {corrected_A, full_flags} stored as 2 consecutive bytes
extern uint8_t DAA_FULL_FLAGS[4096];

// Condition table: [flags << 3 | cc]
extern bool COND_TABLE[2048];

// Precomputed bit masks
extern uint8_t BIT_MASK[8];
extern uint8_t RES_MASK[8];

// Initialize all tables (called once at startup)
void init_tables();

}
