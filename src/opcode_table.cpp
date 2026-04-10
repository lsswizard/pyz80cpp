#include "../include/z80/opcode_table.h"
#include "../include/z80/handlers.h"

namespace z80 {

// ============================================================
// Opcode table storage
// ============================================================
std::array<Instruction, 256> OpcodeTable::main_table;
std::array<Instruction, 256> OpcodeTable::cb_table;
std::array<Instruction, 256> OpcodeTable::ed_table;
std::array<Instruction, 256> OpcodeTable::dd_table;
std::array<Instruction, 256> OpcodeTable::fd_table;
std::array<Instruction, 256> OpcodeTable::ddcb_table;
std::array<Instruction, 256> OpcodeTable::fdcb_table;

// ============================================================
// Opcode table initialization
// ============================================================
void OpcodeTable::init() {
    // Initialize all tables with NOP handlers
    for (int i = 0; i < 256; i++) {
        main_table[i] = Instruction(nullptr, 4, 1, false);
        cb_table[i] = Instruction(nullptr, 8, 1, false);
        ed_table[i] = Instruction(nullptr, 8, 2, false);
        dd_table[i] = Instruction(nullptr, 4, 1, false);
        fd_table[i] = Instruction(nullptr, 4, 1, false);
        ddcb_table[i] = Instruction(nullptr, 23, 4, false);
        fdcb_table[i] = Instruction(nullptr, 23, 4, false);
    }

    // =====================
    // Main opcode table (0x00-0xFF)
    // =====================
    
    // 0x00-0x07: NOP, LD BC/DE/HL/SP, INC/DEC
    main_table[0x00] = {handle_nop, 4, 1, false};
    main_table[0x01] = {handle_ld_rr_nn, 10, 3, false};
    main_table[0x02] = {handle_ld_bc_a, 7, 1, false};
    main_table[0x03] = {handle_inc_rr, 6, 1, false};
    main_table[0x04] = {handle_inc_r, 4, 1, true};
    main_table[0x05] = {handle_dec_r, 4, 1, true};
    main_table[0x06] = {handle_ld_r_n, 7, 2, false};
    main_table[0x07] = {handle_rlca, 4, 1, false};
    
    // 0x08-0x0F
    main_table[0x08] = {handle_ex_af_afp, 4, 1, false};
    main_table[0x09] = {handle_add_hl_rr, 11, 1, false};
    main_table[0x0A] = {handle_ld_a_bc, 7, 1, false};
    main_table[0x0B] = {handle_dec_rr, 6, 1, false};
    main_table[0x0C] = {handle_inc_r, 4, 1, true};
    main_table[0x0D] = {handle_dec_r, 4, 1, true};
    main_table[0x0E] = {handle_ld_r_n, 7, 2, false};
    main_table[0x0F] = {handle_rrca, 4, 1, false};

    // 0x10-0x17: DJNZ, LD DE, INC/DEC DE
    main_table[0x10] = {handle_djnz_e, 13, 2, false};
    main_table[0x11] = {handle_ld_rr_nn, 10, 3, false};
    main_table[0x12] = {handle_ld_de_a, 7, 1, false};
    main_table[0x13] = {handle_inc_rr, 6, 1, false};
    main_table[0x14] = {handle_inc_r, 4, 1, true};
    main_table[0x15] = {handle_dec_r, 4, 1, true};
    main_table[0x16] = {handle_ld_r_n, 7, 2, false};
    main_table[0x17] = {handle_rla, 4, 1, false};

    // 0x18-0x1F: JR, LD HL, INC/DEC HL
    main_table[0x18] = {handle_jr_e, 12, 2, false};
    main_table[0x19] = {handle_add_hl_rr, 11, 1, false};
    main_table[0x1A] = {handle_ld_a_de, 7, 1, false};
    main_table[0x1B] = {handle_dec_rr, 6, 1, false};
    main_table[0x1C] = {handle_inc_r, 4, 1, true};
    main_table[0x1D] = {handle_dec_r, 4, 1, true};
    main_table[0x1E] = {handle_ld_r_n, 7, 2, false};
    main_table[0x1F] = {handle_rra, 4, 1, false};

    // 0x20-0x27: JR cc, ADD HL, LD (nn), LD (HL), INC (HL), DEC (HL), LD (HL) n, ADD A
    main_table[0x20] = {handle_jr_cc_e, 12, 2, false};
    main_table[0x21] = {handle_ld_rr_nn, 10, 3, false};
    main_table[0x22] = {handle_ld_nn_hl, 16, 3, false};
    main_table[0x23] = {handle_inc_rr, 6, 1, false};
    main_table[0x24] = {handle_inc_r, 4, 1, true};
    main_table[0x25] = {handle_dec_r, 4, 1, true};
    main_table[0x26] = {handle_ld_r_n, 7, 2, false};
    main_table[0x27] = {handle_add_a, 4, 1, true};

    // 0x28-0x2F
    main_table[0x28] = {handle_jr_cc_e, 12, 2, false};
    main_table[0x29] = {handle_add_hl_rr, 11, 1, false};
    main_table[0x2A] = {handle_ld_hl_nn, 16, 3, false};
    main_table[0x2B] = {handle_dec_rr, 6, 1, false};
    main_table[0x2C] = {handle_inc_r, 4, 1, true};
    main_table[0x2D] = {handle_dec_r, 4, 1, true};
    main_table[0x2E] = {handle_ld_r_n, 7, 2, false};
    main_table[0x2F] = {handle_sub, 4, 1, true};

    // 0x30-0x37
    main_table[0x30] = {handle_jr_cc_e, 12, 2, false};
    main_table[0x31] = {handle_ld_rr_nn, 10, 3, false};
    main_table[0x32] = {handle_ld_nn_a, 13, 3, false};
    main_table[0x33] = {handle_inc_rr, 6, 1, false};
    main_table[0x34] = {handle_inc_hl, 11, 1, true};
    main_table[0x35] = {handle_dec_hl, 11, 1, true};
    main_table[0x36] = {handle_ld_hl_n, 10, 2, false};
    main_table[0x37] = {handle_scf, 4, 1, false};

    // 0x38-0x3F
    main_table[0x38] = {handle_jr_cc_e, 12, 2, false};
    main_table[0x39] = {handle_add_hl_rr, 11, 1, false};
    main_table[0x3A] = {handle_ld_a_nn, 13, 3, false};
    main_table[0x3B] = {handle_dec_rr, 6, 1, false};
    main_table[0x3C] = {handle_inc_r, 4, 1, true};
    main_table[0x3D] = {handle_dec_r, 4, 1, true};
    main_table[0x3E] = {handle_ld_r_n, 7, 2, false};
    main_table[0x3F] = {handle_ccf, 4, 1, false};

    // 0x40-0x47: LD r,r
    main_table[0x40] = {handle_ld_r_r, 4, 1, false};
    main_table[0x41] = {handle_ld_r_r, 4, 1, false};
    main_table[0x42] = {handle_ld_r_r, 4, 1, false};
    main_table[0x43] = {handle_ld_r_r, 4, 1, false};
    main_table[0x44] = {handle_ld_r_r, 4, 1, false};
    main_table[0x45] = {handle_ld_r_r, 4, 1, false};
    main_table[0x46] = {handle_ld_r_hl, 7, 1, false};
    main_table[0x47] = {handle_ld_r_r, 4, 1, false};

    // 0x48-0x4F
    main_table[0x48] = {handle_ld_r_r, 4, 1, false};
    main_table[0x49] = {handle_ld_r_r, 4, 1, false};
    main_table[0x4A] = {handle_ld_r_r, 4, 1, false};
    main_table[0x4B] = {handle_ld_r_r, 4, 1, false};
    main_table[0x4C] = {handle_ld_r_r, 4, 1, false};
    main_table[0x4D] = {handle_ld_r_r, 4, 1, false};
    main_table[0x4E] = {handle_ld_r_hl, 7, 1, false};
    main_table[0x4F] = {handle_ld_r_r, 4, 1, false};

    // 0x50-0x57
    main_table[0x50] = {handle_ld_r_r, 4, 1, false};
    main_table[0x51] = {handle_ld_r_r, 4, 1, false};
    main_table[0x52] = {handle_ld_r_r, 4, 1, false};
    main_table[0x53] = {handle_ld_r_r, 4, 1, false};
    main_table[0x54] = {handle_ld_r_r, 4, 1, false};
    main_table[0x55] = {handle_ld_r_r, 4, 1, false};
    main_table[0x56] = {handle_ld_r_hl, 7, 1, false};
    main_table[0x57] = {handle_ld_r_r, 4, 1, false};

    // 0x58-0x5F
    main_table[0x58] = {handle_ld_r_r, 4, 1, false};
    main_table[0x59] = {handle_ld_r_r, 4, 1, false};
    main_table[0x5A] = {handle_ld_r_r, 4, 1, false};
    main_table[0x5B] = {handle_ld_r_r, 4, 1, false};
    main_table[0x5C] = {handle_ld_r_r, 4, 1, false};
    main_table[0x5D] = {handle_ld_r_r, 4, 1, false};
    main_table[0x5E] = {handle_ld_r_hl, 7, 1, false};
    main_table[0x5F] = {handle_ld_r_r, 4, 1, false};

    // 0x60-0x67
    main_table[0x60] = {handle_ld_r_r, 4, 1, false};
    main_table[0x61] = {handle_ld_r_r, 4, 1, false};
    main_table[0x62] = {handle_ld_r_r, 4, 1, false};
    main_table[0x63] = {handle_ld_r_r, 4, 1, false};
    main_table[0x64] = {handle_ld_r_r, 4, 1, false};
    main_table[0x65] = {handle_ld_r_r, 4, 1, false};
    main_table[0x66] = {handle_ld_r_hl, 7, 1, false};
    main_table[0x67] = {handle_ld_hl_r, 7, 1, false};

    // 0x68-0x6F
    main_table[0x68] = {handle_ld_r_r, 4, 1, false};
    main_table[0x69] = {handle_ld_r_r, 4, 1, false};
    main_table[0x6A] = {handle_ld_r_r, 4, 1, false};
    main_table[0x6B] = {handle_ld_r_r, 4, 1, false};
    main_table[0x6C] = {handle_ld_r_r, 4, 1, false};
    main_table[0x6D] = {handle_ld_r_r, 4, 1, false};
    main_table[0x6E] = {handle_ld_r_hl, 7, 1, false};
    main_table[0x6F] = {handle_ld_hl_r, 7, 1, false};

    // 0x70-0x77
    main_table[0x70] = {handle_ld_hl_r, 7, 1, false};
    main_table[0x71] = {handle_ld_hl_r, 7, 1, false};
    main_table[0x72] = {handle_ld_hl_r, 7, 1, false};
    main_table[0x73] = {handle_ld_hl_r, 7, 1, false};
    main_table[0x74] = {handle_ld_hl_r, 7, 1, false};
    main_table[0x75] = {handle_ld_hl_r, 7, 1, false};
    main_table[0x76] = {handle_halt, 4, 1, false};
    main_table[0x77] = {handle_ld_hl_r, 7, 1, false};

    // 0x78-0x7F
    main_table[0x78] = {handle_ld_r_r, 4, 1, false};
    main_table[0x79] = {handle_ld_r_r, 4, 1, false};
    main_table[0x7A] = {handle_ld_r_r, 4, 1, false};
    main_table[0x7B] = {handle_ld_r_r, 4, 1, false};
    main_table[0x7C] = {handle_ld_r_r, 4, 1, false};
    main_table[0x7D] = {handle_ld_r_r, 4, 1, false};
    main_table[0x7E] = {handle_ld_r_hl, 7, 1, false};
    main_table[0x7F] = {handle_ld_r_r, 4, 1, false};

    // 0x80-0x87: ADD
    main_table[0x80] = {handle_add_a, 4, 1, true};
    main_table[0x81] = {handle_add_a, 4, 1, true};
    main_table[0x82] = {handle_add_a, 4, 1, true};
    main_table[0x83] = {handle_add_a, 4, 1, true};
    main_table[0x84] = {handle_add_a, 4, 1, true};
    main_table[0x85] = {handle_add_a, 4, 1, true};
    main_table[0x86] = {handle_add_a_hl, 7, 1, true};
    main_table[0x87] = {handle_add_a, 4, 1, true};

    // 0x88-0x8F: ADC
    main_table[0x88] = {handle_adc_a, 4, 1, true};
    main_table[0x89] = {handle_adc_a, 4, 1, true};
    main_table[0x8A] = {handle_adc_a, 4, 1, true};
    main_table[0x8B] = {handle_adc_a, 4, 1, true};
    main_table[0x8C] = {handle_adc_a, 4, 1, true};
    main_table[0x8D] = {handle_adc_a, 4, 1, true};
    main_table[0x8E] = {handle_adc_a, 4, 1, true};
    main_table[0x8F] = {handle_adc_a, 4, 1, true};

    // 0x90-0x97: SUB
    main_table[0x90] = {handle_sub, 4, 1, true};
    main_table[0x91] = {handle_sub, 4, 1, true};
    main_table[0x92] = {handle_sub, 4, 1, true};
    main_table[0x93] = {handle_sub, 4, 1, true};
    main_table[0x94] = {handle_sub, 4, 1, true};
    main_table[0x95] = {handle_sub, 4, 1, true};
    main_table[0x96] = {handle_sub, 4, 1, true};
    main_table[0x97] = {handle_sub, 4, 1, true};

    // 0x98-0x9F: SBC
    main_table[0x98] = {handle_sbc_a, 4, 1, true};
    main_table[0x99] = {handle_sbc_a, 4, 1, true};
    main_table[0x9A] = {handle_sbc_a, 4, 1, true};
    main_table[0x9B] = {handle_sbc_a, 4, 1, true};
    main_table[0x9C] = {handle_sbc_a, 4, 1, true};
    main_table[0x9D] = {handle_sbc_a, 4, 1, true};
    main_table[0x9E] = {handle_sbc_a, 4, 1, true};
    main_table[0x9F] = {handle_sbc_a, 4, 1, true};

    // 0xA0-0xA7: AND
    main_table[0xA0] = {handle_and, 4, 1, true};
    main_table[0xA1] = {handle_and, 4, 1, true};
    main_table[0xA2] = {handle_and, 4, 1, true};
    main_table[0xA3] = {handle_and, 4, 1, true};
    main_table[0xA4] = {handle_and, 4, 1, true};
    main_table[0xA5] = {handle_and, 4, 1, true};
    main_table[0xA6] = {handle_and, 4, 1, true};
    main_table[0xA7] = {handle_and, 4, 1, true};

    // 0xA8-0xAF: XOR
    main_table[0xA8] = {handle_xor, 4, 1, true};
    main_table[0xA9] = {handle_xor, 4, 1, true};
    main_table[0xAA] = {handle_xor, 4, 1, true};
    main_table[0xAB] = {handle_xor, 4, 1, true};
    main_table[0xAC] = {handle_xor, 4, 1, true};
    main_table[0xAD] = {handle_xor, 4, 1, true};
    main_table[0xAE] = {handle_xor, 4, 1, true};
    main_table[0xAF] = {handle_xor, 4, 1, true};

    // 0xB0-0xB7: OR
    main_table[0xB0] = {handle_or, 4, 1, true};
    main_table[0xB1] = {handle_or, 4, 1, true};
    main_table[0xB2] = {handle_or, 4, 1, true};
    main_table[0xB3] = {handle_or, 4, 1, true};
    main_table[0xB4] = {handle_or, 4, 1, true};
    main_table[0xB5] = {handle_or, 4, 1, true};
    main_table[0xB6] = {handle_or, 4, 1, true};
    main_table[0xB7] = {handle_or, 4, 1, true};

    // 0xB8-0xBF: CP
    main_table[0xB8] = {handle_cp, 4, 1, true};
    main_table[0xB9] = {handle_cp, 4, 1, true};
    main_table[0xBA] = {handle_cp, 4, 1, true};
    main_table[0xBB] = {handle_cp, 4, 1, true};
    main_table[0xBC] = {handle_cp, 4, 1, true};
    main_table[0xBD] = {handle_cp, 4, 1, true};
    main_table[0xBE] = {handle_cp_n, 7, 2, true};
    main_table[0xBF] = {handle_cp, 4, 1, true};

    // 0xC0-0xC7: RET cc, POP
    main_table[0xC0] = {handle_ret_cc, 11, 1, false};
    main_table[0xC1] = {handle_pop_rr, 10, 1, false};
    main_table[0xC2] = {handle_jp_cc_nn, 10, 3, false};
    main_table[0xC3] = {handle_jp_nn, 10, 3, false};
    main_table[0xC4] = {handle_call_cc_nn, 17, 3, false};
    main_table[0xC5] = {handle_push_rr, 11, 1, false};
    main_table[0xC6] = {handle_add_a_n, 7, 2, true};
    main_table[0xC7] = {handle_rst, 11, 1, false};

    // 0xC8-0xCF
    main_table[0xC8] = {handle_ret_cc, 11, 1, false};
    main_table[0xC9] = {handle_ret, 10, 1, false};
    main_table[0xCA] = {handle_jp_cc_nn, 10, 3, false};
    main_table[0xCB] = {nullptr, 0, 0, false};  // CB prefix handled specially
    main_table[0xCC] = {handle_call_cc_nn, 17, 3, false};
    main_table[0xCD] = {handle_call_nn, 17, 3, false};
    main_table[0xCE] = {handle_adc_a, 7, 2, true};
    main_table[0xCF] = {handle_rst, 11, 1, false};

    // 0xD0-0xD7
    main_table[0xD0] = {handle_ret_cc, 11, 1, false};
    main_table[0xD1] = {handle_pop_rr, 10, 1, false};
    main_table[0xD2] = {handle_jp_cc_nn, 10, 3, false};
    main_table[0xD3] = {handle_out_n_a, 11, 2, false};
    main_table[0xD4] = {handle_call_cc_nn, 17, 3, false};
    main_table[0xD5] = {handle_push_rr, 11, 1, false};
    main_table[0xD6] = {handle_sub_n, 7, 2, true};
    main_table[0xD7] = {handle_rst, 11, 1, false};

    // 0xD8-0xDF
    main_table[0xD8] = {handle_ret_cc, 11, 1, false};
    main_table[0xD9] = {handle_exx, 4, 1, false};
    main_table[0xDA] = {handle_jp_cc_nn, 10, 3, false};
    main_table[0xDB] = {handle_in_a_n, 11, 2, false};
    main_table[0xDC] = {handle_call_cc_nn, 17, 3, false};
    main_table[0xDD] = {nullptr, 0, 0, false};  // DD prefix
    main_table[0xDE] = {handle_sbc_a, 7, 2, true};
    main_table[0xDF] = {handle_rst, 11, 1, false};

    // 0xE0-0xE7
    main_table[0xE0] = {handle_ret_cc, 11, 1, false};
    main_table[0xE1] = {handle_pop_rr, 10, 1, false};
    main_table[0xE2] = {handle_jp_cc_nn, 10, 3, false};
    main_table[0xE3] = {handle_ex_sp_hl, 19, 1, false};  // 19 T-states!
    main_table[0xE4] = {handle_call_cc_nn, 17, 3, false};
    main_table[0xE5] = {handle_push_rr, 11, 1, false};
    main_table[0xE6] = {handle_and, 7, 2, true};
    main_table[0xE7] = {handle_rst, 11, 1, false};

    // 0xE8-0xEF
    main_table[0xE8] = {handle_ret_cc, 11, 1, false};
    main_table[0xE9] = {handle_jp_hl, 4, 1, false};
    main_table[0xEA] = {handle_jp_cc_nn, 10, 3, false};
    main_table[0xEB] = {handle_ex_de_hl, 4, 1, false};
    main_table[0xEC] = {handle_call_cc_nn, 17, 3, false};
    main_table[0xED] = {nullptr, 0, 0, false};  // ED prefix
    main_table[0xEE] = {handle_xor, 7, 2, true};
    main_table[0xEF] = {handle_rst, 11, 1, false};

    // 0xF0-0xF7
    main_table[0xF0] = {handle_ret_cc, 11, 1, false};
    main_table[0xF1] = {handle_pop_rr, 10, 1, false};
    main_table[0xF2] = {handle_jp_cc_nn, 10, 3, false};
    main_table[0xF3] = {handle_di, 4, 1, false};
    main_table[0xF4] = {handle_call_cc_nn, 17, 3, false};
    main_table[0xF5] = {handle_push_rr, 11, 1, false};
    main_table[0xF6] = {handle_or, 7, 2, true};
    main_table[0xF7] = {handle_rst, 11, 1, false};

    // 0xF8-0xFF
    main_table[0xF8] = {handle_ret_cc, 11, 1, false};
    main_table[0xF9] = {handle_ld_sp_hl, 6, 1, false};
    main_table[0xFA] = {handle_jp_cc_nn, 10, 3, false};
    main_table[0xFB] = {handle_ei, 4, 1, false};
    main_table[0xFC] = {handle_call_cc_nn, 17, 3, false};
    main_table[0xFD] = {nullptr, 0, 0, false};  // FD prefix
    main_table[0xFE] = {handle_cp_n, 7, 2, true};
    main_table[0xFF] = {handle_rst, 11, 1, false};

    // =====================
    // CB prefix table (0xCB00-0xCBFF)
    // =====================
    for (int i = 0; i < 64; i++) {
        // RLC, RRC, RL, RR, SLA, SRA, SLL, SRL for each register
        cb_table[i] = {handle_rlc_r, 8, 2, true};
        cb_table[i + 64] = {handle_rrc_r, 8, 2, true};
        cb_table[i + 128] = {handle_rl_r, 8, 2, true};
    }
    
    // RLC r (0x00-0x07)
    cb_table[0x00] = {handle_rlc_r, 8, 2, true};
    cb_table[0x01] = {handle_rlc_r, 8, 2, true};
    cb_table[0x02] = {handle_rlc_r, 8, 2, true};
    cb_table[0x03] = {handle_rlc_r, 8, 2, true};
    cb_table[0x04] = {handle_rlc_r, 8, 2, true};
    cb_table[0x05] = {handle_rlc_r, 8, 2, true};
    cb_table[0x06] = {handle_rlc_r, 8, 2, true};  // (HL)
    cb_table[0x07] = {handle_rlc_r, 8, 2, true};

    // RRC r (0x08-0x0F)
    cb_table[0x08] = {handle_rrc_r, 8, 2, true};
    cb_table[0x09] = {handle_rrc_r, 8, 2, true};
    cb_table[0x0A] = {handle_rrc_r, 8, 2, true};
    cb_table[0x0B] = {handle_rrc_r, 8, 2, true};
    cb_table[0x0C] = {handle_rrc_r, 8, 2, true};
    cb_table[0x0D] = {handle_rrc_r, 8, 2, true};
    cb_table[0x0E] = {handle_rrc_r, 8, 2, true};
    cb_table[0x0F] = {handle_rrc_r, 8, 2, true};

    // RL r (0x10-0x17)
    cb_table[0x10] = {handle_rl_r, 8, 2, true};
    cb_table[0x11] = {handle_rl_r, 8, 2, true};
    cb_table[0x12] = {handle_rl_r, 8, 2, true};
    cb_table[0x13] = {handle_rl_r, 8, 2, true};
    cb_table[0x14] = {handle_rl_r, 8, 2, true};
    cb_table[0x15] = {handle_rl_r, 8, 2, true};
    cb_table[0x16] = {handle_rl_r, 8, 2, true};
    cb_table[0x17] = {handle_rl_r, 8, 2, true};

    // RR r (0x18-0x1F)
    cb_table[0x18] = {handle_rr_r, 8, 2, true};
    cb_table[0x19] = {handle_rr_r, 8, 2, true};
    cb_table[0x1A] = {handle_rr_r, 8, 2, true};
    cb_table[0x1B] = {handle_rr_r, 8, 2, true};
    cb_table[0x1C] = {handle_rr_r, 8, 2, true};
    cb_table[0x1D] = {handle_rr_r, 8, 2, true};
    cb_table[0x1E] = {handle_rr_r, 8, 2, true};
    cb_table[0x1F] = {handle_rr_r, 8, 2, true};

    // SLA r (0x20-0x27)
    cb_table[0x20] = {handle_sla_r, 8, 2, true};
    cb_table[0x21] = {handle_sla_r, 8, 2, true};
    cb_table[0x22] = {handle_sla_r, 8, 2, true};
    cb_table[0x23] = {handle_sla_r, 8, 2, true};
    cb_table[0x24] = {handle_sla_r, 8, 2, true};
    cb_table[0x25] = {handle_sla_r, 8, 2, true};
    cb_table[0x26] = {handle_sla_r, 8, 2, true};
    cb_table[0x27] = {handle_sla_r, 8, 2, true};

    // SRA r (0x28-0x2F)
    cb_table[0x28] = {handle_sra_r, 8, 2, true};
    cb_table[0x29] = {handle_sra_r, 8, 2, true};
    cb_table[0x2A] = {handle_sra_r, 8, 2, true};
    cb_table[0x2B] = {handle_sra_r, 8, 2, true};
    cb_table[0x2C] = {handle_sra_r, 8, 2, true};
    cb_table[0x2D] = {handle_sra_r, 8, 2, true};
    cb_table[0x2E] = {handle_sra_r, 8, 2, true};
    cb_table[0x2F] = {handle_sra_r, 8, 2, true};

    // SLL r (0x30-0x37) - Undocumented
    cb_table[0x30] = {handle_sll_r, 8, 2, true};
    cb_table[0x31] = {handle_sll_r, 8, 2, true};
    cb_table[0x32] = {handle_sll_r, 8, 2, true};
    cb_table[0x33] = {handle_sll_r, 8, 2, true};
    cb_table[0x34] = {handle_sll_r, 8, 2, true};
    cb_table[0x35] = {handle_sll_r, 8, 2, true};
    cb_table[0x36] = {handle_sll_r, 8, 2, true};
    cb_table[0x37] = {handle_sll_r, 8, 2, true};

    // SRL r (0x38-0x3F)
    cb_table[0x38] = {handle_srl_r, 8, 2, true};
    cb_table[0x39] = {handle_srl_r, 8, 2, true};
    cb_table[0x3A] = {handle_srl_r, 8, 2, true};
    cb_table[0x3B] = {handle_srl_r, 8, 2, true};
    cb_table[0x3C] = {handle_srl_r, 8, 2, true};
    cb_table[0x3D] = {handle_srl_r, 8, 2, true};
    cb_table[0x3E] = {handle_srl_r, 8, 2, true};
    cb_table[0x3F] = {handle_srl_r, 8, 2, true};

    // BIT b, r (0x40-0x7F)
    for (int i = 0; i < 64; i++) {
        cb_table[0x40 + i] = {handle_cb_bit, 8, 2, true};
    }

    // RES b, r (0x80-0xAF)
    for (int i = 0; i < 48; i++) {
        cb_table[0x80 + i] = {handle_cb_res, 8, 2, true};
    }

    // SET b, r (0xB0-0xFF)
    for (int i = 0; i < 80; i++) {
        cb_table[0xB0 + i] = {handle_cb_set, 8, 2, true};
    }

    // =====================
    // ED prefix table
    // =====================
    // Block transfers
    ed_table[0xA0] = {handle_ldi, 16, 2, true};
    ed_table[0xA1] = {handle_cpi, 16, 2, true};
    ed_table[0xA2] = {handle_outi, 12, 2, false};
    ed_table[0xA3] = {handle_otir, 12, 2, false};
    
    ed_table[0xA8] = {handle_ldd, 16, 2, true};
    ed_table[0xA9] = {handle_cpd, 16, 2, true};
    ed_table[0xAA] = {handle_outd, 12, 2, false};
    ed_table[0xAB] = {handle_otdr, 12, 2, false};

    // LDIR/CPIR/LDDR/CPDR
    ed_table[0xB0] = {handle_ldir, 21, 2, true};
    ed_table[0xB1] = {handle_cpir, 21, 2, true};
    ed_table[0xB2] = {handle_inir, 12, 2, false};
    ed_table[0xB3] = {handle_indr, 12, 2, false};
    
    ed_table[0xB8] = {handle_lddr, 21, 2, true};
    ed_table[0xB9] = {handle_cpdr, 21, 2, true};
    ed_table[0xBA] = {handle_ind, 12, 2, false};
    ed_table[0xBB] = {handle_indr, 12, 2, false};

    // IN/OUT with C
    ed_table[0x70] = {handle_in_r_c, 12, 2, true};  // IN (C)
    ed_table[0x71] = {handle_in_r_c, 12, 2, true};  // IN (C)
    ed_table[0x78] = {handle_in_r_c, 12, 2, true};
    ed_table[0x79] = {handle_in_r_c, 12, 2, true};
    ed_table[0x7A] = {handle_in_r_c, 12, 2, true};
    ed_table[0x7B] = {handle_in_r_c, 12, 2, true};
    ed_table[0x7C] = {handle_in_r_c, 12, 2, true};
    ed_table[0x7D] = {handle_in_r_c, 12, 2, true};
    ed_table[0x7E] = {handle_in_r_c, 12, 2, true};
    ed_table[0x7F] = {handle_in_r_c, 12, 2, true};
    
    ed_table[0xA4] = {handle_out_c_r, 12, 2, false};
    ed_table[0xA5] = {handle_out_c_r, 12, 2, false};
    ed_table[0xAC] = {handle_out_c_r, 12, 2, false};
    ed_table[0xAD] = {handle_out_c_r, 12, 2, false};
    ed_table[0xAE] = {handle_out_c_r, 12, 2, false};
    ed_table[0xAF] = {handle_out_c_r, 12, 2, false};

    // 16-bit loads
    ed_table[0x43] = {handle_ld_nn_rr, 20, 4, false};  // LD (nn), BC
    ed_table[0x53] = {handle_ld_nn_rr, 20, 4, false};  // LD (nn), DE
    ed_table[0x63] = {handle_ld_nn_rr, 20, 4, false};  // LD (nn), HL
    ed_table[0x73] = {handle_ld_nn_rr, 20, 4, false};  // LD (nn), SP
    
    ed_table[0x4B] = {handle_ld_rr_nn_ind, 20, 4, false};  // LD BC, (nn)
    ed_table[0x5B] = {handle_ld_rr_nn_ind, 20, 4, false};  // LD DE, (nn)
    ed_table[0x6B] = {handle_ld_rr_nn_ind, 20, 4, false};  // LD HL, (nn)
    ed_table[0x7B] = {handle_ld_rr_nn_ind, 20, 4, false};  // LD SP, (nn)

    // ADC/SBC HL
    ed_table[0x4A] = {handle_adc_hl_rr, 15, 2, true};
    ed_table[0x5A] = {handle_adc_hl_rr, 15, 2, true};
    ed_table[0x6A] = {handle_adc_hl_rr, 15, 2, true};
    ed_table[0x7A] = {handle_adc_hl_rr, 15, 2, true};
    
    ed_table[0x42] = {handle_sbc_hl_rr, 15, 2, true};
    ed_table[0x52] = {handle_sbc_hl_rr, 15, 2, true};
    ed_table[0x62] = {handle_sbc_hl_rr, 15, 2, true};
    ed_table[0x72] = {handle_sbc_hl_rr, 15, 2, true};

    // IM set
    ed_table[0x46] = {handle_im, 8, 2, false};  // IM 0
    ed_table[0x56] = {handle_im, 8, 2, false};  // IM 1
    ed_table[0x57] = {handle_ld_a_i, 9, 2, true};  // LD A, I
    ed_table[0x5E] = {handle_im, 8, 2, false};  // IM 2
    ed_table[0x67] = {handle_rrd, 18, 2, true};
    ed_table[0x6F] = {handle_rld, 18, 2, true};
    
    ed_table[0x47] = {handle_ld_i_a, 9, 2, false};  // LD I, A
    ed_table[0x4F] = {handle_ld_r_a, 9, 2, false};  // LD R, A

    // NEG
    ed_table[0x44] = {handle_neg, 8, 2, true};
    ed_table[0x4C] = {handle_neg, 8, 2, true};
    ed_table[0x54] = {handle_neg, 8, 2, true};
    ed_table[0x5C] = {handle_neg, 8, 2, true};
    ed_table[0x64] = {handle_neg, 8, 2, true};
    ed_table[0x6C] = {handle_neg, 8, 2, true};
    ed_table[0x74] = {handle_neg, 8, 2, true};
    ed_table[0x7C] = {handle_neg, 8, 2, true};

    // RETI/RETN
    ed_table[0x45] = {handle_retn, 14, 2, false};
    ed_table[0x4D] = {handle_reti, 14, 2, false};
    ed_table[0x55] = {handle_retn, 14, 2, false};
    ed_table[0x5D] = {handle_retn, 14, 2, false};
    ed_table[0x65] = {handle_retn, 14, 2, false};
    ed_table[0x6D] = {handle_retn, 14, 2, false};
    ed_table[0x75] = {handle_retn, 14, 2, false};
    ed_table[0x7D] = {handle_retn, 14, 2, false};

    // LD A, I / LD A, R
    ed_table[0x58] = {handle_ld_a_i, 9, 2, true};
    ed_table[0x78] = {handle_ld_a_r, 9, 2, true};

    // =====================
    // DD/FD prefix tables (IX/IY)
    // =====================
    // Most DD/FD opcodes fall through to base instructions
    // Only IX/IY specific ones are different
    
    // Initialize with fallthrough markers (will use main table)
    for (int i = 0; i < 256; i++) {
        dd_table[i] = {nullptr, 0, 0, false};
        fd_table[i] = {nullptr, 0, 0, false};
    }

    // IX/IY specific instructions
    dd_table[0x21] = {handle_dd_fd_ld_ix_nn, 14, 4, false};  // LD IX, nn
    fd_table[0x21] = {handle_dd_fd_ld_ix_nn, 14, 4, false};  // LD IY, nn
    
    dd_table[0x22] = {handle_dd_fd_ld_nn_ix, 20, 4, false};  // LD (nn), IX
    fd_table[0x22] = {handle_dd_fd_ld_nn_ix, 20, 4, false};  // LD (nn), IY
    
    dd_table[0x2A] = {handle_dd_fd_ld_ix_nn_ind, 20, 4, false};  // LD IX, (nn)
    fd_table[0x2A] = {handle_dd_fd_ld_ix_nn_ind, 20, 4, false};  // LD IY, (nn)
    
    dd_table[0x23] = {handle_dd_fd_inc_ix, 10, 2, false};  // INC IX
    fd_table[0x23] = {handle_dd_fd_inc_ix, 10, 2, false};  // INC IY
    
    dd_table[0x2B] = {handle_dd_fd_dec_ix, 10, 2, false};  // DEC IX
    fd_table[0x2B] = {handle_dd_fd_dec_ix, 10, 2, false};  // DEC IY
    
    dd_table[0x29] = {handle_dd_fd_add_ix_rr, 15, 2, false};  // ADD IX, IX
    fd_table[0x29] = {handle_dd_fd_add_ix_rr, 15, 2, false};  // ADD IY, IY
    
    dd_table[0xF9] = {handle_dd_fd_ld_sp_ix, 10, 2, false};  // LD SP, IX
    fd_table[0xF9] = {handle_dd_fd_ld_sp_ix, 10, 2, false};  // LD SP, IY
    
    dd_table[0xE5] = {handle_dd_fd_push_ix, 15, 2, false};  // PUSH IX
    fd_table[0xE5] = {handle_dd_fd_push_ix, 15, 2, false};  // PUSH IY
    
    dd_table[0xE1] = {handle_dd_fd_pop_ix, 14, 2, false};  // POP IX
    fd_table[0xE1] = {handle_dd_fd_pop_ix, 14, 2, false};  // POP IY
    
    dd_table[0xE3] = {handle_dd_fd_ex_sp_ix, 23, 2, false};  // EX (SP), IX
    fd_table[0xE3] = {handle_dd_fd_ex_sp_ix, 23, 2, false};  // EX (SP), IY
    
    dd_table[0xE9] = {handle_dd_fd_jp_ix, 8, 2, false};  // JP (IX)
    fd_table[0xE9] = {handle_dd_fd_jp_ix, 8, 2, false};  // JP (IY)

    // Indexed operations - LD r, (IX+d) and LD (IX+d), r
    // 0x46 = LD (IX+d), A is a common one
    dd_table[0x46] = {handle_dd_fd_ld_ixd_n, 19, 3, false};  // LD (IX+d), n
    fd_table[0x46] = {handle_dd_fd_ld_ixd_n, 19, 3, false};  // LD (IY+d), n
    
    dd_table[0x4E] = {handle_dd_fd_ld_r_ixd, 19, 3, false};  // LD r, (IX+d)
    fd_table[0x4E] = {handle_dd_fd_ld_r_ixd, 19, 3, false};  // LD r, (IY+d)
    
    // INC/DEC (IX+d)
    dd_table[0x34] = {handle_dd_fd_inc_ixd, 23, 3, true};  // INC (IX+d)
    fd_table[0x34] = {handle_dd_fd_inc_ixd, 23, 3, true};
    dd_table[0x35] = {handle_dd_fd_dec_ixd, 23, 3, true};  // DEC (IX+d)
    fd_table[0x35] = {handle_dd_fd_dec_ixd, 23, 3, true};

    // =====================
    // DDCB/FDCB prefix tables
    // =====================
    // 0x00-0x3F: rotate/shift
    // 0x40-0x7F: BIT
    // 0x80-0xAF: RES
    // 0xB0-0xFF: SET
    for (int i = 0; i < 64; i++) {
        ddcb_table[i] = {handle_ddcb_fdcb_rot, 23, 4, true};
        fdcb_table[i] = {handle_ddcb_fdcb_rot, 23, 4, true};
    }
    for (int i = 0; i < 64; i++) {
        ddcb_table[0x40 + i] = {handle_ddcb_fdcb_bit, 20, 4, true};
        fdcb_table[0x40 + i] = {handle_ddcb_fdcb_bit, 20, 4, true};
    }
    for (int i = 0; i < 48; i++) {
        ddcb_table[0x80 + i] = {handle_ddcb_fdcb_res, 23, 4, true};
        fdcb_table[0x80 + i] = {handle_ddcb_fdcb_res, 23, 4, true};
    }
    for (int i = 0; i < 80; i++) {
        ddcb_table[0xB0 + i] = {handle_ddcb_fdcb_set, 23, 4, true};
        fdcb_table[0xB0 + i] = {handle_ddcb_fdcb_set, 23, 4, true};
    }
}

} // namespace z80