#pragma once

#include <cstdint>

namespace z80 {

// ============================================================
// Flag Bit Definitions
// ============================================================
namespace Flags {
    constexpr uint8_t S  = 0x80;  // Sign
    constexpr uint8_t Z  = 0x40;  // Zero
    constexpr uint8_t F5 = 0x20;  // Undocumented copy bit 5
    constexpr uint8_t H  = 0x10;  // Half carry
    constexpr uint8_t F3 = 0x08;  // Undocumented copy bit 3
    constexpr uint8_t PV = 0x04;  // Parity / Overflow
    constexpr uint8_t N  = 0x02;  // Add/Subtract
    constexpr uint8_t C  = 0x01;  // Carry
}

// ============================================================
// Flag Tables — precomputed lookup tables (defined in flags.cpp)
// ============================================================
namespace FlagTables {
    // Parity: PARITY_TABLE[n] = Flags::PV if popcount(n) is even, else 0
    extern uint8_t PARITY_TABLE[256];

    // 8-bit ADD flags: ADD_FLAGS[(a<<8)|b] = flags for a+b
    extern uint8_t ADD_FLAGS[65536];

    // 8-bit SUB flags: SUB_FLAGS[(a<<8)|b] = flags for a-b
    extern uint8_t SUB_FLAGS[65536];

    // INC/DEC flags (carry preserved by caller)
    extern uint8_t INC_FLAGS[256];   // INC_FLAGS[old_val]
    extern uint8_t DEC_FLAGS[256];   // DEC_FLAGS[old_val]

    // Condition table: COND_TABLE[(flags<<3)|cc] = bool result
    // cc: 0=NZ 1=Z 2=NC 3=C 4=PO 5=PE 6=P 7=M
    extern bool COND_TABLE[2048];

    // Initialise all tables — called once at startup
    void init();
}

// ============================================================
// Inline flag helpers
// ============================================================

// 8-bit ADD: flags for a + b (result supplied for C flag boundary)
inline uint8_t calc_add_flags(uint8_t a, uint8_t b) {
    return FlagTables::ADD_FLAGS[(uint16_t(a) << 8) | b];
}

// 8-bit ADC: flags for a + b + carry
inline uint8_t calc_adc_flags(uint8_t a, uint8_t b, uint8_t carry, uint16_t res) {
    uint8_t r = res & 0xFF;
    uint8_t h = ((a & 0x0F) + (b & 0x0F) + carry) > 0x0F ? Flags::H : 0;
    uint8_t pv = (~(a ^ b) & (a ^ r) & 0x80) ? Flags::PV : 0;
    return (r & (Flags::S | Flags::F5 | Flags::F3)) |
           (r == 0 ? Flags::Z : 0) | h | pv |
           (res > 0xFF ? Flags::C : 0);
}

// 8-bit SUB: flags for a - b
inline uint8_t calc_sub_flags(uint8_t a, uint8_t b) {
    return FlagTables::SUB_FLAGS[(uint16_t(a) << 8) | b];
}

// 8-bit SBC: flags for a - b - carry
inline uint8_t calc_sbc_flags(uint8_t a, uint8_t b, uint8_t carry, uint16_t res) {
    uint8_t r = res & 0xFF;
    uint8_t h = ((a & 0x0F) < ((b & 0x0F) + carry)) ? Flags::H : 0;
    uint8_t pv = ((a ^ b) & (a ^ r) & 0x80) ? Flags::PV : 0;
    return Flags::N | (r & (Flags::S | Flags::F5 | Flags::F3)) |
           (r == 0 ? Flags::Z : 0) | h | pv |
           (res > 0xFF ? Flags::C : 0);
}

// INC r — caller preserves carry by masking: F = (F & C) | calc_inc_flags(val)
inline uint8_t calc_inc_flags(uint8_t val) {
    return FlagTables::INC_FLAGS[val];
}

// DEC r — caller preserves carry by masking: F = (F & C) | calc_dec_flags(val)
inline uint8_t calc_dec_flags(uint8_t val) {
    return FlagTables::DEC_FLAGS[val];
}

// ============================================================
// Condition check
// ============================================================
inline bool check_condition(uint8_t flags, int cc) {
    return FlagTables::COND_TABLE[(uint16_t(flags) << 3) | (cc & 0x07)];
}

} // namespace z80
