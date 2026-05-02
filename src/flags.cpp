#include "../include/z80/flags.h"

namespace z80 {
namespace FlagTables {

// Table definitions
uint8_t PARITY_TABLE[256];
uint8_t ADD_FLAGS[65536];
uint8_t SUB_FLAGS[65536];
uint8_t INC_FLAGS[256];
uint8_t DEC_FLAGS[256];
bool    COND_TABLE[2048];

void init() {
    // --------------------------------------------------------
    // PARITY_TABLE[n] = Flags::PV if popcount(n) even, else 0
    // --------------------------------------------------------
    for (int n = 0; n < 256; ++n) {
        int bits = 0;
        for (int b = 0; b < 8; ++b) bits += (n >> b) & 1;
        PARITY_TABLE[n] = (bits & 1) ? 0 : Flags::PV;
    }

    // --------------------------------------------------------
    // ADD_FLAGS[(a<<8)|b] = flags for (a + b)
    // SUB_FLAGS[(a<<8)|b] = flags for (a - b)
    // --------------------------------------------------------
    for (int a = 0; a < 256; ++a) {
        for (int b = 0; b < 256; ++b) {
            int idx = (a << 8) | b;

            // ADD a + b
            {
                int res = a + b;
                uint8_t r = res & 0xFF;
                uint8_t f = 0;
                if (r == 0)                              f |= Flags::Z;
                if (r & 0x80)                            f |= Flags::S;
                if (res > 0xFF)                          f |= Flags::C;
                if ((a ^ b ^ res) & 0x10)               f |= Flags::H;
                if (~(a ^ b) & (a ^ res) & 0x80)        f |= Flags::PV;
                f |= r & (Flags::F5 | Flags::F3);
                ADD_FLAGS[idx] = f;
            }

            // SUB a - b
            {
                int res = a - b;
                uint8_t r = res & 0xFF;
                uint8_t f = Flags::N;
                if (r == 0)                              f |= Flags::Z;
                if (r & 0x80)                            f |= Flags::S;
                if (res < 0)                             f |= Flags::C;
                if ((a ^ b ^ res) & 0x10)               f |= Flags::H;
                if ((a ^ b) & (a ^ res) & 0x80)         f |= Flags::PV;
                f |= r & (Flags::F5 | Flags::F3);
                SUB_FLAGS[idx] = f;
            }
        }
    }

    // --------------------------------------------------------
    // INC_FLAGS[old] = flags produced by INC (carry not included)
    // DEC_FLAGS[old] = flags produced by DEC (carry not included)
    // --------------------------------------------------------
    for (int v = 0; v < 256; ++v) {
        // INC
        {
            uint8_t res = (v + 1) & 0xFF;
            uint8_t f = 0;
            if (res == 0)            f |= Flags::Z;
            if (res & 0x80)          f |= Flags::S;
            if ((v & 0x0F) == 0x0F)  f |= Flags::H;   // low nibble overflow
            if (v == 0x7F)           f |= Flags::PV;  // signed overflow
            f |= res & (Flags::F5 | Flags::F3);
            INC_FLAGS[v] = f;
        }
        // DEC
        {
            uint8_t res = (v - 1) & 0xFF;
            uint8_t f = Flags::N;
            if (res == 0)            f |= Flags::Z;
            if (res & 0x80)          f |= Flags::S;
            if ((v & 0x0F) == 0x00)  f |= Flags::H;   // low nibble borrow
            if (v == 0x80)           f |= Flags::PV;  // signed overflow
            f |= res & (Flags::F5 | Flags::F3);
            DEC_FLAGS[v] = f;
        }
    }

    // --------------------------------------------------------
    // COND_TABLE[(flags<<3)|cc]
    // cc: 0=NZ 1=Z 2=NC 3=C 4=PO 5=PE 6=P(plus) 7=M(minus)
    // --------------------------------------------------------
    for (int fl = 0; fl < 256; ++fl) {
        COND_TABLE[(fl << 3) | 0] = !(fl & Flags::Z);
        COND_TABLE[(fl << 3) | 1] =  (fl & Flags::Z) != 0;
        COND_TABLE[(fl << 3) | 2] = !(fl & Flags::C);
        COND_TABLE[(fl << 3) | 3] =  (fl & Flags::C) != 0;
        COND_TABLE[(fl << 3) | 4] = !(fl & Flags::PV);
        COND_TABLE[(fl << 3) | 5] =  (fl & Flags::PV) != 0;
        COND_TABLE[(fl << 3) | 6] = !(fl & Flags::S);
        COND_TABLE[(fl << 3) | 7] =  (fl & Flags::S) != 0;
    }
}

} // namespace FlagTables
} // namespace z80
