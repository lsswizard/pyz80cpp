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

    // Initialize flag tables
    FlagTables::init();

    // =====================
    // Main opcode table (0x00-0xFF)
    // =====================
    
    // 0x00-0x07: NOP, LD BC/DE/HL/SP, INC/DEC
    main_table[0x00] = Instruction(handle_nop, 4, 1, false);
    main_table[0x01] = Instruction(handle_ld_rr_nn, 10, 3, false);
    main_table[0x02] = Instruction(handle_ld_bc_a, 7, 1, false);
    main_table[0x03] = Instruction(handle_inc_rr, 6, 1, false);
    main_table[0x04] = Instruction(handle_inc_r, 4, 1, true);
    main_table[0x05] = Instruction(handle_dec_r, 4, 1, true);
    main_table[0x06] = Instruction(handle_ld_r_n, 7, 2, false);
    main_table[0x07] = Instruction(handle_rlca, 4, 1, false);
    
    // 0x08-0x0F
    main_table[0x08] = Instruction(handle_ex_af_afp, 4, 1, false);
    main_table[0x09] = Instruction(handle_add_hl_rr, 11, 1, false);
    main_table[0x0A] = Instruction(handle_ld_a_bc, 7, 1, false);
    main_table[0x0B] = Instruction(handle_dec_rr, 6, 1, false);
    main_table[0x0C] = Instruction(handle_inc_r, 4, 1, true);
    main_table[0x0D] = Instruction(handle_dec_r, 4, 1, true);
    main_table[0x0E] = Instruction(handle_ld_r_n, 7, 2, false);
    main_table[0x0F] = Instruction(handle_rrca, 4, 1, false);

    // 0x10-0x17: DJNZ, LD DE, INC/DEC DE
    main_table[0x10] = Instruction(handle_djnz_e, 13, 2, false);
    main_table[0x11] = Instruction(handle_ld_rr_nn, 10, 3, false);
    main_table[0x12] = Instruction(handle_ld_de_a, 7, 1, false);
    main_table[0x13] = Instruction(handle_inc_rr, 6, 1, false);
    main_table[0x14] = Instruction(handle_inc_r, 4, 1, true);
    main_table[0x15] = Instruction(handle_dec_r, 4, 1, true);
    main_table[0x16] = Instruction(handle_ld_r_n, 7, 2, false);
    main_table[0x17] = Instruction(handle_rla, 4, 1, false);

    // 0x18-0x1F: JR, LD HL, INC/DEC HL
    main_table[0x18] = Instruction(handle_jr_e, 12, 2, false);
    main_table[0x19] = Instruction(handle_add_hl_rr, 11, 1, false);
    main_table[0x1A] = Instruction(handle_ld_a_de, 7, 1, false);
    main_table[0x1B] = Instruction(handle_dec_rr, 6, 1, false);
    main_table[0x1C] = Instruction(handle_inc_r, 4, 1, true);
    main_table[0x1D] = Instruction(handle_dec_r, 4, 1, true);
    main_table[0x1E] = Instruction(handle_ld_r_n, 7, 2, false);
    main_table[0x1F] = Instruction(handle_rra, 4, 1, false);

    // 0x20-0x27: JR cc, ADD HL, LD (nn), LD (HL), INC (HL), DEC (HL), LD (HL) n, ADD A
    main_table[0x20] = Instruction(handle_jr_cc_e, 12, 2, false);
    main_table[0x21] = Instruction(handle_ld_rr_nn, 10, 3, false);
    main_table[0x22] = Instruction(handle_ld_nn_hl, 16, 3, false);
    main_table[0x23] = Instruction(handle_inc_rr, 6, 1, false);
    main_table[0x24] = Instruction(handle_inc_r, 4, 1, true);
    main_table[0x25] = Instruction(handle_dec_r, 4, 1, true);
    main_table[0x26] = Instruction(handle_ld_r_n, 7, 2, false);
    main_table[0x27] = Instruction(handle_daa, 4, 1, true);

    // 0x28-0x2F
    main_table[0x28] = Instruction(handle_jr_cc_e, 12, 2, false);
    main_table[0x29] = Instruction(handle_add_hl_rr, 11, 1, false);
    main_table[0x2A] = Instruction(handle_ld_hl_nn, 16, 3, false);
    main_table[0x2B] = Instruction(handle_dec_rr, 6, 1, false);
    main_table[0x2C] = Instruction(handle_inc_r, 4, 1, true);
    main_table[0x2D] = Instruction(handle_dec_r, 4, 1, true);
    main_table[0x2E] = Instruction(handle_ld_r_n, 7, 2, false);
    main_table[0x2F] = Instruction(handle_cpl, 4, 1, false);

    // 0x30-0x37
    main_table[0x30] = Instruction(handle_jr_cc_e, 12, 2, false);
    main_table[0x31] = Instruction(handle_ld_rr_nn, 10, 3, false);
    main_table[0x32] = Instruction(handle_ld_nn_a, 13, 3, false);
    main_table[0x33] = Instruction(handle_inc_rr, 6, 1, false);
    main_table[0x34] = Instruction(handle_inc_hl, 11, 1, true);
    main_table[0x35] = Instruction(handle_dec_hl, 11, 1, true);
    main_table[0x36] = Instruction(handle_ld_hl_n, 10, 2, false);
    main_table[0x37] = Instruction(handle_scf, 4, 1, false);

    // 0x38-0x3F
    main_table[0x38] = Instruction(handle_jr_cc_e, 12, 2, false);
    main_table[0x39] = Instruction(handle_add_hl_rr, 11, 1, false);
    main_table[0x3A] = Instruction(handle_ld_a_nn, 13, 3, false);
    main_table[0x3B] = Instruction(handle_dec_rr, 6, 1, false);
    main_table[0x3C] = Instruction(handle_inc_r, 4, 1, true);
    main_table[0x3D] = Instruction(handle_dec_r, 4, 1, true);
    main_table[0x3E] = Instruction(handle_ld_r_n, 7, 2, false);
    main_table[0x3F] = Instruction(handle_ccf, 4, 1, false);

    // 0x40-0x47: LD r,r
    main_table[0x40] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x41] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x42] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x43] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x44] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x45] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x46] = Instruction(handle_ld_r_hl, 7, 1, false);
    main_table[0x47] = Instruction(handle_ld_r_r, 4, 1, false);

    // 0x48-0x4F
    main_table[0x48] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x49] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x4A] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x4B] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x4C] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x4D] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x4E] = Instruction(handle_ld_r_hl, 7, 1, false);
    main_table[0x4F] = Instruction(handle_ld_r_r, 4, 1, false);

    // 0x50-0x57
    main_table[0x50] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x51] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x52] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x53] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x54] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x55] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x56] = Instruction(handle_ld_r_hl, 7, 1, false);
    main_table[0x57] = Instruction(handle_ld_r_r, 4, 1, false);

    // 0x58-0x5F
    main_table[0x58] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x59] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x5A] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x5B] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x5C] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x5D] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x5E] = Instruction(handle_ld_r_hl, 7, 1, false);
    main_table[0x5F] = Instruction(handle_ld_r_r, 4, 1, false);

    // 0x60-0x67
    main_table[0x60] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x61] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x62] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x63] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x64] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x65] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x66] = Instruction(handle_ld_r_hl, 7, 1, false);
    main_table[0x67] = Instruction(handle_ld_r_r, 4, 1, false);  // LD H,A

    // 0x68-0x6F
    main_table[0x68] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x69] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x6A] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x6B] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x6C] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x6D] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x6E] = Instruction(handle_ld_r_hl, 7, 1, false);
    main_table[0x6F] = Instruction(handle_ld_r_r, 4, 1, false);  // LD L,A

    // 0x70-0x77
    main_table[0x70] = Instruction(handle_ld_hl_r, 7, 1, false);
    main_table[0x71] = Instruction(handle_ld_hl_r, 7, 1, false);
    main_table[0x72] = Instruction(handle_ld_hl_r, 7, 1, false);
    main_table[0x73] = Instruction(handle_ld_hl_r, 7, 1, false);
    main_table[0x74] = Instruction(handle_ld_hl_r, 7, 1, false);
    main_table[0x75] = Instruction(handle_ld_hl_r, 7, 1, false);
    main_table[0x76] = Instruction(handle_halt, 4, 1, false);
    main_table[0x77] = Instruction(handle_ld_hl_r, 7, 1, false);

    // 0x78-0x7F
    main_table[0x78] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x79] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x7A] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x7B] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x7C] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x7D] = Instruction(handle_ld_r_r, 4, 1, false);
    main_table[0x7E] = Instruction(handle_ld_r_hl, 7, 1, false);
    main_table[0x7F] = Instruction(handle_ld_r_r, 4, 1, false);

    // 0x80-0x87: ADD
    main_table[0x80] = Instruction(handle_add_a, 4, 1, true);
    main_table[0x81] = Instruction(handle_add_a, 4, 1, true);
    main_table[0x82] = Instruction(handle_add_a, 4, 1, true);
    main_table[0x83] = Instruction(handle_add_a, 4, 1, true);
    main_table[0x84] = Instruction(handle_add_a, 4, 1, true);
    main_table[0x85] = Instruction(handle_add_a, 4, 1, true);
    main_table[0x86] = Instruction(handle_add_a_hl, 7, 1, true);
    main_table[0x87] = Instruction(handle_add_a, 4, 1, true);

    // 0x88-0x8F: ADC
    main_table[0x88] = Instruction(handle_adc_a, 4, 1, true);
    main_table[0x89] = Instruction(handle_adc_a, 4, 1, true);
    main_table[0x8A] = Instruction(handle_adc_a, 4, 1, true);
    main_table[0x8B] = Instruction(handle_adc_a, 4, 1, true);
    main_table[0x8C] = Instruction(handle_adc_a, 4, 1, true);
    main_table[0x8D] = Instruction(handle_adc_a, 4, 1, true);
    main_table[0x8E] = Instruction(handle_adc_a, 4, 1, true);
    main_table[0x8F] = Instruction(handle_adc_a, 4, 1, true);

    // 0x90-0x97: SUB
    main_table[0x90] = Instruction(handle_sub, 4, 1, true);
    main_table[0x91] = Instruction(handle_sub, 4, 1, true);
    main_table[0x92] = Instruction(handle_sub, 4, 1, true);
    main_table[0x93] = Instruction(handle_sub, 4, 1, true);
    main_table[0x94] = Instruction(handle_sub, 4, 1, true);
    main_table[0x95] = Instruction(handle_sub, 4, 1, true);
    main_table[0x96] = Instruction(handle_sub, 4, 1, true);
    main_table[0x97] = Instruction(handle_sub, 4, 1, true);

    // 0x98-0x9F: SBC
    main_table[0x98] = Instruction(handle_sbc_a, 4, 1, true);
    main_table[0x99] = Instruction(handle_sbc_a, 4, 1, true);
    main_table[0x9A] = Instruction(handle_sbc_a, 4, 1, true);
    main_table[0x9B] = Instruction(handle_sbc_a, 4, 1, true);
    main_table[0x9C] = Instruction(handle_sbc_a, 4, 1, true);
    main_table[0x9D] = Instruction(handle_sbc_a, 4, 1, true);
    main_table[0x9E] = Instruction(handle_sbc_a, 4, 1, true);
    main_table[0x9F] = Instruction(handle_sbc_a, 4, 1, true);

    // 0xA0-0xA7: AND
    main_table[0xA0] = Instruction(handle_and, 4, 1, true);
    main_table[0xA1] = Instruction(handle_and, 4, 1, true);
    main_table[0xA2] = Instruction(handle_and, 4, 1, true);
    main_table[0xA3] = Instruction(handle_and, 4, 1, true);
    main_table[0xA4] = Instruction(handle_and, 4, 1, true);
    main_table[0xA5] = Instruction(handle_and, 4, 1, true);
    main_table[0xA6] = Instruction(handle_and, 4, 1, true);
    main_table[0xA7] = Instruction(handle_and, 4, 1, true);

    // 0xA8-0xAF: XOR
    main_table[0xA8] = Instruction(handle_xor, 4, 1, true);
    main_table[0xA9] = Instruction(handle_xor, 4, 1, true);
    main_table[0xAA] = Instruction(handle_xor, 4, 1, true);
    main_table[0xAB] = Instruction(handle_xor, 4, 1, true);
    main_table[0xAC] = Instruction(handle_xor, 4, 1, true);
    main_table[0xAD] = Instruction(handle_xor, 4, 1, true);
    main_table[0xAE] = Instruction(handle_xor, 4, 1, true);
    main_table[0xAF] = Instruction(handle_xor, 4, 1, true);

    // 0xB0-0xB7: OR
    main_table[0xB0] = Instruction(handle_or, 4, 1, true);
    main_table[0xB1] = Instruction(handle_or, 4, 1, true);
    main_table[0xB2] = Instruction(handle_or, 4, 1, true);
    main_table[0xB3] = Instruction(handle_or, 4, 1, true);
    main_table[0xB4] = Instruction(handle_or, 4, 1, true);
    main_table[0xB5] = Instruction(handle_or, 4, 1, true);
    main_table[0xB6] = Instruction(handle_or, 4, 1, true);
    main_table[0xB7] = Instruction(handle_or, 4, 1, true);

    // 0xB8-0xBF: CP
    main_table[0xB8] = Instruction(handle_cp, 4, 1, true);
    main_table[0xB9] = Instruction(handle_cp, 4, 1, true);
    main_table[0xBA] = Instruction(handle_cp, 4, 1, true);
    main_table[0xBB] = Instruction(handle_cp, 4, 1, true);
    main_table[0xBC] = Instruction(handle_cp, 4, 1, true);
    main_table[0xBD] = Instruction(handle_cp, 4, 1, true);
    main_table[0xBE] = Instruction(handle_cp, 7, 1, true);
    main_table[0xBF] = Instruction(handle_cp, 4, 1, true);

    // 0xC0-0xC7: RET cc, POP
    main_table[0xC0] = Instruction(handle_ret_cc, 11, 1, false);
    main_table[0xC1] = Instruction(handle_pop_rr, 10, 1, false);
    main_table[0xC2] = Instruction(handle_jp_cc_nn, 10, 3, false);
    main_table[0xC3] = Instruction(handle_jp_nn, 10, 3, false);
    main_table[0xC4] = Instruction(handle_call_cc_nn, 17, 3, false);
    main_table[0xC5] = Instruction(handle_push_rr, 11, 1, false);
    main_table[0xC6] = Instruction(handle_add_a_n, 7, 2, true);
    main_table[0xC7] = Instruction(handle_rst, 11, 1, false);

    // 0xC8-0xCF
    main_table[0xC8] = Instruction(handle_ret_cc, 11, 1, false);
    main_table[0xC9] = Instruction(handle_ret, 10, 1, false);
    main_table[0xCA] = Instruction(handle_jp_cc_nn, 10, 3, false);
    main_table[0xCB] = Instruction(nullptr, 0, 0, false);  // CB prefix handled specially
    main_table[0xCC] = Instruction(handle_call_cc_nn, 17, 3, false);
    main_table[0xCD] = Instruction(handle_call_nn, 17, 3, false);
    main_table[0xCE] = Instruction(handle_adc_a_n, 7, 2, true);
    main_table[0xCF] = Instruction(handle_rst, 11, 1, false);

    // 0xD0-0xD7
    main_table[0xD0] = Instruction(handle_ret_cc, 11, 1, false);
    main_table[0xD1] = Instruction(handle_pop_rr, 10, 1, false);
    main_table[0xD2] = Instruction(handle_jp_cc_nn, 10, 3, false);
    main_table[0xD3] = Instruction(handle_out_n_a, 11, 2, false);
    main_table[0xD4] = Instruction(handle_call_cc_nn, 17, 3, false);
    main_table[0xD5] = Instruction(handle_push_rr, 11, 1, false);
    main_table[0xD6] = Instruction(handle_sub_n, 7, 2, true);
    main_table[0xD7] = Instruction(handle_rst, 11, 1, false);

    // 0xD8-0xDF
    main_table[0xD8] = Instruction(handle_ret_cc, 11, 1, false);
    main_table[0xD9] = Instruction(handle_exx, 4, 1, false);
    main_table[0xDA] = Instruction(handle_jp_cc_nn, 10, 3, false);
    main_table[0xDB] = Instruction(handle_in_a_n, 11, 2, false);
    main_table[0xDC] = Instruction(handle_call_cc_nn, 17, 3, false);
    main_table[0xDD] = Instruction(nullptr, 0, 0, false);  // DD prefix
    main_table[0xDE] = Instruction(handle_sbc_a_n, 7, 2, true);
    main_table[0xDF] = Instruction(handle_rst, 11, 1, false);

    // 0xE0-0xE7
    main_table[0xE0] = Instruction(handle_ret_cc, 11, 1, false);
    main_table[0xE1] = Instruction(handle_pop_rr, 10, 1, false);
    main_table[0xE2] = Instruction(handle_jp_cc_nn, 10, 3, false);
    main_table[0xE3] = Instruction(handle_ex_sp_hl, 19, 1, false);  // 19 T-states!
    main_table[0xE4] = Instruction(handle_call_cc_nn, 17, 3, false);
    main_table[0xE5] = Instruction(handle_push_rr, 11, 1, false);
    main_table[0xE6] = Instruction(handle_and_n, 7, 2, true);
    main_table[0xE7] = Instruction(handle_rst, 11, 1, false);

    // 0xE8-0xEF
    main_table[0xE8] = Instruction(handle_ret_cc, 11, 1, false);
    main_table[0xE9] = Instruction(handle_jp_hl, 4, 1, false);
    main_table[0xEA] = Instruction(handle_jp_cc_nn, 10, 3, false);
    main_table[0xEB] = Instruction(handle_ex_de_hl, 4, 1, false);
    main_table[0xEC] = Instruction(handle_call_cc_nn, 17, 3, false);
    main_table[0xED] = Instruction(nullptr, 0, 0, false);  // ED prefix
    main_table[0xEE] = Instruction(handle_xor_n, 7, 2, true);
    main_table[0xEF] = Instruction(handle_rst, 11, 1, false);

    // 0xF0-0xF7
    main_table[0xF0] = Instruction(handle_ret_cc, 11, 1, false);
    main_table[0xF1] = Instruction(handle_pop_rr, 10, 1, false);
    main_table[0xF2] = Instruction(handle_jp_cc_nn, 10, 3, false);
    main_table[0xF3] = Instruction(handle_di, 4, 1, false);
    main_table[0xF4] = Instruction(handle_call_cc_nn, 17, 3, false);
    main_table[0xF5] = Instruction(handle_push_rr, 11, 1, false);
    main_table[0xF6] = Instruction(handle_or_n, 7, 2, true);
    main_table[0xF7] = Instruction(handle_rst, 11, 1, false);

    // 0xF8-0xFF
    main_table[0xF8] = Instruction(handle_ret_cc, 11, 1, false);
    main_table[0xF9] = Instruction(handle_ld_sp_hl, 6, 1, false);
    main_table[0xFA] = Instruction(handle_jp_cc_nn, 10, 3, false);
    main_table[0xFB] = Instruction(handle_ei, 4, 1, false);
    main_table[0xFC] = Instruction(handle_call_cc_nn, 17, 3, false);
    main_table[0xFD] = Instruction(nullptr, 0, 0, false);  // FD prefix
    main_table[0xFE] = Instruction(handle_cp_n, 7, 2, true);
    main_table[0xFF] = Instruction(handle_rst, 11, 1, false);

    // =====================
    // CB prefix table (0xCB00-0xCBFF)
    // =====================
    // RLC r (0x00-0x07)
    cb_table[0x00] = Instruction(handle_rlc_r, 8, 2, true);
    cb_table[0x01] = Instruction(handle_rlc_r, 8, 2, true);
    cb_table[0x02] = Instruction(handle_rlc_r, 8, 2, true);
    cb_table[0x03] = Instruction(handle_rlc_r, 8, 2, true);
    cb_table[0x04] = Instruction(handle_rlc_r, 8, 2, true);
    cb_table[0x05] = Instruction(handle_rlc_r, 8, 2, true);
    cb_table[0x06] = Instruction(handle_rlc_r, 15, 2, true);  // (HL) - 15 cycles
    cb_table[0x07] = Instruction(handle_rlc_r, 8, 2, true);

    // RRC r (0x08-0x0F)
    cb_table[0x08] = Instruction(handle_rrc_r, 8, 2, true);
    cb_table[0x09] = Instruction(handle_rrc_r, 8, 2, true);
    cb_table[0x0A] = Instruction(handle_rrc_r, 8, 2, true);
    cb_table[0x0B] = Instruction(handle_rrc_r, 8, 2, true);
    cb_table[0x0C] = Instruction(handle_rrc_r, 8, 2, true);
    cb_table[0x0D] = Instruction(handle_rrc_r, 8, 2, true);
    cb_table[0x0E] = Instruction(handle_rrc_r, 15, 2, true);  // (HL) - 15 cycles
    cb_table[0x0F] = Instruction(handle_rrc_r, 8, 2, true);

    // RL r (0x10-0x17)
    cb_table[0x10] = Instruction(handle_rl_r, 8, 2, true);
    cb_table[0x11] = Instruction(handle_rl_r, 8, 2, true);
    cb_table[0x12] = Instruction(handle_rl_r, 8, 2, true);
    cb_table[0x13] = Instruction(handle_rl_r, 8, 2, true);
    cb_table[0x14] = Instruction(handle_rl_r, 8, 2, true);
    cb_table[0x15] = Instruction(handle_rl_r, 8, 2, true);
    cb_table[0x16] = Instruction(handle_rl_r, 15, 2, true);  // (HL) - 15 cycles
    cb_table[0x17] = Instruction(handle_rl_r, 8, 2, true);

    // RR r (0x18-0x1F)
    cb_table[0x18] = Instruction(handle_rr_r, 8, 2, true);
    cb_table[0x19] = Instruction(handle_rr_r, 8, 2, true);
    cb_table[0x1A] = Instruction(handle_rr_r, 8, 2, true);
    cb_table[0x1B] = Instruction(handle_rr_r, 8, 2, true);
    cb_table[0x1C] = Instruction(handle_rr_r, 8, 2, true);
    cb_table[0x1D] = Instruction(handle_rr_r, 8, 2, true);
    cb_table[0x1E] = Instruction(handle_rr_r, 15, 2, true);  // (HL) - 15 cycles
    cb_table[0x1F] = Instruction(handle_rr_r, 8, 2, true);

    // SLA r (0x20-0x27)
    cb_table[0x20] = Instruction(handle_sla_r, 8, 2, true);
    cb_table[0x21] = Instruction(handle_sla_r, 8, 2, true);
    cb_table[0x22] = Instruction(handle_sla_r, 8, 2, true);
    cb_table[0x23] = Instruction(handle_sla_r, 8, 2, true);
    cb_table[0x24] = Instruction(handle_sla_r, 8, 2, true);
    cb_table[0x25] = Instruction(handle_sla_r, 8, 2, true);
    cb_table[0x26] = Instruction(handle_sla_r, 15, 2, true);  // (HL) - 15 cycles
    cb_table[0x27] = Instruction(handle_sla_r, 8, 2, true);

    // SRA r (0x28-0x2F)
    cb_table[0x28] = Instruction(handle_sra_r, 8, 2, true);
    cb_table[0x29] = Instruction(handle_sra_r, 8, 2, true);
    cb_table[0x2A] = Instruction(handle_sra_r, 8, 2, true);
    cb_table[0x2B] = Instruction(handle_sra_r, 8, 2, true);
    cb_table[0x2C] = Instruction(handle_sra_r, 8, 2, true);
    cb_table[0x2D] = Instruction(handle_sra_r, 8, 2, true);
    cb_table[0x2E] = Instruction(handle_sra_r, 15, 2, true);  // (HL) - 15 cycles
    cb_table[0x2F] = Instruction(handle_sra_r, 8, 2, true);

    // SLL r (0x30-0x37) - Undocumented
    cb_table[0x30] = Instruction(handle_sll_r, 8, 2, true);
    cb_table[0x31] = Instruction(handle_sll_r, 8, 2, true);
    cb_table[0x32] = Instruction(handle_sll_r, 8, 2, true);
    cb_table[0x33] = Instruction(handle_sll_r, 8, 2, true);
    cb_table[0x34] = Instruction(handle_sll_r, 8, 2, true);
    cb_table[0x35] = Instruction(handle_sll_r, 8, 2, true);
    cb_table[0x36] = Instruction(handle_sll_r, 15, 2, true);  // (HL) - 15 cycles
    cb_table[0x37] = Instruction(handle_sll_r, 8, 2, true);

    // SRL r (0x38-0x3F)
    cb_table[0x38] = Instruction(handle_srl_r, 8, 2, true);
    cb_table[0x39] = Instruction(handle_srl_r, 8, 2, true);
    cb_table[0x3A] = Instruction(handle_srl_r, 8, 2, true);
    cb_table[0x3B] = Instruction(handle_srl_r, 8, 2, true);
    cb_table[0x3C] = Instruction(handle_srl_r, 8, 2, true);
    cb_table[0x3D] = Instruction(handle_srl_r, 8, 2, true);
    cb_table[0x3E] = Instruction(handle_srl_r, 15, 2, true);  // (HL) - 15 cycles
    cb_table[0x3F] = Instruction(handle_srl_r, 8, 2, true);

// BIT b, r (0x40-0x7F)
    for (int i = 0; i < 64; i++) {
        cb_table[0x40 + i] = Instruction(handle_cb_bit, 8, 2, true);
    }
    // BIT b,(HL) - fix timing
    cb_table[0x46] = Instruction(handle_cb_bit, 12, 2, true);
    cb_table[0x4E] = Instruction(handle_cb_bit, 12, 2, true);
    cb_table[0x56] = Instruction(handle_cb_bit, 12, 2, true);
    cb_table[0x5E] = Instruction(handle_cb_bit, 12, 2, true);
    cb_table[0x66] = Instruction(handle_cb_bit, 12, 2, true);
    cb_table[0x6E] = Instruction(handle_cb_bit, 12, 2, true);

    // RES b, r (0x80-0xBF) - full range
    for (int i = 0; i < 64; i++) {
        cb_table[0x80 + i] = Instruction(handle_cb_res, 8, 2, true);
    }
    // RES b,(HL) - fix timing
    cb_table[0x86] = Instruction(handle_cb_res, 15, 2, true);
    cb_table[0x8E] = Instruction(handle_cb_res, 15, 2, true);
    cb_table[0x96] = Instruction(handle_cb_res, 15, 2, true);
    cb_table[0x9E] = Instruction(handle_cb_res, 15, 2, true);
    cb_table[0xA6] = Instruction(handle_cb_res, 15, 2, true);
    cb_table[0xAE] = Instruction(handle_cb_res, 15, 2, true);
    cb_table[0xB6] = Instruction(handle_cb_res, 15, 2, true);
    cb_table[0xBE] = Instruction(handle_cb_res, 15, 2, true);

    // SET b, r (0xC0-0xFF) - fixed range
    for (int i = 0; i < 64; i++) {
        cb_table[0xC0 + i] = Instruction(handle_cb_set, 8, 2, true);
    }
    // SET b,(HL) - fix timing
    cb_table[0xC6] = Instruction(handle_cb_set, 15, 2, true);
    cb_table[0xCE] = Instruction(handle_cb_set, 15, 2, true);
    cb_table[0xD6] = Instruction(handle_cb_set, 15, 2, true);
    cb_table[0xDE] = Instruction(handle_cb_set, 15, 2, true);
    cb_table[0xE6] = Instruction(handle_cb_set, 15, 2, true);
    cb_table[0xEE] = Instruction(handle_cb_set, 15, 2, true);
    cb_table[0xF6] = Instruction(handle_cb_set, 15, 2, true);
    cb_table[0xFE] = Instruction(handle_cb_set, 15, 2, true);

    // =====================
    // ED prefix table
    // =====================
    // Block transfers
    ed_table[0xA0] = Instruction(handle_ldi, 16, 2, true);
    ed_table[0xA1] = Instruction(handle_cpi, 16, 2, true);
    ed_table[0xA2] = Instruction(handle_ini, 16, 2, true);
    ed_table[0xA3] = Instruction(handle_outi, 16, 2, true);
    
    ed_table[0xA8] = Instruction(handle_ldd, 16, 2, true);
    ed_table[0xA9] = Instruction(handle_cpd, 16, 2, true);
    ed_table[0xAA] = Instruction(handle_ind, 16, 2, true);
    ed_table[0xAB] = Instruction(handle_outd, 16, 2, true);

    // LDIR/CPIR/LDDR/CPDR
    ed_table[0xB0] = Instruction(handle_ldir, 21, 2, true);
    ed_table[0xB1] = Instruction(handle_cpir, 21, 2, true);
    ed_table[0xB2] = Instruction(handle_inir, 16, 2, true);
    ed_table[0xB3] = Instruction(handle_otir, 16, 2, true);
    
    ed_table[0xB8] = Instruction(handle_lddr, 21, 2, true);
    ed_table[0xB9] = Instruction(handle_cpdr, 21, 2, true);
    ed_table[0xBA] = Instruction(handle_indr, 16, 2, true);
    ed_table[0xBB] = Instruction(handle_otdr, 16, 2, true);

    // IN r,(C) - 0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78 (B,C,D,E,H,L,F,A)
    for (int r = 0; r < 8; r++) {
        ed_table[0x40 + (r << 3)] = Instruction(handle_in_r_c, 12, 2, true);
    }
    // OUT (C),r - 0x41,0x49,0x51,0x59,0x61,0x69,0x71,0x79
    for (int r = 0; r < 8; r++) {
        ed_table[0x41 + (r << 3)] = Instruction(handle_out_c_r, 12, 2, false);
    }

    // 16-bit loads
    ed_table[0x43] = Instruction(handle_ld_nn_rr_ind, 20, 4, false);  // LD (nn), BC
    ed_table[0x53] = Instruction(handle_ld_nn_rr_ind, 20, 4, false);  // LD (nn), DE
    ed_table[0x63] = Instruction(handle_ld_nn_rr_ind, 20, 4, false);  // LD (nn), HL
    ed_table[0x73] = Instruction(handle_ld_nn_rr_ind, 20, 4, false);  // LD (nn), SP
    
    ed_table[0x4B] = Instruction(handle_ld_rr_nn_ind, 20, 4, false);  // LD BC, (nn)
    ed_table[0x5B] = Instruction(handle_ld_rr_nn_ind, 20, 4, false);  // LD DE, (nn)
    ed_table[0x6B] = Instruction(handle_ld_rr_nn_ind, 20, 4, false);  // LD HL, (nn)
    ed_table[0x7B] = Instruction(handle_ld_rr_nn_ind, 20, 4, false);  // LD SP, (nn)

    // ADC/SBC HL
    ed_table[0x4A] = Instruction(handle_adc_hl_rr, 15, 2, true);
    ed_table[0x5A] = Instruction(handle_adc_hl_rr, 15, 2, true);
    ed_table[0x6A] = Instruction(handle_adc_hl_rr, 15, 2, true);
    ed_table[0x7A] = Instruction(handle_adc_hl_rr, 15, 2, true);
    
    ed_table[0x42] = Instruction(handle_sbc_hl_rr, 15, 2, true);
    ed_table[0x52] = Instruction(handle_sbc_hl_rr, 15, 2, true);
    ed_table[0x62] = Instruction(handle_sbc_hl_rr, 15, 2, true);
    ed_table[0x72] = Instruction(handle_sbc_hl_rr, 15, 2, true);

    // IM set
    ed_table[0x46] = Instruction(handle_im, 8, 2, false);  // IM 0
    ed_table[0x4E] = Instruction(handle_im, 8, 2, false);  // IM 0 (undocumented)
    ed_table[0x56] = Instruction(handle_im, 8, 2, false);  // IM 1
    ed_table[0x57] = Instruction(handle_ld_a_i, 9, 2, true);  // LD A, I
    ed_table[0x5E] = Instruction(handle_im, 8, 2, false);  // IM 2
    ed_table[0x66] = Instruction(handle_im, 8, 2, false);  // IM 0 (undocumented)
    ed_table[0x6E] = Instruction(handle_im, 8, 2, false);  // IM 0 (undocumented)
    ed_table[0x76] = Instruction(handle_im, 8, 2, false);  // IM 1 (undocumented)
    ed_table[0x7E] = Instruction(handle_im, 8, 2, false);  // IM 2 (undocumented)
    ed_table[0x67] = Instruction(handle_rrd, 18, 2, true);
    ed_table[0x6F] = Instruction(handle_rld, 18, 2, true);
    
    ed_table[0x47] = Instruction(handle_ld_i_a, 9, 2, false);  // LD I, A
    ed_table[0x4F] = Instruction(handle_ld_r_a, 9, 2, false);  // LD R, A

    // NEG
    ed_table[0x44] = Instruction(handle_neg, 8, 2, true);
    ed_table[0x4C] = Instruction(handle_neg, 8, 2, true);
    ed_table[0x54] = Instruction(handle_neg, 8, 2, true);
    ed_table[0x5C] = Instruction(handle_neg, 8, 2, true);
    ed_table[0x64] = Instruction(handle_neg, 8, 2, true);
    ed_table[0x6C] = Instruction(handle_neg, 8, 2, true);
    ed_table[0x74] = Instruction(handle_neg, 8, 2, true);
    ed_table[0x7C] = Instruction(handle_neg, 8, 2, true);

    // RETI/RETN
    ed_table[0x45] = Instruction(handle_retn, 14, 2, false);
    ed_table[0x4D] = Instruction(handle_reti, 14, 2, false);
    ed_table[0x55] = Instruction(handle_retn, 14, 2, false);
    ed_table[0x5D] = Instruction(handle_retn, 14, 2, false);
    ed_table[0x65] = Instruction(handle_retn, 14, 2, false);
    ed_table[0x6D] = Instruction(handle_retn, 14, 2, false);
    ed_table[0x75] = Instruction(handle_retn, 14, 2, false);
    ed_table[0x7D] = Instruction(handle_retn, 14, 2, false);

    // LD A, I / LD A, R
    ed_table[0x5F] = Instruction(handle_ld_a_r, 9, 2, true);

    // =====================
    // DD/FD prefix tables (IX/IY)
    // =====================
    // Most DD/FD opcodes fall through to base instructions
    // Only IX/IY specific ones are different
    
    // Initialize with fallthrough markers (will use main table)
    for (int i = 0; i < 256; i++) {
        dd_table[i] = Instruction(nullptr, 0, 0, false);
        fd_table[i] = Instruction(nullptr, 0, 0, false);
    }

    // IX/IY specific instructions
    dd_table[0x21] = Instruction(handle_dd_fd_ld_ix_nn, 14, 4, false);  // LD IX, nn
    fd_table[0x21] = Instruction(handle_dd_fd_ld_ix_nn, 14, 4, false);  // LD IY, nn
    
    dd_table[0x22] = Instruction(handle_dd_fd_ld_nn_ix, 20, 4, false);  // LD (nn), IX
    fd_table[0x22] = Instruction(handle_dd_fd_ld_nn_ix, 20, 4, false);  // LD (nn), IY
    
    dd_table[0x2A] = Instruction(handle_dd_fd_ld_ix_nn_ind, 20, 4, false);  // LD IX, (nn)
    fd_table[0x2A] = Instruction(handle_dd_fd_ld_ix_nn_ind, 20, 4, false);  // LD IY, (nn)
    
    dd_table[0x23] = Instruction(handle_dd_fd_inc_ix, 10, 2, false);  // INC IX
    fd_table[0x23] = Instruction(handle_dd_fd_inc_ix, 10, 2, false);  // INC IY
    
    dd_table[0x2B] = Instruction(handle_dd_fd_dec_ix, 10, 2, false);  // DEC IX
    fd_table[0x2B] = Instruction(handle_dd_fd_dec_ix, 10, 2, false);  // DEC IY
    
    // INC/DEC IXH and IXL (DD prefix) - unified handler
    dd_table[0x24] = Instruction(handle_dd_fd_inc_dec_ixhl, 8, 2, true);  // INC IXH
    dd_table[0x25] = Instruction(handle_dd_fd_inc_dec_ixhl, 8, 2, true);   // DEC IXH
    dd_table[0x2C] = Instruction(handle_dd_fd_inc_dec_ixhl, 8, 2, true);   // INC IXL
    dd_table[0x2D] = Instruction(handle_dd_fd_inc_dec_ixhl, 8, 2, true);   // DEC IXL

    // Same for FD prefix
    fd_table[0x24] = Instruction(handle_dd_fd_inc_dec_ixhl, 8, 2, true);  // INC IYH
    fd_table[0x25] = Instruction(handle_dd_fd_inc_dec_ixhl, 8, 2, true);   // DEC IYH
    fd_table[0x2C] = Instruction(handle_dd_fd_inc_dec_ixhl, 8, 2, true);   // INC IYL
    fd_table[0x2D] = Instruction(handle_dd_fd_inc_dec_ixhl, 8, 2, true);   // DEC IYL
    
    dd_table[0x09] = Instruction(handle_dd_fd_add_ix_rr, 15, 2, false);  // ADD IX, BC
    fd_table[0x09] = Instruction(handle_dd_fd_add_ix_rr, 15, 2, false);  // ADD IY, BC
    dd_table[0x19] = Instruction(handle_dd_fd_add_ix_rr, 15, 2, false);  // ADD IX, DE
    fd_table[0x19] = Instruction(handle_dd_fd_add_ix_rr, 15, 2, false);  // ADD IY, DE
    dd_table[0x29] = Instruction(handle_dd_fd_add_ix_rr, 15, 2, false);  // ADD IX, IX
    fd_table[0x29] = Instruction(handle_dd_fd_add_ix_rr, 15, 2, false);  // ADD IY, IY
    dd_table[0x39] = Instruction(handle_dd_fd_add_ix_rr, 15, 2, false);  // ADD IX, SP
    fd_table[0x39] = Instruction(handle_dd_fd_add_ix_rr, 15, 2, false);  // ADD IY, SP
    
    dd_table[0xF9] = Instruction(handle_dd_fd_ld_sp_ix, 10, 2, false);  // LD SP, IX
    fd_table[0xF9] = Instruction(handle_dd_fd_ld_sp_ix, 10, 2, false);  // LD SP, IY
    
    dd_table[0xE5] = Instruction(handle_dd_fd_push_ix, 15, 2, false);  // PUSH IX
    fd_table[0xE5] = Instruction(handle_dd_fd_push_ix, 15, 2, false);  // PUSH IY
    
    dd_table[0xE1] = Instruction(handle_dd_fd_pop_ix, 14, 2, false);  // POP IX
    fd_table[0xE1] = Instruction(handle_dd_fd_pop_ix, 14, 2, false);  // POP IY
    
    dd_table[0xE3] = Instruction(handle_dd_fd_ex_sp_ix, 23, 2, false);  // EX (SP), IX
    fd_table[0xE3] = Instruction(handle_dd_fd_ex_sp_ix, 23, 2, false);  // EX (SP), IY
    
    dd_table[0xE9] = Instruction(handle_dd_fd_jp_ix, 8, 2, false);  // JP (IX)
    fd_table[0xE9] = Instruction(handle_dd_fd_jp_ix, 8, 2, false);  // JP (IY)

    // Indexed operations - LD r, (IX+d) and LD (IX+d), r
    // 0x46 = LD (IX+d), A is a common one
    // LD r,(IX+d) for all r != 6: opcodes 0x46,0x4E,0x56,0x5E,0x66,0x6E,0x7E
    for (int r = 0; r < 8; r++) {
        if (r != 6) {
            dd_table[0x46 + (r << 3)] = Instruction(handle_dd_fd_ld_r_ixd, 23, 3, false);
            fd_table[0x46 + (r << 3)] = Instruction(handle_dd_fd_ld_r_ixd, 23, 3, false);
            }
            }
            // LD (IX+d),r for all r != 6: opcodes 0x70..0x77 except 0x76 (HALT)
            for (int r = 0; r < 8; r++) {
            if (r != 6) {
            dd_table[0x70 + r] = Instruction(handle_dd_fd_ld_ixd_r, 23, 3, false);
            fd_table[0x70 + r] = Instruction(handle_dd_fd_ld_ixd_r, 23, 3, false);
            }
            }
            // LD (IX+d),n
            dd_table[0x36] = Instruction(handle_dd_fd_ld_ixd_n, 23, 3, false);
            fd_table[0x36] = Instruction(handle_dd_fd_ld_ixd_n, 23, 3, false);
    
    // INC/DEC (IX+d)
    dd_table[0x34] = Instruction(handle_dd_fd_inc_ixd, 23, 3, true);  // INC (IX+d)
    fd_table[0x34] = Instruction(handle_dd_fd_inc_ixd, 23, 3, true);
    dd_table[0x35] = Instruction(handle_dd_fd_dec_ixd, 23, 3, true);  // DEC (IX+d)
    fd_table[0x35] = Instruction(handle_dd_fd_dec_ixd, 23, 3, true);

    // ADD A, IXH/IXL (0x84/0x85) and similar arithmetic on IXH/IXL
    // These take precedence over indexed (IX+d) versions
    dd_table[0x84] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);   // ADD A, IXH
    dd_table[0x85] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);   // ADD A, IXL
    dd_table[0x8C] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);   // ADC A, IXH
    dd_table[0x8D] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);   // ADC A, IXL

    dd_table[0x94] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);    // SUB A, IXH
    dd_table[0x95] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);    // SUB A, IXL
    dd_table[0x9C] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);    // SBC A, IXH
    dd_table[0x9D] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);    // SBC A, IXL

    dd_table[0xA4] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);   // AND A, IXH
    dd_table[0xA5] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);   // AND A, IXL

    dd_table[0xB4] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);    // OR A, IXH
    dd_table[0xB5] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);    // OR A, IXL

    dd_table[0xAC] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);    // XOR A, IXH
    dd_table[0xAD] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);    // XOR A, IXL

    dd_table[0xBC] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);     // CP A, IXH
    dd_table[0xBD] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);     // CP A, IXL
    
    // ADD/SUB/AND/OR/XOR/CP A,(IX+d) - 0x86,0x8E,0x96,0x9E,0xA6,0xAE,0xB6,0xBE with displacement
    // ADD A,(IX+d): DD 86 dd
    dd_table[0x86] = Instruction(handle_dd_fd_add_a_ixd, 19, 3, true);  // ADD A,(IX+d)
    dd_table[0x8E] = Instruction(handle_dd_fd_add_a_ixd, 19, 3, true);  // ADC A,(IX+d)
    dd_table[0x96] = Instruction(handle_dd_fd_sub_ixd, 19, 3, true);    // SUB A,(IX+d)
    dd_table[0x9E] = Instruction(handle_dd_fd_sub_ixd, 19, 3, true);    // SBC A,(IX+d)
    dd_table[0xA6] = Instruction(handle_dd_fd_and_ixd, 19, 3, false);    // AND A,(IX+d)
    dd_table[0xAE] = Instruction(handle_dd_fd_xor_ixd, 19, 3, false);   // XOR A,(IX+d)
    dd_table[0xB6] = Instruction(handle_dd_fd_or_ixd, 19, 3, false);     // OR A,(IX+d)
    dd_table[0xBE] = Instruction(handle_dd_fd_cp_ixd, 19, 3, true);      // CP A,(IX+d)
    
    // Same for FD prefix (IYH/IYL)
    fd_table[0x84] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);   // ADD A, IYH
    fd_table[0x85] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);   // ADD A, IYL
    fd_table[0x8C] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);   // ADC A, IYH
    fd_table[0x8D] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);   // ADC A, IYL
    fd_table[0x94] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);
    fd_table[0x95] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);
    fd_table[0x9C] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);
    fd_table[0x9D] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);
    fd_table[0xA4] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);
    fd_table[0xA5] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);
    fd_table[0xB4] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);
    fd_table[0xB5] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);
    fd_table[0xAC] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);
    fd_table[0xAD] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);
    fd_table[0xBC] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);
    fd_table[0xBD] = Instruction(handle_dd_fd_alu_ixhl, 8, 2, true);
    
    // ADD/SUB/AND/OR/XOR/CP A,(IY+d) - same as DD but for IY
    fd_table[0x86] = Instruction(handle_dd_fd_add_a_ixd, 19, 3, true);  // ADD A,(IY+d)
    fd_table[0x8E] = Instruction(handle_dd_fd_add_a_ixd, 19, 3, true);  // ADC A,(IY+d)
    fd_table[0x96] = Instruction(handle_dd_fd_sub_ixd, 19, 3, true);    // SUB A,(IY+d)
    fd_table[0x9E] = Instruction(handle_dd_fd_sub_ixd, 19, 3, true);    // SBC A,(IY+d)
    fd_table[0xA6] = Instruction(handle_dd_fd_and_ixd, 19, 3, false);   // AND A,(IY+d)
    fd_table[0xAE] = Instruction(handle_dd_fd_xor_ixd, 19, 3, false);   // XOR A,(IY+d)
    fd_table[0xB6] = Instruction(handle_dd_fd_or_ixd, 19, 3, false);    // OR A,(IY+d)
    fd_table[0xBE] = Instruction(handle_dd_fd_cp_ixd, 19, 3, true);      // CP A,(IY+d)
    
// LD r, IXH (0x44=LD B,IXH, 0x4C=LD C,IXH, 0x54=LD D,IXH, 0x5C=LD E,IXH, 0x64=LD H,IXH, 0x6C=LD L,IXH)
    // These are in the pattern 0x40 + r*8, which normally is LD (IX+d),r for indexed
    // but for DD prefix with r=IXH/L it means LD r,IXH/IXL
    dd_table[0x44] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD B, IXH
    dd_table[0x4C] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD C, IXH
    dd_table[0x54] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD D, IXH
    dd_table[0x5C] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD E, IXH
    dd_table[0x64] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD H, IXH
    dd_table[0x6C] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD L, IXH
    
    dd_table[0x45] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD B, IXL
    dd_table[0x4D] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD C, IXL
    dd_table[0x55] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD D, IXL
    dd_table[0x5D] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD E, IXL
    dd_table[0x65] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD H, IXL
    dd_table[0x6D] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD L, IXL
    dd_table[0x7D] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD A, IXL
    dd_table[0x7F] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD IXL, A
    dd_table[0x7C] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD A, IXH
    
    fd_table[0x44] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD B, IYH
    fd_table[0x4C] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x54] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x5C] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x64] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x6C] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x45] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x4D] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x55] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x5D] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x65] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x6D] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x7C] = Instruction(handle_ld_ixhl_r, 8, 2, false);  // LD A, IYH

    // =====================
    // DD/FD IXH/IXL register loads (0x26, 0x2E, 0x60-0x67)
    // =====================
    // LD IXH, n (0x26) and LD IXL, n (0x2E)
    dd_table[0x26] = Instruction(handle_ld_ixhl_n, 11, 3, false);
    fd_table[0x26] = Instruction(handle_ld_ixhl_n, 11, 3, false);
    dd_table[0x2E] = Instruction(handle_ld_ixhl_n, 11, 3, false);
    fd_table[0x2E] = Instruction(handle_ld_ixhl_n, 11, 3, false);
    
    // LD IXH, r (0x60-0x67) - 0x60=IXH,B, 0x61=IXH,C, ..., 0x67=IXH,A
    dd_table[0x60] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    dd_table[0x61] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    dd_table[0x62] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    dd_table[0x63] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    dd_table[0x67] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x60] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x61] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x62] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x63] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x67] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    
    // LD IXL, r (0x68-0x6F)
    dd_table[0x68] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    dd_table[0x69] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    dd_table[0x6A] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    dd_table[0x6B] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    dd_table[0x6F] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x68] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x69] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x6A] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x6B] = Instruction(handle_ld_ixhl_r, 8, 2, false);
    fd_table[0x6F] = Instruction(handle_ld_ixhl_r, 8, 2, false);

    // =====================
    // DDCB/FDCB prefix tables
    // =====================
    // 0x00-0x3F: rotate/shift
    // 0x40-0x7F: BIT
    // 0x80-0xAF: RES
    // 0xB0-0xFF: SET
    for (int i = 0; i < 64; i++) {
        ddcb_table[i] = Instruction(handle_ddcb_fdcb_rot, 23, 4, true);
        fdcb_table[i] = Instruction(handle_ddcb_fdcb_rot, 23, 4, true);
    }
    for (int i = 0; i < 64; i++) {
        ddcb_table[0x40 + i] = Instruction(handle_ddcb_fdcb_bit, 20, 4, true);
        fdcb_table[0x40 + i] = Instruction(handle_ddcb_fdcb_bit, 20, 4, true);
    }
    for (int i = 0; i < 48; i++) {
        ddcb_table[0x80 + i] = Instruction(handle_ddcb_fdcb_res, 23, 4, true);
        fdcb_table[0x80 + i] = Instruction(handle_ddcb_fdcb_res, 23, 4, true);
    }
    for (int i = 0; i < 80; i++) {
        ddcb_table[0xB0 + i] = Instruction(handle_ddcb_fdcb_set, 23, 4, true);
        fdcb_table[0xB0 + i] = Instruction(handle_ddcb_fdcb_set, 23, 4, true);
    }
}

} // namespace z80