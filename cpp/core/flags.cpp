#include "flags.h"
#include <cstring>

namespace z80flags {

constexpr uint8_t _F53 = FLAG_F3 | FLAG_F5;

// Table storage
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
bool    COND_TABLE[2048];

uint8_t BIT_MASK[8];
uint8_t RES_MASK[8];

static void build_parity() {
    for (int i = 0; i < 256; i++) {
        int b = 0, n = i;
        while (n) { b++; n &= n - 1; }
        PARITY_TABLE[i] = (b % 2) ? 0 : 1;
    }
}

static void build_sz() {
    for (int i = 0; i < 256; i++) {
        uint8_t v = 0;
        if (i & 0x80) v |= FLAG_S;
        if (i == 0)   v |= FLAG_Z;
        SZ_TABLE[i] = v;
    }
}

static void build_sz53() {
    for (int i = 0; i < 256; i++) {
        uint8_t v = 0;
        if (i & 0x80) v |= FLAG_S;
        if (i == 0)   v |= FLAG_Z;
        v |= (i & _F53);
        SZ53_TABLE[i] = v;
    }
}

static void build_szp() {
    for (int i = 0; i < 256; i++) {
        uint8_t v = 0;
        if (i & 0x80) v |= FLAG_S;
        if (i == 0)   v |= FLAG_Z;
        if (PARITY_TABLE[i]) v |= FLAG_PV;
        SZP_TABLE[i] = v;
    }
}

static void build_sz53p() {
    for (int i = 0; i < 256; i++) {
        uint8_t v = 0;
        if (i & 0x80) v |= FLAG_S;
        if (i == 0)   v |= FLAG_Z;
        if (PARITY_TABLE[i]) v |= FLAG_PV;
        v |= (i & _F53);
        SZ53P_TABLE[i] = v;
    }
}

static void build_szhzp() {
    for (int i = 0; i < 256; i++) {
        uint8_t v = FLAG_H;
        if (i & 0x80) v |= FLAG_S;
        if (i == 0)   v |= FLAG_Z;
        if (PARITY_TABLE[i]) v |= FLAG_PV;
        v |= (i & _F53);
        SZHZP_TABLE[i] = v;
    }
}

static void build_rot() {
    for (int v = 0; v < 256; v++) {
        ROT_RESULT[0][v] = ((v << 1) | (v >> 7)) & 0xFF;
        ROT_CARRY[0][v]  = (v >> 7) & 1;
        ROT_RESULT[1][v] = ((v >> 1) | (v << 7)) & 0xFF;
        ROT_CARRY[1][v]  = v & 1;
        ROT_RESULT[4][v] = (v << 1) & 0xFF;
        ROT_CARRY[4][v]  = (v >> 7) & 1;
        ROT_RESULT[5][v] = (v >> 1) | (v & 0x80);
        ROT_CARRY[5][v]  = v & 1;
        ROT_RESULT[6][v] = ((v << 1) | 1) & 0xFF;
        ROT_CARRY[6][v]  = (v >> 7) & 1;
        ROT_RESULT[7][v] = v >> 1;
        ROT_CARRY[7][v]  = v & 1;
        RL_CARRY_0[v] = (v << 1) & 0xFF;
        RL_CARRY_1[v] = ((v << 1) | 1) & 0xFF;
        RR_CARRY_0[v] = v >> 1;
        RR_CARRY_1[v] = (v >> 1) | 0x80;
    }
}

static void build_alu() {
    // INC
    for (int a = 0; a < 256; a++) {
        int nv = (a + 1) & 0xFF;
        uint8_t f = nv & (FLAG_S | _F53);
        if (nv == 0) f |= FLAG_Z;
        if ((a & 0x0F) == 0x0F) f |= FLAG_H;
        if (a == 0x7F) f |= FLAG_PV;
        INC_FLAGS[a] = f;
    }
    // DEC
    for (int a = 0; a < 256; a++) {
        int nv = (a - 1) & 0xFF;
        uint8_t f = FLAG_N | (nv & (FLAG_S | _F53));
        if (nv == 0) f |= FLAG_Z;
        if ((a & 0x0F) == 0x00) f |= FLAG_H;
        if (a == 0x80) f |= FLAG_PV;
        DEC_FLAGS[a] = f;
    }
    // ADD/ADC/SUB/SBC
    for (int a = 0; a < 256; a++) {
        for (int b = 0; b < 256; b++) {
            int idx = (a << 8) | b;
            // ADD
            {
                int r = a + b;
                int r8 = r & 0xFF;
                uint8_t f = r8 & (FLAG_S | _F53);
                if (r8 == 0) f |= FLAG_Z;
                if (((a & 0x0F) + (b & 0x0F)) & 0x10) f |= FLAG_H;
                if (r > 0xFF) f |= FLAG_C;
                if (((a ^ b) & 0x80) == 0 && ((r8 ^ a) & 0x80) != 0) f |= FLAG_PV;
                ADD_FLAGS[idx] = f;
            }
            // ADC
            {
                int r = a + b + 1;
                int r8 = r & 0xFF;
                uint8_t f = r8 & (FLAG_S | _F53);
                if (r8 == 0) f |= FLAG_Z;
                if (((a & 0x0F) + (b & 0x0F) + 1) & 0x10) f |= FLAG_H;
                if (r > 0xFF) f |= FLAG_C;
                if (((a ^ b) & 0x80) == 0 && ((r8 ^ a) & 0x80) != 0) f |= FLAG_PV;
                ADC_FLAGS[idx] = f;
            }
            // SUB
            {
                int r = a - b;
                int r8 = r & 0xFF;
                uint8_t f = FLAG_N | (r8 & (FLAG_S | _F53));
                if (r8 == 0) f |= FLAG_Z;
                if ((a & 0x0F) < (b & 0x0F)) f |= FLAG_H;
                if (r < 0) f |= FLAG_C;
                if (((a ^ b) & 0x80) != 0 && ((r8 ^ a) & 0x80) != 0) f |= FLAG_PV;
                SUB_FLAGS[idx] = f;
            }
            // SBC
            {
                int r = a - b - 1;
                int r8 = r & 0xFF;
                uint8_t f = FLAG_N | (r8 & (FLAG_S | _F53));
                if (r8 == 0) f |= FLAG_Z;
                if ((a & 0x0F) < ((b & 0x0F) + 1)) f |= FLAG_H;
                if (r < 0) f |= FLAG_C;
                if (((a ^ b) & 0x80) != 0 && ((r8 ^ a) & 0x80) != 0) f |= FLAG_PV;
                SBC_FLAGS[idx] = f;
            }
        }
    }
}

static void build_daa() {
    for (int n = 0; n < 2; n++) {
        for (int h = 0; h < 2; h++) {
            for (int c = 0; c < 2; c++) {
                int base_idx = (n << 10) | (h << 9) | (c << 8);
                int input_f = (n << 1) | c;
                for (int orig_a = 0; orig_a < 256; orig_a++) {
                    int a = orig_a;
                    int new_c = c;
                    int new_h = 0;
                    if (!n) {
                        if (h || (a & 0x0F) > 9) {
                            a = a + 0x06;
                            new_h = 1;
                        }
                        if (c || a > 0x9F) {
                            a = (a + 0x60) & 0xFF;
                            new_c = 1;
                        } else {
                            a = a & 0xFF;
                        }
                    } else {
                        if (h) {
                            a = (a - 0x06) & 0xFF;
                            new_h = 1;
                        }
                        if (c) {
                            a = (a - 0x60) & 0xFF;
                            new_c = 1;
                        }
                    }
                    int idx = (base_idx | orig_a) * 2;
                    int flags = input_f & FLAG_N;
                    if (new_c) flags |= FLAG_C;
                    if (new_h) flags |= FLAG_H;
                    if (a == 0) flags |= FLAG_Z;
                    if (a & 0x80) flags |= FLAG_S;
                    flags |= a & _F53;
                    if (PARITY_TABLE[a]) flags |= FLAG_PV;
                    DAA_FULL_FLAGS[idx]     = (uint8_t)a;
                    DAA_FULL_FLAGS[idx + 1] = (uint8_t)flags;
                }
            }
        }
    }
}

static void build_cond() {
    for (int f = 0; f < 256; f++) {
        int base = f << 3;
        COND_TABLE[base + 0] = !(f & FLAG_Z);
        COND_TABLE[base + 1] =  (f & FLAG_Z);
        COND_TABLE[base + 2] = !(f & FLAG_C);
        COND_TABLE[base + 3] =  (f & FLAG_C);
        COND_TABLE[base + 4] = !(f & FLAG_PV);
        COND_TABLE[base + 5] =  (f & FLAG_PV);
        COND_TABLE[base + 6] = !(f & FLAG_S);
        COND_TABLE[base + 7] =  (f & FLAG_S);
    }
}

static void build_bit_masks() {
    for (int i = 0; i < 8; i++) {
        BIT_MASK[i] = (uint8_t)(1 << i);
        RES_MASK[i] = (uint8_t)(~(1 << i) & 0xFF);
    }
}

void init_tables() {
    build_parity();
    build_sz();
    build_sz53();
    build_szp();
    build_sz53p();
    build_szhzp();
    build_rot();
    build_alu();
    build_daa();
    build_cond();
    build_bit_masks();
}

uint8_t add16_flags(uint16_t hl, uint16_t reg, uint8_t current_f) {
    uint32_t full = hl + reg;
    uint16_t r16 = full & 0xFFFF;
    uint8_t f = current_f & (FLAG_S | FLAG_Z | FLAG_PV);
    if (((hl & 0x0FFF) + (reg & 0x0FFF)) > 0x0FFF) f |= FLAG_H;
    if (full > 0xFFFF) f |= FLAG_C;
    f |= (r16 >> 8) & _F53;
    return f;
}

uint8_t adc16_flags(uint16_t hl, uint16_t reg, uint8_t carry) {
    uint32_t result = hl + reg + carry;
    uint16_t r16 = result & 0xFFFF;
    uint8_t f = 0;
    if (r16 == 0) f |= FLAG_Z;
    if (r16 & 0x8000) f |= FLAG_S;
    if (((hl & 0x0FFF) + (reg & 0x0FFF) + carry) > 0x0FFF) f |= FLAG_H;
    if (result > 0xFFFF) f |= FLAG_C;
    if (((hl ^ reg) & 0x8000) == 0 && ((r16 ^ hl) & 0x8000) != 0) f |= FLAG_PV;
    f |= (r16 >> 8) & _F53;
    return f;
}

uint8_t sbc16_flags(uint16_t hl, uint16_t reg, uint8_t carry) {
    int32_t result = hl - reg - carry;
    uint16_t r16 = result & 0xFFFF;
    uint8_t f = FLAG_N;
    if (r16 == 0) f |= FLAG_Z;
    if (r16 & 0x8000) f |= FLAG_S;
    if ((hl & 0x0FFF) < (reg & 0x0FFF) + carry) f |= FLAG_H;
    if (hl < reg + carry) f |= FLAG_C;
    if ((hl ^ reg) & (hl ^ r16) & 0x8000) f |= FLAG_PV;
    f |= (r16 >> 8) & _F53;
    return f;
}

}
