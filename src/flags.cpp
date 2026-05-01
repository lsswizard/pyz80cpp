#include "../include/z80/flags.h"
#include <array>

namespace z80 {
namespace FlagTables {

// Definition of external tables (only what's actually used)
uint8_t PARITY_TABLE[256];
uint8_t ADD_FLAGS[65536];
uint8_t SUB_FLAGS[65536];
uint8_t INC_FLAGS[256];
uint8_t DEC_FLAGS[256];

bool COND_TABLE[2048];

void init() {
    // Initialize PARITY_TABLE
    for (int i = 0; i < 256; ++i) {
        int set_bits = 0;
        for (int j = 0; j < 8; ++j) {
            if ((i >> j) & 1) {
                set_bits++;
            }
        }
        PARITY_TABLE[i] = ((set_bits % 2) == 0) ? Flags::PV : 0;
    }

    // Initialize ADD_FLAGS
    for (uint16_t x = 0; x < 256; ++x) {
        for (uint16_t y = 0; y < 256; ++y) {
            uint16_t result = x + y;
            uint8_t flags = 0;
            if ((result & 0xFF) == 0) flags |= Flags::Z;
            if (result & 0x80) flags |= Flags::S;
            if (((x ^ y ^ result) & 0x10) != 0) flags |= Flags::H;
            if (result > 0xFF) flags |= Flags::C;
            if (((x ^ result) & (y ^ result) & 0x80) != 0) flags |= Flags::PV;

            flags |= (result & Flags::F5);
            flags |= (result & Flags::F3);
            ADD_FLAGS[(x << 8) | y] = flags;
        }
    }

    // Initialize SUB_FLAGS
    for (uint16_t x = 0; x < 256; ++x) {
        for (uint16_t y = 0; y < 256; ++y) {
            uint16_t result = x - y;
            uint8_t flags = Flags::N;
            if ((result & 0xFF) == 0) flags |= Flags::Z;
            if (result & 0x80) flags |= Flags::S;
            if (((x ^ y ^ result) & 0x10) != 0) flags |= Flags::H;
            if (y > x) flags |= Flags::C;
            if (((x ^ result) & (~y ^ result) & 0x80) != 0) flags |= Flags::PV;

            flags |= (result & Flags::F5);
            flags |= (result & Flags::F3);
            SUB_FLAGS[(x << 8) | y] = flags;
        }
    }

    // Initialize INC_FLAGS
    for (int i = 0; i < 256; ++i) {
        uint8_t result = i + 1;
        uint8_t flags = 0;
        if (result == 0) flags |= Flags::Z;
        if (result & 0x80) flags |= Flags::S;
        if ((i & 0x0F) == 0x0F) flags |= Flags::H;
        if (i == 0x7F) flags |= Flags::PV;

        flags |= (result & Flags::F5);
        flags |= (result & Flags::F3);
        INC_FLAGS[i] = flags;
    }

    // Initialize DEC_FLAGS
    for (int i = 0; i < 256; ++i) {
        uint8_t result = i - 1;
        uint8_t flags = Flags::N;
        if (result == 0) flags |= Flags::Z;
        if (result & 0x80) flags |= Flags::S;
        if ((i & 0x0F) == 0x00) flags |= Flags::H;
        if (i == 0x80) flags |= Flags::PV;

        flags |= (result & Flags::F5);
        flags |= (result & Flags::F3);
        DEC_FLAGS[i] = flags;
    }

    // Initialize COND_TABLE
    for (int flags = 0; flags < 256; ++flags) {
        for (int cc = 0; cc < 8; ++cc) {
            uint16_t index = (flags << 3) | cc;
            bool condition = false;
            switch (cc) {
                case 0: condition = !(flags & Flags::Z); break;
                case 1: condition = (flags & Flags::Z); break;
                case 2: condition = !(flags & Flags::C); break;
                case 3: condition = (flags & Flags::C); break;
                case 4: condition = !(flags & Flags::PV); break;
                case 5: condition = (flags & Flags::PV); break;
                case 6: condition = !(flags & Flags::S); break;
                case 7: condition = (flags & Flags::S); break;
            }
            COND_TABLE[index] = condition;
        }
    }
}

} // namespace FlagTables
} // namespace z80