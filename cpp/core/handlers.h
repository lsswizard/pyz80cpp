#pragma once

#include <cstdint>

class CPU;

typedef void (*OpHandler)(CPU& cpu);

struct OpcodeEntry {
    OpHandler handler;
    uint8_t cycles;
    uint8_t length;
    bool affects_f;
    bool is_ld_a_ir;
};

struct DecodeSlot {
    OpHandler handler;
    uint8_t cycles;
    uint8_t length;
    bool affects_f;
    bool is_ld_a_ir;
};

// Handler tables (extern, built in decoder.cpp)
extern OpcodeEntry base_handlers[256];
extern OpcodeEntry cb_handlers[256];
extern OpcodeEntry ed_handlers[256];
extern OpcodeEntry dd_handlers[256];
extern OpcodeEntry fd_handlers[256];
extern OpcodeEntry ddcb_handlers[256];
extern OpcodeEntry fdcb_handlers[256];
extern OpcodeEntry dd_ed_handlers[256];
extern OpcodeEntry fd_ed_handlers[256];

// Build all handler tables
void build_handler_tables();

// Individual handler functions
// Base
void op_nop(CPU& cpu);
void op_halt(CPU& cpu);
void op_di(CPU& cpu);
void op_ei(CPU& cpu);
void op_ld_r_r(CPU& cpu);
void op_ld_r_n(CPU& cpu);
void op_ld_r_hl(CPU& cpu);
void op_ld_hl_r(CPU& cpu);
void op_ld_hl_n(CPU& cpu);
void op_ld_a_bc(CPU& cpu);
void op_ld_a_de(CPU& cpu);
void op_ld_a_nn(CPU& cpu);
void op_ld_bc_a(CPU& cpu);
void op_ld_de_a(CPU& cpu);
void op_ld_nn_a(CPU& cpu);
void op_ld_rr_nn(CPU& cpu);
void op_ld_hl_nn(CPU& cpu);
void op_ld_nn_hl(CPU& cpu);
void op_ld_sp_hl(CPU& cpu);
void op_push_rr(CPU& cpu);
void op_pop_rr(CPU& cpu);
void op_ex_de_hl(CPU& cpu);
void op_ex_af_afp(CPU& cpu);
void op_exx(CPU& cpu);
void op_ex_sp_hl(CPU& cpu);
void op_add_a(CPU& cpu);
void op_adc_a(CPU& cpu);
void op_sub(CPU& cpu);
void op_sbc_a(CPU& cpu);
void op_and(CPU& cpu);
void op_or(CPU& cpu);
void op_xor(CPU& cpu);
void op_cp(CPU& cpu);
void op_add_a_n(CPU& cpu);
void op_adc_a_n(CPU& cpu);
void op_sub_n(CPU& cpu);
void op_sbc_a_n(CPU& cpu);
void op_and_n(CPU& cpu);
void op_or_n(CPU& cpu);
void op_xor_n(CPU& cpu);
void op_cp_n(CPU& cpu);
void op_inc_r(CPU& cpu);
void op_dec_r(CPU& cpu);
void op_inc_hl(CPU& cpu);
void op_dec_hl(CPU& cpu);
void op_add_hl_rr(CPU& cpu);
void op_inc_rr(CPU& cpu);
void op_dec_rr(CPU& cpu);
void op_daa(CPU& cpu);
void op_cpl(CPU& cpu);
void op_ccf(CPU& cpu);
void op_scf(CPU& cpu);
void op_rlca(CPU& cpu);
void op_rrca(CPU& cpu);
void op_rla(CPU& cpu);
void op_rra(CPU& cpu);
void op_jp_nn(CPU& cpu);
void op_jp_cc_nn(CPU& cpu);
void op_jp_hl(CPU& cpu);
void op_jr_e(CPU& cpu);
void op_jr_cc_e(CPU& cpu);
void op_djnz_e(CPU& cpu);
void op_call_nn(CPU& cpu);
void op_call_cc_nn(CPU& cpu);
void op_ret(CPU& cpu);
void op_ret_cc(CPU& cpu);
void op_rst(CPU& cpu);
void op_in_a_n(CPU& cpu);
void op_out_n_a(CPU& cpu);

// CB
void op_cb_rot(CPU& cpu);
void op_cb_bit(CPU& cpu);
void op_cb_set(CPU& cpu);
void op_cb_res(CPU& cpu);
void op_cb_set_hl(CPU& cpu);
void op_cb_res_hl(CPU& cpu);

// ED
void op_ldi(CPU& cpu);
void op_ldir(CPU& cpu);
void op_ldd(CPU& cpu);
void op_lddr(CPU& cpu);
void op_cpi(CPU& cpu);
void op_cpir(CPU& cpu);
void op_cpd(CPU& cpu);
void op_cpdr(CPU& cpu);
void op_ini(CPU& cpu);
void op_inir(CPU& cpu);
void op_ind(CPU& cpu);
void op_indr(CPU& cpu);
void op_outi(CPU& cpu);
void op_otir(CPU& cpu);
void op_outd(CPU& cpu);
void op_otdr(CPU& cpu);
void op_adc_hl_rr(CPU& cpu);
void op_sbc_hl_rr(CPU& cpu);
void op_ld_rr_nn_ind(CPU& cpu);
void op_ld_nn_rr(CPU& cpu);
void op_neg(CPU& cpu);
void op_reti(CPU& cpu);
void op_retn(CPU& cpu);
void op_im(CPU& cpu);
void op_in_r_c(CPU& cpu);
void op_out_c_r(CPU& cpu);
void op_ld_i_a(CPU& cpu);
void op_ld_r_a(CPU& cpu);
void op_ld_a_i(CPU& cpu);
void op_ld_a_r(CPU& cpu);
void op_rld(CPU& cpu);
void op_rrd(CPU& cpu);

// DD/FD indexed
void op_dd_fd_ld_ix_nn(CPU& cpu);
void op_dd_fd_ld_nn_ix(CPU& cpu);
void op_dd_fd_ld_ix_nn_ind(CPU& cpu);
void op_dd_fd_inc_ix(CPU& cpu);
void op_dd_fd_dec_ix(CPU& cpu);
void op_dd_fd_ld_sp_ix(CPU& cpu);
void op_dd_fd_push_ix(CPU& cpu);
void op_dd_fd_pop_ix(CPU& cpu);
void op_dd_fd_ex_sp_ix(CPU& cpu);
void op_dd_fd_add_ix_rr(CPU& cpu);
void op_dd_fd_jp_ix(CPU& cpu);
void op_dd_fd_ld_r_ixd(CPU& cpu);
void op_dd_fd_ld_ixd_r(CPU& cpu);
void op_dd_fd_ld_ixd_n(CPU& cpu);
void op_dd_fd_inc_ixh(CPU& cpu);
void op_dd_fd_dec_ixh(CPU& cpu);
void op_dd_fd_inc_ixl(CPU& cpu);
void op_dd_fd_dec_ixl(CPU& cpu);
void op_dd_fd_ld_ixh_n(CPU& cpu);
void op_dd_fd_ld_ixl_n(CPU& cpu);
void op_dd_fd_ld_r_ixh(CPU& cpu);
void op_dd_fd_ld_r_ixl(CPU& cpu);
void op_dd_fd_ld_ixh_r(CPU& cpu);
void op_dd_fd_ld_ixl_r(CPU& cpu);
void op_dd_fd_ld_ixh_ixh(CPU& cpu);
void op_dd_fd_ld_ixh_ixl(CPU& cpu);
void op_dd_fd_ld_ixl_ixh(CPU& cpu);
void op_dd_fd_ld_ixl_ixl(CPU& cpu);
void op_dd_fd_ld_a_ixh(CPU& cpu);
void op_dd_fd_ld_a_ixl(CPU& cpu);
void op_dd_fd_alu_ixh(CPU& cpu);
void op_dd_fd_alu_ixl(CPU& cpu);
void op_dd_fd_alu_ixd(CPU& cpu);
void op_dd_fd_inc_ixd(CPU& cpu);
void op_dd_fd_dec_ixd(CPU& cpu);
void op_dd_fd_adc_ix_rr(CPU& cpu);
void op_dd_fd_sbc_ix_rr(CPU& cpu);

// DDCB/FDCB
void op_ddcb_fdcb_rot(CPU& cpu);
void op_ddcb_fdcb_bit(CPU& cpu);
void op_ddcb_fdcb_res(CPU& cpu);
void op_ddcb_fdcb_set(CPU& cpu);
