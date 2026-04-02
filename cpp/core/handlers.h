#pragma once

#include <cstdint>

class CPU;

typedef int (*OpHandler)(CPU& cpu);

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
int op_nop(CPU& cpu);
int op_halt(CPU& cpu);
int op_di(CPU& cpu);
int op_ei(CPU& cpu);
int op_ld_r_r(CPU& cpu);
int op_ld_r_n(CPU& cpu);
int op_ld_r_hl(CPU& cpu);
int op_ld_hl_r(CPU& cpu);
int op_ld_hl_n(CPU& cpu);
int op_ld_a_bc(CPU& cpu);
int op_ld_a_de(CPU& cpu);
int op_ld_a_nn(CPU& cpu);
int op_ld_bc_a(CPU& cpu);
int op_ld_de_a(CPU& cpu);
int op_ld_nn_a(CPU& cpu);
int op_ld_rr_nn(CPU& cpu);
int op_ld_hl_nn(CPU& cpu);
int op_ld_nn_hl(CPU& cpu);
int op_ld_sp_hl(CPU& cpu);
int op_push_rr(CPU& cpu);
int op_pop_rr(CPU& cpu);
int op_ex_de_hl(CPU& cpu);
int op_ex_af_afp(CPU& cpu);
int op_exx(CPU& cpu);
int op_ex_sp_hl(CPU& cpu);
int op_add_a(CPU& cpu);
int op_adc_a(CPU& cpu);
int op_sub(CPU& cpu);
int op_sbc_a(CPU& cpu);
int op_and(CPU& cpu);
int op_or(CPU& cpu);
int op_xor(CPU& cpu);
int op_cp(CPU& cpu);
int op_add_a_n(CPU& cpu);
int op_adc_a_n(CPU& cpu);
int op_sub_n(CPU& cpu);
int op_sbc_a_n(CPU& cpu);
int op_and_n(CPU& cpu);
int op_or_n(CPU& cpu);
int op_xor_n(CPU& cpu);
int op_cp_n(CPU& cpu);
int op_inc_r(CPU& cpu);
int op_dec_r(CPU& cpu);
int op_inc_hl(CPU& cpu);
int op_dec_hl(CPU& cpu);
int op_add_hl_rr(CPU& cpu);
int op_inc_rr(CPU& cpu);
int op_dec_rr(CPU& cpu);
int op_daa(CPU& cpu);
int op_cpl(CPU& cpu);
int op_ccf(CPU& cpu);
int op_scf(CPU& cpu);
int op_rlca(CPU& cpu);
int op_rrca(CPU& cpu);
int op_rla(CPU& cpu);
int op_rra(CPU& cpu);
int op_jp_nn(CPU& cpu);
int op_jp_cc_nn(CPU& cpu);
int op_jp_hl(CPU& cpu);
int op_jr_e(CPU& cpu);
int op_jr_cc_e(CPU& cpu);
int op_djnz_e(CPU& cpu);
int op_call_nn(CPU& cpu);
int op_call_cc_nn(CPU& cpu);
int op_ret(CPU& cpu);
int op_ret_cc(CPU& cpu);
int op_rst(CPU& cpu);
int op_in_a_n(CPU& cpu);
int op_out_n_a(CPU& cpu);

// CB
int op_cb_rot(CPU& cpu);
int op_cb_bit(CPU& cpu);
int op_cb_set(CPU& cpu);
int op_cb_res(CPU& cpu);
int op_cb_set_hl(CPU& cpu);
int op_cb_res_hl(CPU& cpu);

// ED
int op_ldi(CPU& cpu);
int op_ldir(CPU& cpu);
int op_ldd(CPU& cpu);
int op_lddr(CPU& cpu);
int op_cpi(CPU& cpu);
int op_cpir(CPU& cpu);
int op_cpd(CPU& cpu);
int op_cpdr(CPU& cpu);
int op_ini(CPU& cpu);
int op_inir(CPU& cpu);
int op_ind(CPU& cpu);
int op_indr(CPU& cpu);
int op_outi(CPU& cpu);
int op_otir(CPU& cpu);
int op_outd(CPU& cpu);
int op_otdr(CPU& cpu);
int op_adc_hl_rr(CPU& cpu);
int op_sbc_hl_rr(CPU& cpu);
int op_ld_rr_nn_ind(CPU& cpu);
int op_ld_nn_rr(CPU& cpu);
int op_neg(CPU& cpu);
int op_reti(CPU& cpu);
int op_retn(CPU& cpu);
int op_im(CPU& cpu);
int op_in_r_c(CPU& cpu);
int op_out_c_r(CPU& cpu);
int op_ld_i_a(CPU& cpu);
int op_ld_r_a(CPU& cpu);
int op_ld_a_i(CPU& cpu);
int op_ld_a_r(CPU& cpu);
int op_rld(CPU& cpu);
int op_rrd(CPU& cpu);

// DD/FD indexed
int op_dd_fd_ld_ix_nn(CPU& cpu);
int op_dd_fd_ld_nn_ix(CPU& cpu);
int op_dd_fd_ld_ix_nn_ind(CPU& cpu);
int op_dd_fd_inc_ix(CPU& cpu);
int op_dd_fd_dec_ix(CPU& cpu);
int op_dd_fd_ld_sp_ix(CPU& cpu);
int op_dd_fd_push_ix(CPU& cpu);
int op_dd_fd_pop_ix(CPU& cpu);
int op_dd_fd_ex_sp_ix(CPU& cpu);
int op_dd_fd_add_ix_rr(CPU& cpu);
int op_dd_fd_jp_ix(CPU& cpu);
int op_dd_fd_ld_r_ixd(CPU& cpu);
int op_dd_fd_ld_ixd_r(CPU& cpu);
int op_dd_fd_ld_ixd_n(CPU& cpu);
int op_dd_fd_inc_ixh(CPU& cpu);
int op_dd_fd_dec_ixh(CPU& cpu);
int op_dd_fd_inc_ixl(CPU& cpu);
int op_dd_fd_dec_ixl(CPU& cpu);
int op_dd_fd_ld_ixh_n(CPU& cpu);
int op_dd_fd_ld_ixl_n(CPU& cpu);
int op_dd_fd_ld_r_ixh(CPU& cpu);
int op_dd_fd_ld_r_ixl(CPU& cpu);
int op_dd_fd_ld_ixh_r(CPU& cpu);
int op_dd_fd_ld_ixl_r(CPU& cpu);
int op_dd_fd_ld_ixh_ixh(CPU& cpu);
int op_dd_fd_ld_ixh_ixl(CPU& cpu);
int op_dd_fd_ld_ixl_ixh(CPU& cpu);
int op_dd_fd_ld_ixl_ixl(CPU& cpu);
int op_dd_fd_ld_a_ixh(CPU& cpu);
int op_dd_fd_ld_a_ixl(CPU& cpu);
int op_dd_fd_alu_ixh(CPU& cpu);
int op_dd_fd_alu_ixl(CPU& cpu);
int op_dd_fd_alu_ixd(CPU& cpu);
int op_dd_fd_inc_ixd(CPU& cpu);
int op_dd_fd_dec_ixd(CPU& cpu);
int op_dd_fd_adc_ix_rr(CPU& cpu);
int op_dd_fd_sbc_ix_rr(CPU& cpu);

// DDCB/FDCB
int op_ddcb_fdcb_rot(CPU& cpu);
int op_ddcb_fdcb_bit(CPU& cpu);
int op_ddcb_fdcb_res(CPU& cpu);
int op_ddcb_fdcb_set(CPU& cpu);
