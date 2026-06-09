#include "../include/z80/opcode_table.h"
#include "../include/z80/handlers.h"

namespace z80 {

std::array<Instruction, 256> OpcodeTable::main_table;
std::array<Instruction, 256> OpcodeTable::cb_table;
std::array<Instruction, 256> OpcodeTable::ed_table;
std::array<Instruction, 256> OpcodeTable::dd_table;
std::array<Instruction, 256> OpcodeTable::fd_table;
std::array<Instruction, 256> OpcodeTable::ddcb_table;
std::array<Instruction, 256> OpcodeTable::fdcb_table;

// Convenience macro: set both DD and FD tables identically
#define BOTH(_op, fn, cy, ln, af) \
    dd_table[_op] = Instruction(fn, cy, ln, af); \
    fd_table[_op] = Instruction(fn, cy, ln, af)

void OpcodeTable::init() {
    // Initialise flag tables first
    FlagTables::init();

    // Default entries — null handler means "unimplemented / fall through"
    for (int i = 0; i < 256; ++i) {
        main_table[i] = {nullptr, 4, 1, false};
        cb_table[i]   = {nullptr, 8, 2, false};
        ed_table[i]   = {nullptr, 8, 2, false};
        dd_table[i]   = {nullptr, 0, 0, false};
        fd_table[i]   = {nullptr, 0, 0, false};
        ddcb_table[i] = {nullptr, 23, 4, false};
        fdcb_table[i] = {nullptr, 23, 4, false};
    }

    // ================================================================
    // Main opcode table (unprefixed)
    // ================================================================

    main_table[0x00] = {handle_nop,        4,  1, false};
    main_table[0x01] = {handle_ld_rr_nn,  10,  3, false};
    main_table[0x02] = {handle_ld_bc_a,    7,  1, false};
    main_table[0x03] = {handle_inc_rr,     6,  1, false};
    main_table[0x04] = {handle_inc_r,      4,  1, true};
    main_table[0x05] = {handle_dec_r,      4,  1, true};
    main_table[0x06] = {handle_ld_r_n,     7,  2, false};
    main_table[0x07] = {handle_rlca,       4,  1, true};

    main_table[0x08] = {handle_ex_af_afp,  4,  1, false};
    main_table[0x09] = {handle_add_hl_rr, 11,  1, true};
    main_table[0x0A] = {handle_ld_a_bc,    7,  1, false};
    main_table[0x0B] = {handle_dec_rr,     6,  1, false};
    main_table[0x0C] = {handle_inc_r,      4,  1, true};
    main_table[0x0D] = {handle_dec_r,      4,  1, true};
    main_table[0x0E] = {handle_ld_r_n,     7,  2, false};
    main_table[0x0F] = {handle_rrca,       4,  1, true};

    main_table[0x10] = {handle_djnz_e,    13,  2, false};
    main_table[0x11] = {handle_ld_rr_nn,  10,  3, false};
    main_table[0x12] = {handle_ld_de_a,    7,  1, false};
    main_table[0x13] = {handle_inc_rr,     6,  1, false};
    main_table[0x14] = {handle_inc_r,      4,  1, true};
    main_table[0x15] = {handle_dec_r,      4,  1, true};
    main_table[0x16] = {handle_ld_r_n,     7,  2, false};
    main_table[0x17] = {handle_rla,        4,  1, true};

    main_table[0x18] = {handle_jr_e,      12,  2, false};
    main_table[0x19] = {handle_add_hl_rr, 11,  1, true};
    main_table[0x1A] = {handle_ld_a_de,    7,  1, false};
    main_table[0x1B] = {handle_dec_rr,     6,  1, false};
    main_table[0x1C] = {handle_inc_r,      4,  1, true};
    main_table[0x1D] = {handle_dec_r,      4,  1, true};
    main_table[0x1E] = {handle_ld_r_n,     7,  2, false};
    main_table[0x1F] = {handle_rra,        4,  1, true};

    main_table[0x20] = {handle_jr_cc_e,   12,  2, false};  // JR NZ,e
    main_table[0x21] = {handle_ld_rr_nn,  10,  3, false};
    main_table[0x22] = {handle_ld_nn_hl,  16,  3, false};
    main_table[0x23] = {handle_inc_rr,     6,  1, false};
    main_table[0x24] = {handle_inc_r,      4,  1, true};
    main_table[0x25] = {handle_dec_r,      4,  1, true};
    main_table[0x26] = {handle_ld_r_n,     7,  2, false};
    main_table[0x27] = {handle_daa,        4,  1, true};

    main_table[0x28] = {handle_jr_cc_e,   12,  2, false};  // JR Z,e
    main_table[0x29] = {handle_add_hl_rr, 11,  1, true};
    main_table[0x2A] = {handle_ld_hl_nn,  16,  3, false};
    main_table[0x2B] = {handle_dec_rr,     6,  1, false};
    main_table[0x2C] = {handle_inc_r,      4,  1, true};
    main_table[0x2D] = {handle_dec_r,      4,  1, true};
    main_table[0x2E] = {handle_ld_r_n,     7,  2, false};
    main_table[0x2F] = {handle_cpl,        4,  1, true};

    main_table[0x30] = {handle_jr_cc_e,   12,  2, false};  // JR NC,e
    main_table[0x31] = {handle_ld_rr_nn,  10,  3, false};
    main_table[0x32] = {handle_ld_nn_a,   13,  3, false};
    main_table[0x33] = {handle_inc_rr,     6,  1, false};
    main_table[0x34] = {handle_inc_hl,    11,  1, true};
    main_table[0x35] = {handle_dec_hl,    11,  1, true};
    main_table[0x36] = {handle_ld_hl_n,   10,  2, false};
    main_table[0x37] = {handle_scf,        4,  1, true};

    main_table[0x38] = {handle_jr_cc_e,   12,  2, false};  // JR C,e
    main_table[0x39] = {handle_add_hl_rr, 11,  1, true};
    main_table[0x3A] = {handle_ld_a_nn,   13,  3, false};
    main_table[0x3B] = {handle_dec_rr,     6,  1, false};
    main_table[0x3C] = {handle_inc_r,      4,  1, true};
    main_table[0x3D] = {handle_dec_r,      4,  1, true};
    main_table[0x3E] = {handle_ld_r_n,     7,  2, false};
    main_table[0x3F] = {handle_ccf,        4,  1, true};

    // 0x40-0x7F: LD r,r / LD r,(HL) / LD (HL),r / HALT
    for (int op = 0x40; op <= 0x7F; ++op) {
        int dst = (op >> 3) & 7;
        int src = op & 7;
        if (op == 0x76) {
            main_table[op] = Instruction(handle_halt, 4, 1, false);
        } else if (src == 6) {
            main_table[op] = Instruction(handle_ld_r_hl, 7, 1, false);   // LD r,(HL)
        } else if (dst == 6) {
            main_table[op] = Instruction(handle_ld_hl_r, 7, 1, false);   // LD (HL),r
        } else {
            main_table[op] = Instruction(handle_ld_r_r, 4, 1, false);
        }
    }

    // 0x80-0xBF: ALU r / ALU (HL)
    for (int op = 0x80; op <= 0xBF; ++op) {
        int src  = op & 7;
        int alu  = (op >> 3) & 7;
        int cy   = (src == 6) ? 7 : 4;
        OpHandler fn;
        switch (alu) {
            case 0: fn = handle_add_a; break;
            case 1: fn = handle_adc_a; break;
            case 2: fn = handle_sub;   break;
            case 3: fn = handle_sbc_a; break;
            case 4: fn = handle_and;   break;
            case 5: fn = handle_xor;   break;
            case 6: fn = handle_or;    break;
            default:fn = handle_cp;    break;
        }
        main_table[op] = {fn, (uint8_t)cy, 1, true};
    }

    // 0xC0-0xFF
    main_table[0xC0] = {handle_ret_cc,    11,  1, false};
    main_table[0xC1] = {handle_pop_rr,    10,  1, false};
    main_table[0xC2] = {handle_jp_cc_nn,  10,  3, false};
    main_table[0xC3] = {handle_jp_nn,     10,  3, false};
    main_table[0xC4] = {handle_call_cc_nn,17,  3, false};
    main_table[0xC5] = {handle_push_rr,   11,  1, false};
    main_table[0xC6] = {handle_add_a_n,    7,  2, true};
    main_table[0xC7] = {handle_rst,       11,  1, false};

    main_table[0xC8] = {handle_ret_cc,    11,  1, false};
    main_table[0xC9] = {handle_ret,       10,  1, false};
    main_table[0xCA] = {handle_jp_cc_nn,  10,  3, false};
    // 0xCB = prefix
    main_table[0xCC] = {handle_call_cc_nn,17,  3, false};
    main_table[0xCD] = {handle_call_nn,   17,  3, false};
    main_table[0xCE] = {handle_adc_a_n,    7,  2, true};
    main_table[0xCF] = {handle_rst,       11,  1, false};

    main_table[0xD0] = {handle_ret_cc,    11,  1, false};
    main_table[0xD1] = {handle_pop_rr,    10,  1, false};
    main_table[0xD2] = {handle_jp_cc_nn,  10,  3, false};
    main_table[0xD3] = {handle_out_n_a,   11,  2, false};
    main_table[0xD4] = {handle_call_cc_nn,17,  3, false};
    main_table[0xD5] = {handle_push_rr,   11,  1, false};
    main_table[0xD6] = {handle_sub_n,      7,  2, true};
    main_table[0xD7] = {handle_rst,       11,  1, false};

    main_table[0xD8] = {handle_ret_cc,    11,  1, false};
    main_table[0xD9] = {handle_exx,        4,  1, false};
    main_table[0xDA] = {handle_jp_cc_nn,  10,  3, false};
    main_table[0xDB] = {handle_in_a_n,    11,  2, false};
    main_table[0xDC] = {handle_call_cc_nn,17,  3, false};
    // 0xDD = prefix
    main_table[0xDE] = {handle_sbc_a_n,    7,  2, true};
    main_table[0xDF] = {handle_rst,       11,  1, false};

    main_table[0xE0] = {handle_ret_cc,    11,  1, false};
    main_table[0xE1] = {handle_pop_rr,    10,  1, false};
    main_table[0xE2] = {handle_jp_cc_nn,  10,  3, false};
    main_table[0xE3] = {handle_ex_sp_hl,  19,  1, false};
    main_table[0xE4] = {handle_call_cc_nn,17,  3, false};
    main_table[0xE5] = {handle_push_rr,   11,  1, false};
    main_table[0xE6] = {handle_and_n,      7,  2, true};
    main_table[0xE7] = {handle_rst,       11,  1, false};

    main_table[0xE8] = {handle_ret_cc,    11,  1, false};
    main_table[0xE9] = {handle_jp_hl,      4,  1, false};
    main_table[0xEA] = {handle_jp_cc_nn,  10,  3, false};
    main_table[0xEB] = {handle_ex_de_hl,   4,  1, false};
    main_table[0xEC] = {handle_call_cc_nn,17,  3, false};
    // 0xED = prefix
    main_table[0xEE] = {handle_xor_n,      7,  2, true};
    main_table[0xEF] = {handle_rst,       11,  1, false};

    main_table[0xF0] = {handle_ret_cc,    11,  1, false};
    main_table[0xF1] = {handle_pop_rr,    10,  1, false};
    main_table[0xF2] = {handle_jp_cc_nn,  10,  3, false};
    main_table[0xF3] = {handle_di,         4,  1, false};
    main_table[0xF4] = {handle_call_cc_nn,17,  3, false};
    main_table[0xF5] = {handle_push_rr,   11,  1, false};
    main_table[0xF6] = {handle_or_n,       7,  2, true};
    main_table[0xF7] = {handle_rst,       11,  1, false};

    main_table[0xF8] = {handle_ret_cc,    11,  1, false};
    main_table[0xF9] = {handle_ld_sp_hl,   6,  1, false};
    main_table[0xFA] = {handle_jp_cc_nn,  10,  3, false};
    main_table[0xFB] = {handle_ei,         4,  1, false};
    main_table[0xFC] = {handle_call_cc_nn,17,  3, false};
    // 0xFD = prefix
    main_table[0xFE] = {handle_cp_n,       7,  2, true};
    main_table[0xFF] = {handle_rst,       11,  1, false};

    // ================================================================
    // CB prefix table (0xCB nn)
    // ================================================================
    for (int op = 0x00; op <= 0x3F; ++op) {
        int src = op & 7;
        int cy  = (src == 6) ? 15 : 8;
        OpHandler fn;
        switch ((op >> 3) & 7) {
            case 0: fn = handle_rlc_r; break;
            case 1: fn = handle_rrc_r; break;
            case 2: fn = handle_rl_r;  break;
            case 3: fn = handle_rr_r;  break;
            case 4: fn = handle_sla_r; break;
            case 5: fn = handle_sra_r; break;
            case 6: fn = handle_sll_r; break;
            default:fn = handle_srl_r; break;
        }
        cb_table[op] = {fn, (uint8_t)cy, 2, true};
    }
    for (int op = 0x40; op <= 0x7F; ++op) {
        int cy = (op & 7) == 6 ? 12 : 8;
        cb_table[op] = {handle_cb_bit, (uint8_t)cy, 2, true};
    }
    for (int op = 0x80; op <= 0xBF; ++op) {
        int cy = (op & 7) == 6 ? 15 : 8;
        cb_table[op] = {handle_cb_res, (uint8_t)cy, 2, false};
    }
    for (int op = 0xC0; op <= 0xFF; ++op) {
        int cy = (op & 7) == 6 ? 15 : 8;
        cb_table[op] = {handle_cb_set, (uint8_t)cy, 2, false};
    }

    // ================================================================
    // ED prefix table (0xED nn)
    // ================================================================

    // IN r,(C) — 0x40,0x48,0x50,0x58,0x60,0x68,0x70(=discard),0x78
    for (int r = 0; r < 8; ++r)
        ed_table[0x40 + (r << 3)] = Instruction(handle_in_r_c, 12, 2, true);

    // OUT (C),r — 0x41,0x49,0x51,0x59,0x61,0x69,0x71(=0),0x79
    for (int r = 0; r < 8; ++r)
        ed_table[0x41 + (r << 3)] = Instruction(handle_out_c_r, 12, 2, false);

    // SBC HL,rr
    ed_table[0x42] = Instruction(handle_sbc_hl_rr, 15, 2, true);
    ed_table[0x52] = Instruction(handle_sbc_hl_rr, 15, 2, true);
    ed_table[0x62] = Instruction(handle_sbc_hl_rr, 15, 2, true);
    ed_table[0x72] = Instruction(handle_sbc_hl_rr, 15, 2, true);

    // LD (nn),rr
    ed_table[0x43] = Instruction(handle_ld_nn_rr_ind, 20, 4, false);
    ed_table[0x53] = Instruction(handle_ld_nn_rr_ind, 20, 4, false);
    ed_table[0x63] = Instruction(handle_ld_nn_rr_ind, 20, 4, false);
    ed_table[0x73] = Instruction(handle_ld_nn_rr_ind, 20, 4, false);

    // NEG (multiple mappings)
    for (int op : {0x44, 0x4C, 0x54, 0x5C, 0x64, 0x6C, 0x74, 0x7C})
        ed_table[op] = Instruction(handle_neg, 8, 2, true);

    // RETN (primary + undocumented mirrors)
    for (int op : {0x45, 0x55, 0x65, 0x75})
        ed_table[op] = Instruction(handle_retn, 14, 2, false);

    // RETI
    for (int op : {0x4D, 0x5D, 0x6D, 0x7D})
        ed_table[op] = Instruction(handle_reti, 14, 2, false);

    // IM 0/1/2
    ed_table[0x46] = Instruction(handle_im, 8, 2, false);
    ed_table[0x4E] = Instruction(handle_im, 8, 2, false);
    ed_table[0x56] = Instruction(handle_im, 8, 2, false);
    ed_table[0x5E] = Instruction(handle_im, 8, 2, false);
    ed_table[0x66] = Instruction(handle_im, 8, 2, false);
    ed_table[0x6E] = Instruction(handle_im, 8, 2, false);
    ed_table[0x76] = Instruction(handle_im, 8, 2, false);
    ed_table[0x7E] = Instruction(handle_im, 8, 2, false);

    // LD I,A / LD R,A / LD A,I / LD A,R
    ed_table[0x47] = {handle_ld_i_a,  9, 2, false};
    ed_table[0x4F] = {handle_ld_r_a,  9, 2, false};
    ed_table[0x57] = {handle_ld_a_i,  9, 2, true};
    ed_table[0x5F] = {handle_ld_a_r,  9, 2, true};

    // ADC HL,rr
    ed_table[0x4A] = Instruction(handle_adc_hl_rr, 15, 2, true);
    ed_table[0x5A] = Instruction(handle_adc_hl_rr, 15, 2, true);
    ed_table[0x6A] = Instruction(handle_adc_hl_rr, 15, 2, true);
    ed_table[0x7A] = Instruction(handle_adc_hl_rr, 15, 2, true);

    // LD rr,(nn)
    ed_table[0x4B] = Instruction(handle_ld_rr_nn_ind, 20, 4, false);
    ed_table[0x5B] = Instruction(handle_ld_rr_nn_ind, 20, 4, false);
    ed_table[0x6B] = Instruction(handle_ld_rr_nn_ind, 20, 4, false);
    ed_table[0x7B] = Instruction(handle_ld_rr_nn_ind, 20, 4, false);

    // RLD / RRD
    ed_table[0x6F] = Instruction(handle_rld, 18, 2, true);
    ed_table[0x67] = Instruction(handle_rrd, 18, 2, true);

    // Block transfer / compare
    ed_table[0xA0] = {handle_ldi,  16, 2, true};
    ed_table[0xA1] = {handle_cpi,  16, 2, true};
    ed_table[0xA2] = {handle_ini,  16, 2, true};
    ed_table[0xA3] = Instruction(handle_outi, 16, 2, true);
    ed_table[0xA8] = {handle_ldd,  16, 2, true};
    ed_table[0xA9] = {handle_cpd,  16, 2, true};
    ed_table[0xAA] = {handle_ind,  16, 2, true};
    ed_table[0xAB] = Instruction(handle_outd, 16, 2, true);
    // Repeat base is 16 T-states for the non-repeating core;
    // the repeating case adds 5T dynamically in the handler.
    ed_table[0xB0] = Instruction(handle_ldir, 16, 2, true);
    ed_table[0xB1] = Instruction(handle_cpir, 16, 2, true);
    ed_table[0xB2] = Instruction(handle_inir, 16, 2, true);
    ed_table[0xB3] = Instruction(handle_otir, 16, 2, true);
    ed_table[0xB8] = Instruction(handle_lddr, 16, 2, true);
    ed_table[0xB9] = Instruction(handle_cpdr, 16, 2, true);
    ed_table[0xBA] = Instruction(handle_indr, 16, 2, true);
    ed_table[0xBB] = Instruction(handle_otdr, 16, 2, true);

    // ================================================================
    // DD/FD prefix tables (IX/IY)
    // Entries left as nullptr fall through to the main table.
    // ================================================================

    // LD IX/IY,nn — 14 T-states
    BOTH(0x21, handle_dd_fd_ld_ix_nn,    14, 4, false);

    // LD (nn),IX/IY — 20 T-states
    BOTH(0x22, handle_dd_fd_ld_nn_ix,    20, 4, false);

    // LD IX/IY,(nn) — 20 T-states
    BOTH(0x2A, handle_dd_fd_ld_ix_nn_ind,20, 4, false);

    // INC IX/IY, DEC IX/IY — 10 T-states
    BOTH(0x23, handle_dd_fd_inc_ix,      10, 2, false);
    BOTH(0x2B, handle_dd_fd_dec_ix,      10, 2, false);

    // INC/DEC IXH, IXL — 8 T-states
    BOTH(0x24, handle_dd_fd_inc_ixhl,     8, 2, true);   // INC IXH/IYH
    BOTH(0x25, handle_dd_fd_dec_ixhl,     8, 2, true);   // DEC IXH/IYH
    BOTH(0x2C, handle_dd_fd_inc_ixhl,     8, 2, true);   // INC IXL/IYL
    BOTH(0x2D, handle_dd_fd_dec_ixhl,     8, 2, true);   // DEC IXL/IYL

    // LD IXH,n / LD IXL,n — 11 T-states
    BOTH(0x26, handle_dd_fd_ld_ixhl_n,   11, 3, false);  // LD IXH/IYH,n
    BOTH(0x2E, handle_dd_fd_ld_ixhl_n,   11, 3, false);  // LD IXL/IYL,n

    // ADD IX/IY,rr — 15 T-states
    BOTH(0x09, handle_dd_fd_add_ix_rr,   15, 2, true);
    BOTH(0x19, handle_dd_fd_add_ix_rr,   15, 2, true);
    BOTH(0x29, handle_dd_fd_add_ix_rr,   15, 2, true);
    BOTH(0x39, handle_dd_fd_add_ix_rr,   15, 2, true);

    // LD SP,IX/IY — 10 T-states
    BOTH(0xF9, handle_dd_fd_ld_sp_ix,    10, 2, false);

    // PUSH/POP IX/IY
    BOTH(0xE5, handle_dd_fd_push_ix,     15, 2, false);
    BOTH(0xE1, handle_dd_fd_pop_ix,      14, 2, false);

    // EX (SP),IX/IY — 23 T-states
    BOTH(0xE3, handle_dd_fd_ex_sp_ix,    23, 2, false);

    // JP (IX)/(IY) — 8 T-states
    BOTH(0xE9, handle_dd_fd_jp_ix,        8, 2, false);

    // LD r,(IX+d)/(IY+d) — 19 T-states
    // opcodes: 0x46,0x4E,0x56,0x5E,0x66,0x6E,0x7E  [and LD A,(IX+d) = 0x7E]
    for (int r = 0; r < 8; ++r) {
        if (r == 6) continue;   // 0x76 = HALT in main table, skip
        BOTH(0x46 + (r << 3), handle_dd_fd_ld_r_ixd, 19, 3, false);
    }
    BOTH(0x7E, handle_dd_fd_ld_r_ixd, 19, 3, false);   // LD A,(IX+d)

    // LD (IX+d),r — 19 T-states: opcodes 0x70-0x77 except 0x76
    for (int r = 0; r < 8; ++r) {
        if (r == 6) continue;
        BOTH(0x70 + r, handle_dd_fd_ld_ixd_r, 19, 3, false);
    }

    // LD (IX+d),n — 19 T-states
    BOTH(0x36, handle_dd_fd_ld_ixd_n, 19, 3, false);

    // INC (IX+d) / DEC (IX+d) — 23 T-states
    BOTH(0x34, handle_dd_fd_inc_ixd,  23, 3, true);
    BOTH(0x35, handle_dd_fd_dec_ixd,  23, 3, true);

    // Undocumented LD r,IXH/L and LD IXH/L,r — 8 T-states
    // Using unified handle_dd_fd_ld_ixhl which remaps H(4)→IXH(8), L(5)→IXL(9)
    // LD r,IXH  (B,C,D,E,IXH,IXL,A)
    BOTH(0x44, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x4C, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x54, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x5C, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x64, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x6C, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x7C, handle_dd_fd_ld_ixhl, 8, 2, false);
    // LD r,IXL
    BOTH(0x45, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x4D, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x55, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x5D, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x65, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x6D, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x7D, handle_dd_fd_ld_ixhl, 8, 2, false);
    // LD IXH,r  (B,C,D,E,IXH,IXL,A)
    BOTH(0x60, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x61, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x62, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x63, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x67, handle_dd_fd_ld_ixhl, 8, 2, false);
    // LD IXL,r  (B,C,D,E,IXH,IXL,A)
    BOTH(0x68, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x69, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x6A, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x6B, handle_dd_fd_ld_ixhl, 8, 2, false);
    BOTH(0x6F, handle_dd_fd_ld_ixhl, 8, 2, false);

    // Undocumented ALU with IXH/IXL — 8 T-states
    // All eight ALU ops × two half-regs
    // ADD:0x84/0x85  ADC:0x8C/0x8D  SUB:0x94/0x95  SBC:0x9C/0x9D
    // AND:0xA4/0xA5  XOR:0xAC/0xAD  OR:0xB4/0xB5   CP:0xBC/0xBD
    // ALU A,IXH/IYH  (ADD,ADC,SUB,SBC,AND,XOR,OR,CP)
    BOTH(0x84, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0x8C, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0x94, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0x9C, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0xA4, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0xAC, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0xB4, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0xBC, handle_dd_fd_alu_ixhl, 8, 2, true);
    // ALU A,IXL/IYL
    BOTH(0x85, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0x8D, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0x95, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0x9D, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0xA5, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0xAD, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0xB5, handle_dd_fd_alu_ixhl, 8, 2, true);
    BOTH(0xBD, handle_dd_fd_alu_ixhl, 8, 2, true);

    // ALU with (IX+d)/(IY+d) — 19 T-states
    // ADD:0x86  ADC:0x8E  SUB:0x96  SBC:0x9E  AND:0xA6  XOR:0xAE  OR:0xB6  CP:0xBE
    // ALU A,(IX+d)/(IY+d)
    BOTH(0x86, handle_dd_fd_alu_ixd, 19, 3, true);
    BOTH(0x8E, handle_dd_fd_alu_ixd, 19, 3, true);
    BOTH(0x96, handle_dd_fd_alu_ixd, 19, 3, true);
    BOTH(0x9E, handle_dd_fd_alu_ixd, 19, 3, true);
    BOTH(0xA6, handle_dd_fd_alu_ixd, 19, 3, true);
    BOTH(0xAE, handle_dd_fd_alu_ixd, 19, 3, true);
    BOTH(0xB6, handle_dd_fd_alu_ixd, 19, 3, true);
    BOTH(0xBE, handle_dd_fd_alu_ixd, 19, 3, true);

    // ================================================================
    // DDCB / FDCB prefix tables
    // Opcodes 0x00-0x3F: rotate/shift; 0x40-0x7F: BIT; 0x80-0xBF: RES; 0xC0-0xFF: SET
    // ================================================================
    for (int op = 0x00; op < 0x40; ++op) {
        ddcb_table[op] = Instruction(handle_ddcb_fdcb_rot, 23, 4, true);
        fdcb_table[op] = Instruction(handle_ddcb_fdcb_rot, 23, 4, true);
    }
    for (int op = 0x40; op < 0x80; ++op) {
        ddcb_table[op] = Instruction(handle_ddcb_fdcb_bit, 20, 4, true);
        fdcb_table[op] = Instruction(handle_ddcb_fdcb_bit, 20, 4, true);
    }
    for (int op = 0x80; op < 0xC0; ++op) {
        ddcb_table[op] = Instruction(handle_ddcb_fdcb_res, 23, 4, false);
        fdcb_table[op] = Instruction(handle_ddcb_fdcb_res, 23, 4, false);
    }
    for (int op = 0xC0; op <= 0xFF; ++op) {
        ddcb_table[op] = Instruction(handle_ddcb_fdcb_set, 23, 4, false);
        fdcb_table[op] = Instruction(handle_ddcb_fdcb_set, 23, 4, false);
    }
}

#undef BOTH

} // namespace z80
