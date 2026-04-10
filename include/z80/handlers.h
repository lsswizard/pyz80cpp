#pragma once

#include "z80.h"
#include "registers.h"
#include "flags.h"
#include "opcode_table.h"

namespace z80 {

// ============================================================
// Instruction Handlers
// ============================================================

void handle_nop(Z80& cpu);
void handle_halt(Z80& cpu);
void handle_di(Z80& cpu);
void handle_ei(Z80& cpu);

// Load instructions
void handle_ld_r_r(Z80& cpu);
void handle_ld_r_n(Z80& cpu);
void handle_ld_r_hl(Z80& cpu);
void handle_ld_hl_r(Z80& cpu);
void handle_ld_hl_n(Z80& cpu);
void handle_ld_a_bc(Z80& cpu);
void handle_ld_a_de(Z80& cpu);
void handle_ld_a_nn(Z80& cpu);
void handle_ld_bc_a(Z80& cpu);
void handle_ld_de_a(Z80& cpu);
void handle_ld_nn_a(Z80& cpu);
void handle_ld_rr_nn(Z80& cpu);
void handle_ld_hl_nn(Z80& cpu);
void handle_ld_nn_hl(Z80& cpu);
void handle_ld_sp_hl(Z80& cpu);
void handle_push_rr(Z80& cpu);
void handle_pop_rr(Z80& cpu);

// Exchange instructions
void handle_ex_de_hl(Z80& cpu);
void handle_ex_af_afp(Z80& cpu);
void handle_exx(Z80& cpu);
void handle_ex_sp_hl(Z80& cpu);

// ALU instructions
void handle_add_a(Z80& cpu);
void handle_adc_a(Z80& cpu);
void handle_sub(Z80& cpu);
void handle_sbc_a(Z80& cpu);
void handle_and(Z80& cpu);
void handle_or(Z80& cpu);
void handle_xor(Z80& cpu);
void handle_cp(Z80& cpu);
void handle_add_a_n(Z80& cpu);
void handle_add_a_hl(Z80& cpu);
void handle_sub_n(Z80& cpu);
void handle_cp_n(Z80& cpu);

// Increment/Decrement
void handle_inc_r(Z80& cpu);
void handle_dec_r(Z80& cpu);
void handle_inc_hl(Z80& cpu);
void handle_dec_hl(Z80& cpu);
void handle_add_hl_rr(Z80& cpu);
void handle_inc_rr(Z80& cpu);
void handle_dec_rr(Z80& cpu);

// General ALU
void handle_daa(Z80& cpu);
void handle_cpl(Z80& cpu);
void handle_ccf(Z80& cpu);
void handle_scf(Z80& cpu);

// Rotates
void handle_rlca(Z80& cpu);
void handle_rrca(Z80& cpu);
void handle_rla(Z80& cpu);
void handle_rra(Z80& cpu);

// Jump instructions
void handle_jp_nn(Z80& cpu);
void handle_jp_cc_nn(Z80& cpu);
void handle_jp_hl(Z80& cpu);
void handle_jr_e(Z80& cpu);
void handle_jr_cc_e(Z80& cpu);
void handle_djnz_e(Z80& cpu);

// Call/Return
void handle_call_nn(Z80& cpu);
void handle_call_cc_nn(Z80& cpu);
void handle_ret(Z80& cpu);
void handle_ret_cc(Z80& cpu);
void handle_rst(Z80& cpu);

// I/O
void handle_in_a_n(Z80& cpu);
void handle_out_n_a(Z80& cpu);

// CB prefix handlers
void handle_rlc_r(Z80& cpu);
void handle_rrc_r(Z80& cpu);
void handle_rl_r(Z80& cpu);
void handle_rr_r(Z80& cpu);
void handle_sla_r(Z80& cpu);
void handle_sra_r(Z80& cpu);
void handle_sll_r(Z80& cpu);
void handle_srl_r(Z80& cpu);
void handle_cb_bit(Z80& cpu);
void handle_cb_res(Z80& cpu);
void handle_cb_set(Z80& cpu);

// ED prefix handlers
void handle_ldi(Z80& cpu);
void handle_ldir(Z80& cpu);
void handle_ldd(Z80& cpu);
void handle_lddr(Z80& cpu);
void handle_cpi(Z80& cpu);
void handle_cpir(Z80& cpu);
void handle_cpd(Z80& cpu);
void handle_cpdr(Z80& cpu);
void handle_ini(Z80& cpu);
void handle_inir(Z80& cpu);
void handle_ind(Z80& cpu);
void handle_indr(Z80& cpu);
void handle_outi(Z80& cpu);
void handle_otir(Z80& cpu);
void handle_outd(Z80& cpu);
void handle_otdr(Z80& cpu);
void handle_adc_hl_rr(Z80& cpu);
void handle_sbc_hl_rr(Z80& cpu);
void handle_ld_rr_nn_ind(Z80& cpu);
void handle_ld_nn_rr(Z80& cpu);
void handle_neg(Z80& cpu);
void handle_reti(Z80& cpu);
void handle_retn(Z80& cpu);
void handle_im(Z80& cpu);
void handle_in_r_c(Z80& cpu);
void handle_out_c_r(Z80& cpu);
void handle_ld_i_a(Z80& cpu);
void handle_ld_r_a(Z80& cpu);
void handle_ld_a_i(Z80& cpu);
void handle_ld_a_r(Z80& cpu);
void handle_rld(Z80& cpu);
void handle_rrd(Z80& cpu);

// DD/FD prefix handlers
void handle_dd_fd_ld_ix_nn(Z80& cpu);
void handle_dd_fd_ld_nn_ix(Z80& cpu);
void handle_dd_fd_ld_ix_nn_ind(Z80& cpu);
void handle_dd_fd_inc_ix(Z80& cpu);
void handle_dd_fd_dec_ix(Z80& cpu);
void handle_dd_fd_ld_sp_ix(Z80& cpu);
void handle_dd_fd_push_ix(Z80& cpu);
void handle_dd_fd_pop_ix(Z80& cpu);
void handle_dd_fd_ex_sp_ix(Z80& cpu);
void handle_dd_fd_add_ix_rr(Z80& cpu);
void handle_dd_fd_jp_ix(Z80& cpu);
void handle_dd_fd_ld_r_ixd(Z80& cpu);
void handle_dd_fd_ld_ixd_r(Z80& cpu);
void handle_dd_fd_ld_ixd_n(Z80& cpu);
void handle_dd_fd_add_a_ixd(Z80& cpu);
void handle_dd_fd_sub_ixd(Z80& cpu);
void handle_dd_fd_and_ixd(Z80& cpu);
void handle_dd_fd_or_ixd(Z80& cpu);
void handle_dd_fd_xor_ixd(Z80& cpu);
void handle_dd_fd_cp_ixd(Z80& cpu);
void handle_dd_fd_inc_ixd(Z80& cpu);
void handle_dd_fd_dec_ixd(Z80& cpu);

// DDCB/FDCB handlers
void handle_ddcb_fdcb_rot(Z80& cpu);
void handle_ddcb_fdcb_bit(Z80& cpu);
void handle_ddcb_fdcb_res(Z80& cpu);
void handle_ddcb_fdcb_set(Z80& cpu);

} // namespace z80