#include "decoder.h"
#include "cpu.h"
#include "flags.h"
#include <cstring>
#include <cstdio>
#include <initializer_list>

// Handler table definitions
OpcodeEntry base_handlers[256];
OpcodeEntry cb_handlers[256];
OpcodeEntry ed_handlers[256];
OpcodeEntry dd_handlers[256];
OpcodeEntry fd_handlers[256];
OpcodeEntry ddcb_handlers[256];
OpcodeEntry fdcb_handlers[256];
OpcodeEntry dd_ed_handlers[256];  // DD ED xx handlers
OpcodeEntry fd_ed_handlers[256];  // FD ED xx handlers

// ============================================================
// Decoder constructor - builds cache
// ============================================================
Decoder::Decoder() {
    std::memset(_cache, 0, sizeof(_cache));
}

const DecodeSlot& Decoder::decode(uint8_t* mem, uint16_t addr) {
    addr &= 0xFFFF;
    DecodeSlot& slot = _cache[addr];
    if (slot.handler != nullptr) return slot;

    // When mem is nullptr (e.g., CallbackBus), we cannot cache or decode
    // In this case, return empty slot and let step() handle it inline
    if (mem == nullptr) {
        return slot;  // handler == nullptr, will be handled in step()
    }

    uint8_t opcode = mem[addr];
    uint8_t len, cyc;
    OpHandler h;
    bool af = false, ir = false;

    if (opcode == 0xCB) {
        uint8_t cb_op = mem[(addr + 1) & 0xFFFF];
        slot = DecodeSlot{cb_handlers[cb_op].handler, cb_handlers[cb_op].cycles, 2, true, false};
    } else if (opcode == 0xED) {
        uint8_t ed_op = mem[(addr + 1) & 0xFFFF];
        slot = DecodeSlot{ed_handlers[ed_op].handler, ed_handlers[ed_op].cycles, 2, ed_handlers[ed_op].affects_f, false};
    } else if (opcode == 0xDD) {
        uint8_t dd_op = mem[(addr + 1) & 0xFFFF];
        if (dd_op == 0xCB) {
            uint8_t cb_op = mem[(addr + 3) & 0xFFFF];
            slot = DecodeSlot{ddcb_handlers[cb_op].handler, ddcb_handlers[cb_op].cycles, 4, ddcb_handlers[cb_op].affects_f, false};
        } else if (dd_op == 0xED) {
            // DD ED xx: Extended operations on IX (e.g., ADC IX,BC)
            uint8_t ed_op = mem[(addr + 2) & 0xFFFF];
            slot = DecodeSlot{dd_ed_handlers[ed_op].handler, dd_ed_handlers[ed_op].cycles, dd_ed_handlers[ed_op].length, dd_ed_handlers[ed_op].affects_f, false};
        } else if (dd_handlers[dd_op].handler) {
            slot = DecodeSlot{dd_handlers[dd_op].handler, dd_handlers[dd_op].cycles, dd_handlers[dd_op].length, dd_handlers[dd_op].affects_f, false};
        } else {
            // DD prefix fallthrough: return null so step() handles it inline
            // with correct PC adjustment
            slot = DecodeSlot{nullptr, 4, 1, false, false};
        }
    } else if (opcode == 0xFD) {
        uint8_t fd_op = mem[(addr + 1) & 0xFFFF];
        if (fd_op == 0xCB) {
            uint8_t cb_op = mem[(addr + 3) & 0xFFFF];
            slot = DecodeSlot{fdcb_handlers[cb_op].handler, fdcb_handlers[cb_op].cycles, 4, fdcb_handlers[cb_op].affects_f, false};
        } else if (fd_op == 0xED) {
            // FD ED xx: Extended operations on IY (e.g., ADC IY,BC)
            uint8_t ed_op = mem[(addr + 2) & 0xFFFF];
            slot = DecodeSlot{fd_ed_handlers[ed_op].handler, fd_ed_handlers[ed_op].cycles, fd_ed_handlers[ed_op].length, fd_ed_handlers[ed_op].affects_f, false};
        } else if (fd_handlers[fd_op].handler) {
            slot = DecodeSlot{fd_handlers[fd_op].handler, fd_handlers[fd_op].cycles, fd_handlers[fd_op].length, fd_handlers[fd_op].affects_f, false};
        } else {
            // FD prefix fallthrough: return null so step() handles it inline
            // with correct PC adjustment
            slot = DecodeSlot{nullptr, 4, 1, false, false};
        }
    } else {
        slot = DecodeSlot{base_handlers[opcode].handler, base_handlers[opcode].cycles, base_handlers[opcode].length, base_handlers[opcode].affects_f, base_handlers[opcode].is_ld_a_ir};
    }

    return slot;
}

void Decoder::invalidate(uint16_t addr) {
    addr &= 0xFFFF;
    for (int i = 0; i < 4; i++) {
        _cache[(addr - i) & 0xFFFF] = DecodeSlot{nullptr, 4, 1, false, false};
    }
}

void Decoder::invalidate_all() {
    std::memset(_cache, 0, sizeof(_cache));
}

// ============================================================
// Decode from raw bytes (for buses without direct memory access)
// opcode: the first byte at PC
// b1, b2, b3: bytes at PC+1, PC+2, PC+3 (for prefix handling)
// Returns a DecodeSlot (no caching - recalculated each time)
// ============================================================
DecodeSlot Decoder::decode_from_bytes(uint8_t opcode, uint8_t b1, uint8_t b2, uint8_t b3) {
    if (opcode == 0xCB) {
        return DecodeSlot{cb_handlers[b1].handler, cb_handlers[b1].cycles, 2, true, false};
    } else if (opcode == 0xED) {
        return DecodeSlot{ed_handlers[b1].handler, ed_handlers[b1].cycles, 2, ed_handlers[b1].affects_f, false};
    } else if (opcode == 0xDD) {
        if (b1 == 0xCB) {
            return DecodeSlot{ddcb_handlers[b3].handler, ddcb_handlers[b3].cycles, 4, ddcb_handlers[b3].affects_f, false};
        } else if (b1 == 0xED) {
            return DecodeSlot{dd_ed_handlers[b2].handler, dd_ed_handlers[b2].cycles, dd_ed_handlers[b2].length, dd_ed_handlers[b2].affects_f, false};
        } else {
            return DecodeSlot{dd_handlers[b1].handler, dd_handlers[b1].cycles, dd_handlers[b1].length, dd_handlers[b1].affects_f, false};
        }
    } else if (opcode == 0xFD) {
        if (b1 == 0xCB) {
            return DecodeSlot{fdcb_handlers[b3].handler, fdcb_handlers[b3].cycles, 4, fdcb_handlers[b3].affects_f, false};
        } else if (b1 == 0xED) {
            return DecodeSlot{fd_ed_handlers[b2].handler, fd_ed_handlers[b2].cycles, fd_ed_handlers[b2].length, fd_ed_handlers[b2].affects_f, false};
        } else {
            return DecodeSlot{fd_handlers[b1].handler, fd_handlers[b1].cycles, fd_handlers[b1].length, fd_handlers[b1].affects_f, false};
        }
    } else {
        return DecodeSlot{base_handlers[opcode].handler, base_handlers[opcode].cycles, base_handlers[opcode].length, base_handlers[opcode].affects_f, base_handlers[opcode].is_ld_a_ir};
    }
}

// ============================================================
// Helper to set a DecodeSlot in the cache
// ============================================================
static void set_slot(DecodeSlot* cache, uint16_t addr, OpHandler h, uint8_t cyc, uint8_t len, bool af, bool ir) {
    cache[addr & 0xFFFF] = DecodeSlot{h, cyc, len, af, ir};
}

// ============================================================
// Build base opcode table (0x00-0xFF)
// ============================================================
static void build_base_table() {
    // LD r,r' (0x40-0x7F, excluding 0x76 HALT)
    for (int dest = 0; dest < 8; dest++) {
        for (int src = 0; src < 8; src++) {
            int op = 0x40 | (dest << 3) | src;
            if (op == 0x76) continue;
            int cyc = (dest == 6 || src == 6) ? 7 : 4;
            base_handlers[op] = OpcodeEntry{op_ld_r_r, (uint8_t)cyc, 1, false, false};
        }
    }

    // LD r,n (0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x36, 0x3E)
    for (int reg = 0; reg < 8; reg++) {
        int op = 0x06 | (reg << 3);
        int cyc = (reg == 6) ? 10 : 7;
        base_handlers[op] = OpcodeEntry{op_ld_r_n, (uint8_t)cyc, 2, false, false};
    }

    // LD A,(BC), LD A,(DE), LD A,(nn)
    base_handlers[0x0A] = OpcodeEntry{op_ld_a_bc, 7, 1, false, false};
    base_handlers[0x1A] = OpcodeEntry{op_ld_a_de, 7, 1, false, false};
    base_handlers[0x3A] = OpcodeEntry{op_ld_a_nn, 13, 3, false, false};

    // LD (BC),A, LD (DE),A, LD (nn),A
    base_handlers[0x02] = OpcodeEntry{op_ld_bc_a, 7, 1, false, false};
    base_handlers[0x12] = OpcodeEntry{op_ld_de_a, 7, 1, false, false};
    base_handlers[0x32] = OpcodeEntry{op_ld_nn_a, 13, 3, false, false};

    // LD rr,nn
    base_handlers[0x01] = OpcodeEntry{op_ld_rr_nn, 10, 3, false, false};
    base_handlers[0x11] = OpcodeEntry{op_ld_rr_nn, 10, 3, false, false};
    base_handlers[0x21] = OpcodeEntry{op_ld_rr_nn, 10, 3, false, false};
    base_handlers[0x31] = OpcodeEntry{op_ld_rr_nn, 10, 3, false, false};

    // LD HL,(nn), LD (nn),HL
    base_handlers[0x2A] = OpcodeEntry{op_ld_hl_nn, 16, 3, false, false};
    base_handlers[0x22] = OpcodeEntry{op_ld_nn_hl, 16, 3, false, false};

    // LD SP,HL
    base_handlers[0xF9] = OpcodeEntry{op_ld_sp_hl, 6, 1, false, false};

    // PUSH/POP
    base_handlers[0xC5] = OpcodeEntry{op_push_rr, 11, 1, false, false};
    base_handlers[0xD5] = OpcodeEntry{op_push_rr, 11, 1, false, false};
    base_handlers[0xE5] = OpcodeEntry{op_push_rr, 11, 1, false, false};
    base_handlers[0xF5] = OpcodeEntry{op_push_rr, 11, 1, false, false};
    base_handlers[0xC1] = OpcodeEntry{op_pop_rr, 10, 1, false, false};
    base_handlers[0xD1] = OpcodeEntry{op_pop_rr, 10, 1, false, false};
    base_handlers[0xE1] = OpcodeEntry{op_pop_rr, 10, 1, false, false};
    base_handlers[0xF1] = OpcodeEntry{op_pop_rr, 10, 1, false, false};

    // Exchange
    base_handlers[0xEB] = OpcodeEntry{op_ex_de_hl, 4, 1, false, false};
    base_handlers[0x08] = OpcodeEntry{op_ex_af_afp, 4, 1, false, false};
    base_handlers[0xD9] = OpcodeEntry{op_exx, 4, 1, false, false};
    base_handlers[0xE3] = OpcodeEntry{op_ex_sp_hl, 19, 1, false, false};

    // ADD A,r
    for (int reg = 0; reg < 8; reg++) {
        int op = 0x80 | reg;
        int cyc = (reg == 6) ? 7 : 4;
        base_handlers[op] = OpcodeEntry{op_add_a, (uint8_t)cyc, 1, true, false};
    }
    base_handlers[0xC6] = OpcodeEntry{op_add_a_n, 7, 2, true, false};

    // ADC A,r
    for (int reg = 0; reg < 8; reg++) {
        int op = 0x88 | reg;
        int cyc = (reg == 6) ? 7 : 4;
        base_handlers[op] = OpcodeEntry{op_adc_a, (uint8_t)cyc, 1, true, false};
    }
    base_handlers[0xCE] = OpcodeEntry{op_adc_a_n, 7, 2, true, false};

    // SUB r
    for (int reg = 0; reg < 8; reg++) {
        int op = 0x90 | reg;
        int cyc = (reg == 6) ? 7 : 4;
        base_handlers[op] = OpcodeEntry{op_sub, (uint8_t)cyc, 1, true, false};
    }
    base_handlers[0xD6] = OpcodeEntry{op_sub_n, 7, 2, true, false};

    // SBC A,r
    for (int reg = 0; reg < 8; reg++) {
        int op = 0x98 | reg;
        int cyc = (reg == 6) ? 7 : 4;
        base_handlers[op] = OpcodeEntry{op_sbc_a, (uint8_t)cyc, 1, true, false};
    }
    base_handlers[0xDE] = OpcodeEntry{op_sbc_a_n, 7, 2, true, false};

    // AND r
    for (int reg = 0; reg < 8; reg++) {
        int op = 0xA0 | reg;
        int cyc = (reg == 6) ? 7 : 4;
        base_handlers[op] = OpcodeEntry{op_and, (uint8_t)cyc, 1, true, false};
    }
    base_handlers[0xE6] = OpcodeEntry{op_and_n, 7, 2, true, false};

    // XOR r
    for (int reg = 0; reg < 8; reg++) {
        int op = 0xA8 | reg;
        int cyc = (reg == 6) ? 7 : 4;
        base_handlers[op] = OpcodeEntry{op_xor, (uint8_t)cyc, 1, true, false};
    }
    base_handlers[0xEE] = OpcodeEntry{op_xor_n, 7, 2, true, false};

    // OR r
    for (int reg = 0; reg < 8; reg++) {
        int op = 0xB0 | reg;
        int cyc = (reg == 6) ? 7 : 4;
        base_handlers[op] = OpcodeEntry{op_or, (uint8_t)cyc, 1, true, false};
    }
    base_handlers[0xF6] = OpcodeEntry{op_or_n, 7, 2, true, false};

    // CP r
    for (int reg = 0; reg < 8; reg++) {
        int op = 0xB8 | reg;
        int cyc = (reg == 6) ? 7 : 4;
        base_handlers[op] = OpcodeEntry{op_cp, (uint8_t)cyc, 1, true, false};
    }
    base_handlers[0xFE] = OpcodeEntry{op_cp_n, 7, 2, true, false};

    // INC r
    for (int reg = 0; reg < 8; reg++) {
        int op = 0x04 | (reg << 3);
        int cyc = (reg == 6) ? 11 : 4;
        base_handlers[op] = OpcodeEntry{op_inc_r, (uint8_t)cyc, 1, true, false};
    }

    // DEC r
    for (int reg = 0; reg < 8; reg++) {
        int op = 0x05 | (reg << 3);
        int cyc = (reg == 6) ? 11 : 4;
        base_handlers[op] = OpcodeEntry{op_dec_r, (uint8_t)cyc, 1, true, false};
    }

    // ADD HL,rr
    base_handlers[0x09] = OpcodeEntry{op_add_hl_rr, 11, 1, true, false};
    base_handlers[0x19] = OpcodeEntry{op_add_hl_rr, 11, 1, true, false};
    base_handlers[0x29] = OpcodeEntry{op_add_hl_rr, 11, 1, true, false};
    base_handlers[0x39] = OpcodeEntry{op_add_hl_rr, 11, 1, true, false};

    // INC rr
    base_handlers[0x03] = OpcodeEntry{op_inc_rr, 6, 1, false, false};
    base_handlers[0x13] = OpcodeEntry{op_inc_rr, 6, 1, false, false};
    base_handlers[0x23] = OpcodeEntry{op_inc_rr, 6, 1, false, false};
    base_handlers[0x33] = OpcodeEntry{op_inc_rr, 6, 1, false, false};

    // DEC rr
    base_handlers[0x0B] = OpcodeEntry{op_dec_rr, 6, 1, false, false};
    base_handlers[0x1B] = OpcodeEntry{op_dec_rr, 6, 1, false, false};
    base_handlers[0x2B] = OpcodeEntry{op_dec_rr, 6, 1, false, false};
    base_handlers[0x3B] = OpcodeEntry{op_dec_rr, 6, 1, false, false};

    // DAA, CPL, CCF, SCF
    base_handlers[0x27] = OpcodeEntry{op_daa, 4, 1, true, false};
    base_handlers[0x2F] = OpcodeEntry{op_cpl, 4, 1, true, false};
    base_handlers[0x3F] = OpcodeEntry{op_ccf, 4, 1, true, false};
    base_handlers[0x37] = OpcodeEntry{op_scf, 4, 1, true, false};

    // Rotates
    base_handlers[0x07] = OpcodeEntry{op_rlca, 4, 1, true, false};
    base_handlers[0x0F] = OpcodeEntry{op_rrca, 4, 1, true, false};
    base_handlers[0x17] = OpcodeEntry{op_rla, 4, 1, true, false};
    base_handlers[0x1F] = OpcodeEntry{op_rra, 4, 1, true, false};

    // JP
    base_handlers[0xC3] = OpcodeEntry{op_jp_nn, 10, 3, false, false};
    base_handlers[0xE9] = OpcodeEntry{op_jp_hl, 4, 1, false, false};
    for (int cc = 0; cc < 8; cc++) {
        base_handlers[0xC2 | (cc << 3)] = OpcodeEntry{op_jp_cc_nn, 10, 3, false, false};
    }

    // JR
    base_handlers[0x18] = OpcodeEntry{op_jr_e, 12, 2, false, false};
    base_handlers[0x10] = OpcodeEntry{op_djnz_e, 8, 2, false, false};
    for (int cc = 0; cc < 4; cc++) {
        base_handlers[0x20 | (cc << 3)] = OpcodeEntry{op_jr_cc_e, 7, 2, false, false};
    }

    // CALL
    base_handlers[0xCD] = OpcodeEntry{op_call_nn, 17, 3, false, false};
    for (int cc = 0; cc < 8; cc++) {
        base_handlers[0xC4 | (cc << 3)] = OpcodeEntry{op_call_cc_nn, 10, 3, false, false};
    }

    // RET
    base_handlers[0xC9] = OpcodeEntry{op_ret, 10, 1, false, false};
    for (int cc = 0; cc < 8; cc++) {
        base_handlers[0xC0 | (cc << 3)] = OpcodeEntry{op_ret_cc, 5, 1, false, false};
    }

    // RST
    for (int p = 0; p < 8; p++) {
        base_handlers[0xC7 | (p << 3)] = OpcodeEntry{op_rst, 11, 1, false, false};
    }

    // I/O
    base_handlers[0xDB] = OpcodeEntry{op_in_a_n, 11, 2, false, false};
    base_handlers[0xD3] = OpcodeEntry{op_out_n_a, 11, 2, false, false};

    // NOP, HALT, DI, EI
    base_handlers[0x00] = OpcodeEntry{op_nop, 4, 1, false, false};
    base_handlers[0x76] = OpcodeEntry{op_halt, 4, 1, false, false};
    base_handlers[0xF3] = OpcodeEntry{op_di, 4, 1, false, false};
    base_handlers[0xFB] = OpcodeEntry{op_ei, 4, 1, false, false};
}

// ============================================================
// Build CB table
// ============================================================
static void build_cb_table() {
    for (int op = 0; op < 256; op++) {
        int op_type = (op >> 6) & 3;
        int bit_or_op = (op >> 3) & 7;
        int reg = op & 7;

        if (op_type == 0) {
            // Rotates/shifts
            if (reg == 6) {
                cb_handlers[op] = OpcodeEntry{op_cb_rot, 15, 2, true, false};
            } else {
                cb_handlers[op] = OpcodeEntry{op_cb_rot, 8, 2, true, false};
            }
        } else if (op_type == 1) {
            // BIT
            if (reg == 6) {
                cb_handlers[op] = OpcodeEntry{op_cb_bit, 12, 2, true, false};
            } else {
                cb_handlers[op] = OpcodeEntry{op_cb_bit, 8, 2, true, false};
            }
        } else if (op_type == 2) {
            // RES
            if (reg == 6) {
                cb_handlers[op] = OpcodeEntry{op_cb_res_hl, 15, 2, false, false};
            } else {
                cb_handlers[op] = OpcodeEntry{op_cb_res, 8, 2, false, false};
            }
        } else {
            // SET
            if (reg == 6) {
                cb_handlers[op] = OpcodeEntry{op_cb_set_hl, 15, 2, false, false};
            } else {
                cb_handlers[op] = OpcodeEntry{op_cb_set, 8, 2, false, false};
            }
        }
    }
}

// ============================================================
// Build ED table
// ============================================================
static void build_ed_table() {
    // Block transfers
    ed_handlers[0xA0] = OpcodeEntry{op_ldi, 16, 2, true, false};
    ed_handlers[0xB0] = OpcodeEntry{op_ldir, 16, 2, true, false};
    ed_handlers[0xA8] = OpcodeEntry{op_ldd, 16, 2, true, false};
    ed_handlers[0xB8] = OpcodeEntry{op_lddr, 16, 2, true, false};

    // Block compares
    ed_handlers[0xA1] = OpcodeEntry{op_cpi, 16, 2, true, false};
    ed_handlers[0xB1] = OpcodeEntry{op_cpir, 16, 2, true, false};
    ed_handlers[0xA9] = OpcodeEntry{op_cpd, 16, 2, true, false};
    ed_handlers[0xB9] = OpcodeEntry{op_cpdr, 16, 2, true, false};

    // Block I/O
    ed_handlers[0xA2] = OpcodeEntry{op_ini, 16, 2, true, false};
    ed_handlers[0xB2] = OpcodeEntry{op_inir, 16, 2, true, false};
    ed_handlers[0xAA] = OpcodeEntry{op_ind, 16, 2, true, false};
    ed_handlers[0xBA] = OpcodeEntry{op_indr, 16, 2, true, false};
    ed_handlers[0xA3] = OpcodeEntry{op_outi, 16, 2, true, false};
    ed_handlers[0xB3] = OpcodeEntry{op_otir, 16, 2, true, false};
    ed_handlers[0xAB] = OpcodeEntry{op_outd, 16, 2, true, false};
    ed_handlers[0xBB] = OpcodeEntry{op_otdr, 16, 2, true, false};

    // ADC/SBC HL,rr
    ed_handlers[0x4A] = OpcodeEntry{op_adc_hl_rr, 15, 2, true, false};
    ed_handlers[0x5A] = OpcodeEntry{op_adc_hl_rr, 15, 2, true, false};
    ed_handlers[0x6A] = OpcodeEntry{op_adc_hl_rr, 15, 2, true, false};
    ed_handlers[0x7A] = OpcodeEntry{op_adc_hl_rr, 15, 2, true, false};
    ed_handlers[0x42] = OpcodeEntry{op_sbc_hl_rr, 15, 2, true, false};
    ed_handlers[0x52] = OpcodeEntry{op_sbc_hl_rr, 15, 2, true, false};
    ed_handlers[0x62] = OpcodeEntry{op_sbc_hl_rr, 15, 2, true, false};
    ed_handlers[0x72] = OpcodeEntry{op_sbc_hl_rr, 15, 2, true, false};

    // LD (nn),rr / LD rr,(nn)
    ed_handlers[0x4B] = OpcodeEntry{op_ld_rr_nn_ind, 20, 4, false, false};
    ed_handlers[0x5B] = OpcodeEntry{op_ld_rr_nn_ind, 20, 4, false, false};
    ed_handlers[0x6B] = OpcodeEntry{op_ld_rr_nn_ind, 20, 4, false, false};
    ed_handlers[0x7B] = OpcodeEntry{op_ld_rr_nn_ind, 20, 4, false, false};
    ed_handlers[0x43] = OpcodeEntry{op_ld_nn_rr, 20, 4, false, false};
    ed_handlers[0x53] = OpcodeEntry{op_ld_nn_rr, 20, 4, false, false};
    ed_handlers[0x63] = OpcodeEntry{op_ld_nn_rr, 20, 4, false, false};
    ed_handlers[0x73] = OpcodeEntry{op_ld_nn_rr, 20, 4, false, false};

    // NEG
    for (int op : {0x44, 0x54, 0x64, 0x74, 0x4C, 0x5C, 0x6C, 0x7C}) {
        ed_handlers[op] = OpcodeEntry{op_neg, 8, 2, true, false};
    }

    // RETI/RETN
    ed_handlers[0x4D] = OpcodeEntry{op_reti, 14, 2, false, false};
    for (int op : {0x45, 0x55, 0x65, 0x75, 0x5D, 0x6D, 0x7D}) {
        ed_handlers[op] = OpcodeEntry{op_retn, 14, 2, false, false};
    }

    // IM
    for (int op : {0x46, 0x66, 0x4E, 0x6E}) ed_handlers[op] = OpcodeEntry{op_im, 8, 2, false, false};
    for (int op : {0x56, 0x76}) ed_handlers[op] = OpcodeEntry{op_im, 8, 2, false, false};
    for (int op : {0x5E, 0x7E}) ed_handlers[op] = OpcodeEntry{op_im, 8, 2, false, false};

    // IN r,(C)
    for (int op = 0x40; op <= 0x78; op += 8) {
        ed_handlers[op] = OpcodeEntry{op_in_r_c, 12, 2, true, false};
    }
    // OUT (C),r
    for (int op = 0x41; op <= 0x79; op += 8) {
        ed_handlers[op] = OpcodeEntry{op_out_c_r, 12, 2, false, false};
    }
    // OUT (C),0
    ed_handlers[0x71] = OpcodeEntry{op_out_c_r, 12, 2, false, false};

    // LD I,A / LD R,A / LD A,I / LD A,R
    ed_handlers[0x47] = OpcodeEntry{op_ld_i_a, 9, 2, false, false};
    ed_handlers[0x4F] = OpcodeEntry{op_ld_r_a, 9, 2, false, false};
    ed_handlers[0x57] = OpcodeEntry{op_ld_a_i, 9, 2, true, true};
    ed_handlers[0x5F] = OpcodeEntry{op_ld_a_r, 9, 2, true, true};

    // RLD/RRD
    ed_handlers[0x6F] = OpcodeEntry{op_rld, 18, 2, true, false};
    ed_handlers[0x67] = OpcodeEntry{op_rrd, 18, 2, true, false};

    // Undefined ED = NOP
    for (int op = 0; op < 256; op++) {
        if (ed_handlers[op].handler == nullptr) {
            ed_handlers[op] = OpcodeEntry{op_nop, 8, 2, false, false};
        }
    }
}

// ============================================================
// Build DD/FD tables
// ============================================================
static void build_dd_fd_table(OpcodeEntry* table, bool is_iy) {
    // NOTE: Do NOT copy base handlers here.
    // Undefined DD/FD opcodes should have handler=nullptr so that
    // step() can execute them as fallthrough to base instructions.
    
    // LD IX/IY,nn
    table[0x21] = OpcodeEntry{op_dd_fd_ld_ix_nn, 14, 4, false, false};
    table[0x22] = OpcodeEntry{op_dd_fd_ld_nn_ix, 20, 4, false, false};
    table[0x2A] = OpcodeEntry{op_dd_fd_ld_ix_nn_ind, 20, 4, false, false};
    table[0x23] = OpcodeEntry{op_dd_fd_inc_ix, 10, 2, false, false};
    table[0x2B] = OpcodeEntry{op_dd_fd_dec_ix, 10, 2, false, false};
    table[0xF9] = OpcodeEntry{op_dd_fd_ld_sp_ix, 10, 2, false, false};
    table[0xE1] = OpcodeEntry{op_dd_fd_pop_ix, 14, 2, false, false};
    table[0xE3] = OpcodeEntry{op_dd_fd_ex_sp_ix, 23, 2, false, false};
    table[0xE5] = OpcodeEntry{op_dd_fd_push_ix, 15, 2, false, false};

    // ADD IX/IY,rr
    table[0x09] = OpcodeEntry{op_dd_fd_add_ix_rr, 15, 2, true, false};
    table[0x19] = OpcodeEntry{op_dd_fd_add_ix_rr, 15, 2, true, false};
    table[0x29] = OpcodeEntry{op_dd_fd_add_ix_rr, 15, 2, true, false};
    table[0x39] = OpcodeEntry{op_dd_fd_add_ix_rr, 15, 2, true, false};

    // IXH/IYH, IXL/IYL
    table[0x24] = OpcodeEntry{op_dd_fd_inc_ixh, 8, 2, true, false};
    table[0x25] = OpcodeEntry{op_dd_fd_dec_ixh, 8, 2, true, false};
    table[0x26] = OpcodeEntry{op_dd_fd_ld_ixh_n, 11, 3, false, false};
    table[0x2C] = OpcodeEntry{op_dd_fd_inc_ixl, 8, 2, true, false};
    table[0x2D] = OpcodeEntry{op_dd_fd_dec_ixl, 8, 2, true, false};
    table[0x2E] = OpcodeEntry{op_dd_fd_ld_ixl_n, 11, 3, false, false};

    // LD r,IXH/IYH
    table[0x44] = OpcodeEntry{op_dd_fd_ld_r_ixh, 8, 2, false, false};
    table[0x4C] = OpcodeEntry{op_dd_fd_ld_r_ixh, 8, 2, false, false};
    table[0x54] = OpcodeEntry{op_dd_fd_ld_r_ixh, 8, 2, false, false};
    table[0x5C] = OpcodeEntry{op_dd_fd_ld_r_ixh, 8, 2, false, false};

    // LD r,IXL/IYL
    table[0x45] = OpcodeEntry{op_dd_fd_ld_r_ixl, 8, 2, false, false};
    table[0x4D] = OpcodeEntry{op_dd_fd_ld_r_ixl, 8, 2, false, false};
    table[0x55] = OpcodeEntry{op_dd_fd_ld_r_ixl, 8, 2, false, false};
    table[0x5D] = OpcodeEntry{op_dd_fd_ld_r_ixl, 8, 2, false, false};

    // LD IXH/IYH,r
    table[0x60] = OpcodeEntry{op_dd_fd_ld_ixh_r, 8, 2, false, false};
    table[0x61] = OpcodeEntry{op_dd_fd_ld_ixh_r, 8, 2, false, false};
    table[0x62] = OpcodeEntry{op_dd_fd_ld_ixh_r, 8, 2, false, false};
    table[0x63] = OpcodeEntry{op_dd_fd_ld_ixh_r, 8, 2, false, false};
    table[0x64] = OpcodeEntry{op_dd_fd_ld_ixh_ixh, 8, 2, false, false};
    table[0x65] = OpcodeEntry{op_dd_fd_ld_ixh_ixl, 8, 2, false, false};
    table[0x67] = OpcodeEntry{op_dd_fd_ld_ixh_r, 8, 2, false, false};
    table[0x68] = OpcodeEntry{op_dd_fd_ld_ixl_r, 8, 2, false, false};
    table[0x69] = OpcodeEntry{op_dd_fd_ld_ixl_r, 8, 2, false, false};
    table[0x6A] = OpcodeEntry{op_dd_fd_ld_ixl_r, 8, 2, false, false};
    table[0x6B] = OpcodeEntry{op_dd_fd_ld_ixl_r, 8, 2, false, false};
    table[0x6C] = OpcodeEntry{op_dd_fd_ld_ixl_ixh, 8, 2, false, false};
    table[0x6D] = OpcodeEntry{op_dd_fd_ld_ixl_ixl, 8, 2, false, false};
    table[0x6F] = OpcodeEntry{op_dd_fd_ld_ixl_r, 8, 2, false, false};

    // LD A,IXH/IYH, LD A,IXL/IYL
    table[0x7C] = OpcodeEntry{op_dd_fd_ld_a_ixh, 8, 2, false, false};
    table[0x7D] = OpcodeEntry{op_dd_fd_ld_a_ixl, 8, 2, false, false};

    // INC/DEC (IX/IY+d)
    table[0x34] = OpcodeEntry{op_dd_fd_inc_ixd, 23, 3, true, false};
    table[0x35] = OpcodeEntry{op_dd_fd_dec_ixd, 23, 3, true, false};
    table[0x36] = OpcodeEntry{op_dd_fd_ld_ixd_n, 19, 4, false, false};

    // ALU with IXH/IXL
    table[0x84] = OpcodeEntry{op_dd_fd_alu_ixh, 8, 2, true, false};
    table[0x85] = OpcodeEntry{op_dd_fd_alu_ixl, 8, 2, true, false};
    table[0x8C] = OpcodeEntry{op_dd_fd_alu_ixh, 8, 2, true, false};
    table[0x8D] = OpcodeEntry{op_dd_fd_alu_ixl, 8, 2, true, false};
    table[0x94] = OpcodeEntry{op_dd_fd_alu_ixh, 8, 2, true, false};
    table[0x95] = OpcodeEntry{op_dd_fd_alu_ixl, 8, 2, true, false};
    table[0x9C] = OpcodeEntry{op_dd_fd_alu_ixh, 8, 2, true, false};
    table[0x9D] = OpcodeEntry{op_dd_fd_alu_ixl, 8, 2, true, false};
    table[0xA4] = OpcodeEntry{op_dd_fd_alu_ixh, 8, 2, true, false};
    table[0xA5] = OpcodeEntry{op_dd_fd_alu_ixl, 8, 2, true, false};
    table[0xAC] = OpcodeEntry{op_dd_fd_alu_ixh, 8, 2, true, false};
    table[0xAD] = OpcodeEntry{op_dd_fd_alu_ixl, 8, 2, true, false};
    table[0xB4] = OpcodeEntry{op_dd_fd_alu_ixh, 8, 2, true, false};
    table[0xB5] = OpcodeEntry{op_dd_fd_alu_ixl, 8, 2, true, false};
    table[0xBC] = OpcodeEntry{op_dd_fd_alu_ixh, 8, 2, true, false};
    table[0xBD] = OpcodeEntry{op_dd_fd_alu_ixl, 8, 2, true, false};

    // ALU with (IX/IY+d)
    table[0x86] = OpcodeEntry{op_dd_fd_alu_ixd, 19, 3, true, false};
    table[0x8E] = OpcodeEntry{op_dd_fd_alu_ixd, 19, 3, true, false};
    table[0x96] = OpcodeEntry{op_dd_fd_alu_ixd, 19, 3, true, false};
    table[0x9E] = OpcodeEntry{op_dd_fd_alu_ixd, 19, 3, true, false};
    table[0xA6] = OpcodeEntry{op_dd_fd_alu_ixd, 19, 3, true, false};
    table[0xAE] = OpcodeEntry{op_dd_fd_alu_ixd, 19, 3, true, false};
    table[0xB6] = OpcodeEntry{op_dd_fd_alu_ixd, 19, 3, true, false};
    table[0xBE] = OpcodeEntry{op_dd_fd_alu_ixd, 19, 3, true, false};

    // LD r,(IX/IY+d)
    for (int reg = 0; reg < 8; reg++) {
        if (reg != 6) {
            table[0x46 | (reg << 3)] = OpcodeEntry{op_dd_fd_ld_r_ixd, 19, 3, false, false};
            table[0x70 | reg] = OpcodeEntry{op_dd_fd_ld_ixd_r, 19, 3, false, false};
        }
    }

    // JP (IX/IY)
    table[0xE9] = OpcodeEntry{op_dd_fd_jp_ix, 8, 2, false, false};

    // Undefined DD/FD = fall through to base (leave as nullptr)
    // The step() function will detect null handler and use base_handlers instead
}

// ============================================================
// Build DDCB/FDCB tables
// ============================================================
static void build_ddcb_fdcb_table(OpcodeEntry* table) {
    for (int op = 0; op < 256; op++) {
        int op_type = (op >> 6) & 3;
        int reg = op & 7;

        if (op_type == 0) {
            // Rotates
            table[op] = OpcodeEntry{op_ddcb_fdcb_rot, 23, 4, true, false};
        } else if (op_type == 1) {
            // BIT
            table[op] = OpcodeEntry{op_ddcb_fdcb_bit, 20, 4, true, false};
        } else if (op_type == 2) {
            // RES
            table[op] = OpcodeEntry{op_ddcb_fdcb_res, 23, 4, false, false};
        } else {
            // SET
            table[op] = OpcodeEntry{op_ddcb_fdcb_set, 23, 4, false, false};
        }
    }
}

// ============================================================
// Public: build all tables
// ============================================================
void build_handler_tables() {
    std::memset(base_handlers, 0, sizeof(base_handlers));
    std::memset(cb_handlers, 0, sizeof(cb_handlers));
    std::memset(ed_handlers, 0, sizeof(ed_handlers));
    std::memset(dd_handlers, 0, sizeof(dd_handlers));
    std::memset(fd_handlers, 0, sizeof(fd_handlers));
    std::memset(ddcb_handlers, 0, sizeof(ddcb_handlers));
    std::memset(fdcb_handlers, 0, sizeof(fdcb_handlers));
    std::memset(dd_ed_handlers, 0, sizeof(dd_ed_handlers));
    std::memset(fd_ed_handlers, 0, sizeof(fd_ed_handlers));

    build_base_table();
    build_cb_table();
    build_ed_table();
    build_dd_fd_table(dd_handlers, false);
    build_dd_fd_table(fd_handlers, true);
    build_ddcb_fdcb_table(ddcb_handlers);
    build_ddcb_fdcb_table(fdcb_handlers);
    
    // DD ED xx and FD ED xx handlers (ADC/SBC IX/IY,rr)
    dd_ed_handlers[0x4A] = OpcodeEntry{op_dd_fd_adc_ix_rr, 15, 4, true, false};
    dd_ed_handlers[0x5A] = OpcodeEntry{op_dd_fd_adc_ix_rr, 15, 4, true, false};
    dd_ed_handlers[0x6A] = OpcodeEntry{op_dd_fd_adc_ix_rr, 15, 4, true, false};
    dd_ed_handlers[0x7A] = OpcodeEntry{op_dd_fd_adc_ix_rr, 15, 4, true, false};
    dd_ed_handlers[0x42] = OpcodeEntry{op_dd_fd_sbc_ix_rr, 15, 4, true, false};
    dd_ed_handlers[0x52] = OpcodeEntry{op_dd_fd_sbc_ix_rr, 15, 4, true, false};
    dd_ed_handlers[0x62] = OpcodeEntry{op_dd_fd_sbc_ix_rr, 15, 4, true, false};
    dd_ed_handlers[0x72] = OpcodeEntry{op_dd_fd_sbc_ix_rr, 15, 4, true, false};
    
    // Same for FD prefix (IY version)
    fd_ed_handlers[0x4A] = OpcodeEntry{op_dd_fd_adc_ix_rr, 15, 4, true, false};
    fd_ed_handlers[0x5A] = OpcodeEntry{op_dd_fd_adc_ix_rr, 15, 4, true, false};
    fd_ed_handlers[0x6A] = OpcodeEntry{op_dd_fd_adc_ix_rr, 15, 4, true, false};
    fd_ed_handlers[0x7A] = OpcodeEntry{op_dd_fd_adc_ix_rr, 15, 4, true, false};
    fd_ed_handlers[0x42] = OpcodeEntry{op_dd_fd_sbc_ix_rr, 15, 4, true, false};
    fd_ed_handlers[0x52] = OpcodeEntry{op_dd_fd_sbc_ix_rr, 15, 4, true, false};
    fd_ed_handlers[0x62] = OpcodeEntry{op_dd_fd_sbc_ix_rr, 15, 4, true, false};
    fd_ed_handlers[0x72] = OpcodeEntry{op_dd_fd_sbc_ix_rr, 15, 4, true, false};
}
