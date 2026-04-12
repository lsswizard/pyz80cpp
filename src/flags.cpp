#include "../include/z80/flags.h"
#include <array>

namespace z80 {
namespace FlagTables {

// Definition of external tables
uint8_t PARITY_TABLE[256];
uint8_t SZ_TABLE[256];
uint8_t SZ53_TABLE[256];
uint8_t SZP_TABLE[256];
uint8_t SZ53P_TABLE[256];
uint8_t SZHZP_TABLE[256];

uint8_t ROT_RESULT[8][256];
uint8_t ROT_CARRY[8][256];
uint8_t RL_CARRY_0[256];
uint8_t RL_CARRY_1[256];
uint8_t RR_CARRY_0[256];
uint8_t RR_CARRY_1[256];

uint8_t ADD_FLAGS[65536];
uint8_t ADC_FLAGS[65536];
uint8_t SUB_FLAGS[65536];
uint8_t SBC_FLAGS[65536];
uint8_t INC_FLAGS[256];
uint8_t DEC_FLAGS[256];

uint8_t DAA_FULL_FLAGS[4096];
bool COND_TABLE[2048];

uint8_t BIT_MASK[8] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};
uint8_t RES_MASK[8] = {
    0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F
};

void init() {
    // Initialize PARITY_TABLE, SZ_TABLE, SZ53_TABLE, SZP_TABLE, SZ53P_TABLE, SZHZP_TABLE
    for (int i = 0; i < 256; ++i) {
        uint8_t flags = 0;
        if (i == 0) flags |= Flags::Z;
        if (i & 0x80) flags |= Flags::S; // Sign flag
        
        // Calculate parity
        int set_bits = 0;
        for (int j = 0; j < 8; ++j) {
            if ((i >> j) & 1) {
                set_bits++;
            }
        }
        if ((set_bits % 2) == 0) flags |= Flags::PV; // Parity flag
        
        if (i & Flags::F5) flags |= Flags::F5;
        if (i & Flags::F3) flags |= Flags::F3;

        PARITY_TABLE[i] = ((set_bits % 2) == 0) ? Flags::PV : 0;
        SZ_TABLE[i] = flags & (Flags::S | Flags::Z);
        SZ53_TABLE[i] = flags & (Flags::S | Flags::Z | Flags::F5 | Flags::F3);
        SZP_TABLE[i] = flags & (Flags::S | Flags::Z | Flags::PV);
        SZ53P_TABLE[i] = flags & (Flags::S | Flags::Z | Flags::F5 | Flags::F3 | Flags::PV);
        SZHZP_TABLE[i] = flags & (Flags::S | Flags::Z | Flags::H | Flags::PV); // H flag needs to be set separately for arithmetic operations
    }

    // Initialize ADD_FLAGS, ADC_FLAGS, SUB_FLAGS, SBC_FLAGS
    for (uint16_t x = 0; x < 256; ++x) {
        for (uint16_t y = 0; y < 256; ++y) {
            uint16_t index = (x << 8) | y;

            // ADD_FLAGS
            {
                uint16_t result = x + y;
                uint8_t flags = 0;
                if ((result & 0xFF) == 0) flags |= Flags::Z;
                if (result & 0x80) flags |= Flags::S;
                if (((x ^ y ^ result) & 0x10) != 0) flags |= Flags::H; // Half Carry
                if (result > 0xFF) flags |= Flags::C;
                if (((x ^ result) & (y ^ result) & 0x80) != 0) flags |= Flags::PV; // Overflow
                
                flags |= (result & Flags::F5); // Undocumented F5
                flags |= (result & Flags::F3); // Undocumented F3
                ADD_FLAGS[index] = flags;
            }

            // ADC_FLAGS
            for (int c_in = 0; c_in < 2; ++c_in) {
                uint16_t adc_index = (x << 8) | y | (c_in << 8);
                uint16_t result = x + y + c_in;
                uint8_t flags = 0;
                if ((result & 0xFF) == 0) flags |= Flags::Z;
                if (result & 0x80) flags |= Flags::S;
                if (((x ^ y ^ result) & 0x10) != 0) flags |= Flags::H; // Half Carry
                if (result > 0xFF) flags |= Flags::C;
                if (((x ^ result) & (y ^ result) & 0x80) != 0) flags |= Flags::PV; // Overflow
                
                flags |= (result & Flags::F5); // Undocumented F5
                flags |= (result & Flags::F3); // Undocumented F3
                ADC_FLAGS[adc_index] = flags;
            }

            // SUB_FLAGS
            {
                uint16_t result = x - y;
                uint8_t flags = Flags::N; // N flag is set for subtraction
                if ((result & 0xFF) == 0) flags |= Flags::Z;
                if (result & 0x80) flags |= Flags::S;
                if (((x ^ y ^ result) & 0x10) != 0) flags |= Flags::H; // Half Carry (borrow)
                if (y > x) flags |= Flags::C; // Carry (borrow)
                if (((x ^ result) & (~y ^ result) & 0x80) != 0) flags |= Flags::PV; // Overflow
                
                flags |= (result & Flags::F5); // Undocumented F5
                flags |= (result & Flags::F3); // Undocumented F3
                SUB_FLAGS[index] = flags;
            }

            // SBC_FLAGS
            for (int c_in = 0; c_in < 2; ++c_in) {
                uint16_t sbc_index = (x << 8) | y | (c_in << 8);
                uint16_t result = x - y - c_in;
                uint8_t flags = Flags::N; // N flag is set for subtraction
                if ((result & 0xFF) == 0) flags |= Flags::Z;
                if (result & 0x80) flags |= Flags::S;
                if (((x ^ y ^ result) & 0x10) != 0) flags |= Flags::H; // Half Carry (borrow)
                if (y + c_in > x) flags |= Flags::C; // Carry (borrow)
                if (((x ^ result) & (~y ^ result) & 0x80) != 0) flags |= Flags::PV; // Overflow
                
                flags |= (result & Flags::F5); // Undocumented F5
                flags |= (result & Flags::F3); // Undocumented F3
                SBC_FLAGS[sbc_index] = flags;
            }
        }
    }

    // Initialize INC_FLAGS
    for (int i = 0; i < 256; ++i) {
        uint8_t result = i + 1;
        uint8_t flags = 0;
        if (result == 0) flags |= Flags::Z;
        if (result & 0x80) flags |= Flags::S;
        if ((i & 0x0F) == 0x0F) flags |= Flags::H; // Half carry (if low nibble overflows)
        if (i == 0x7F) flags |= Flags::PV; // Overflow
        
        flags |= (result & Flags::F5);
        flags |= (result & Flags::F3);
        INC_FLAGS[i] = flags;
    }

    // Initialize DEC_FLAGS
    for (int i = 0; i < 256; ++i) {
        uint8_t result = i - 1;
        uint8_t flags = Flags::N; // N flag for subtraction
        if (result == 0) flags |= Flags::Z;
        if (result & 0x80) flags |= Flags::S;
        if ((i & 0x0F) == 0x00) flags |= Flags::H; // Half carry (if low nibble underflows)
        if (i == 0x80) flags |= Flags::PV; // Overflow
        
        flags |= (result & Flags::F5);
        flags |= (result & Flags::F3);
        DEC_FLAGS[i] = flags;
    }

    // Initialize DAA_FULL_FLAGS
    // DAA: index = (N << 10) | (H << 9) | (C << 8) | A
    for (int N = 0; N < 2; ++N) {
        for (int H = 0; H < 2; ++H) {
            for (int C = 0; C < 2; ++C) {
                for (int A = 0; A < 256; ++A) {
                    uint16_t index = (N << 10) | (H << 9) | (C << 8) | A;
                    uint8_t result_A = A;
                    uint8_t flags = 0;
                    
                    uint8_t correction = 0;
                    if (H || ((A & 0x0F) > 9)) {
                        correction |= 0x06;
                    }
                    if (C || (A > 0x99)) {
                        correction |= 0x60;
                    }

                    if (N) { // Subtraction
                        result_A -= correction;
                        flags |= Flags::N;
                        if (H || ((A & 0x0F) > 9)) {
                             // This is complicated due to half carry
                        }
                    } else { // Addition
                        result_A += correction;
                        if (H || ((A & 0x0F) > 9)) {
                            // Half carry flag logic needs careful handling
                        }
                    }

                    if (C || (A > 0x99)) {
                        flags |= Flags::C;
                    }

                    if (result_A == 0) flags |= Flags::Z;
                    if (result_A & 0x80) flags |= Flags::S;
                    
                    // PV flag calculation for DAA is complex
                    int set_bits = 0;
                    for (int j = 0; j < 8; ++j) {
                        if ((result_A >> j) & 1) {
                            set_bits++;
                        }
                    }
                    if ((set_bits % 2) == 0) flags |= Flags::PV;

                    flags |= (result_A & Flags::F5);
                    flags |= (result_A & Flags::F3);

                    DAA_FULL_FLAGS[index] = flags;
                }
            }
        }
    }

    // Initialize ROT_RESULT and ROT_CARRY, RL_CARRY_0, etc.
    for (int val = 0; val < 256; ++val) {
        // RLC (0)
        ROT_RESULT[0][val] = (val << 1) | (val >> 7);
        ROT_CARRY[0][val] = (val >> 7) & Flags::C;
        
        // RRC (1)
        ROT_RESULT[1][val] = (val >> 1) | (val << 7);
        ROT_CARRY[1][val] = val & Flags::C;
        
        // RL (2) - with carry 0
        RL_CARRY_0[val] = (val << 1);
        ROT_RESULT[2][val] = (val << 1); // No carry
        ROT_CARRY[2][val] = (val >> 7) & Flags::C;

        // RL (2) - with carry 1
        RL_CARRY_1[val] = (val << 1) | Flags::C;
        
        // RR (3) - with carry 0
        RR_CARRY_0[val] = (val >> 1);
        ROT_RESULT[3][val] = (val >> 1); // No carry
        ROT_CARRY[3][val] = val & Flags::C;

        // RR (3) - with carry 1
        RR_CARRY_1[val] = (val >> 1) | 0x80; // If carry is 1, MSB becomes 1


        // SLA (4)
        ROT_RESULT[4][val] = val << 1;
        ROT_CARRY[4][val] = (val >> 7) & Flags::C;

        // SRA (5)
        ROT_RESULT[5][val] = (val >> 1) | (val & 0x80);
        ROT_CARRY[5][val] = val & Flags::C;

        // SLL (6) - undocumented
        ROT_RESULT[6][val] = (val << 1) | 0x01;
        ROT_CARRY[6][val] = (val >> 7) & Flags::C;

        // SRL (7)
        ROT_RESULT[7][val] = val >> 1;
        ROT_CARRY[7][val] = val & Flags::C;
    }

    // Initialize COND_TABLE
    // cc: 0=NZ, 1=Z, 2=NC, 3=C, 4=PO, 5=PE, 6=P, 7=M
    for (int flags = 0; flags < 256; ++flags) {
        for (int cc = 0; cc < 8; ++cc) {
            uint16_t index = (flags << 3) | cc;
            bool condition = false;
            switch (cc) {
                case 0: condition = !(flags & Flags::Z); break; // NZ
                case 1: condition = (flags & Flags::Z); break;  // Z
                case 2: condition = !(flags & Flags::C); break; // NC
                case 3: condition = (flags & Flags::C); break;  // C
                case 4: condition = !(flags & Flags::PV); break; // PO (Parity Odd)
                case 5: condition = (flags & Flags::PV); break;  // PE (Parity Even)
                case 6: condition = !(flags & Flags::S); break; // P (Positive)
                case 7: condition = (flags & Flags::S); break;  // M (Minus)
            }
            COND_TABLE[index] = condition;
        }
    }
}

} // namespace FlagTables
} // namespace z80