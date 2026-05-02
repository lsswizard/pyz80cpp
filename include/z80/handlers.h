#pragma once

#include "z80.h"

namespace z80 {

// ============================================================
// Handler declarations
// ============================================================

// Misc / control
void handle_nop(Z80&);
void handle_halt(Z80&);
void handle_di(Z80&);
void handle_ei(Z80&);
void handle_im(Z80&);

// Load — register to register / immediate
void handle_ld_r_r(Z80&);
void handle_ld_r_n(Z80&);
void handle_ld_r_hl(Z80&);
void handle_ld_hl_r(Z80&);
void handle_ld_hl_n(Z80&);

// Load — 16-bit
void handle_ld_rr_nn(Z80&);
void handle_ld_hl_nn(Z80&);   // LD HL, (nn)
void handle_ld_nn_hl(Z80&);   // LD (nn), HL
void handle_ld_sp_hl(Z80&);
void handle_push_rr(Z80&);
void handle_pop_rr(Z80&);

// Load — accumulator ↔ memory
void handle_ld_a_bc(Z80&);
void handle_ld_a_de(Z80&);
void handle_ld_a_nn(Z80&);
void handle_ld_bc_a(Z80&);
void handle_ld_de_a(Z80&);
void handle_ld_nn_a(Z80&);

// Load — ED-prefix (LD rr,(nn) / LD (nn),rr / LD A,I / LD A,R / LD I,A / LD R,A)
void handle_ld_rr_nn_ind(Z80&);
void handle_ld_nn_rr_ind(Z80&);
void handle_ld_a_i(Z80&);
void handle_ld_a_r(Z80&);
void handle_ld_i_a(Z80&);
void handle_ld_r_a(Z80&);

// Exchange
void handle_ex_de_hl(Z80&);
void handle_ex_af_afp(Z80&);
void handle_exx(Z80&);
void handle_ex_sp_hl(Z80&);

// ALU — register / (HL)
void handle_add_a(Z80&);
void handle_adc_a(Z80&);
void handle_sub(Z80&);
void handle_sbc_a(Z80&);
void handle_and(Z80&);
void handle_or(Z80&);
void handle_xor(Z80&);
void handle_cp(Z80&);

// ALU — immediate
void handle_add_a_n(Z80&);
void handle_adc_a_n(Z80&);
void handle_sub_n(Z80&);
void handle_sbc_a_n(Z80&);
void handle_and_n(Z80&);
void handle_or_n(Z80&);
void handle_xor_n(Z80&);
void handle_cp_n(Z80&);

// ALU — (HL) addressed (need explicit T-state read)
void handle_add_a_hl(Z80&);

// Inc / Dec
void handle_inc_r(Z80&);
void handle_dec_r(Z80&);
void handle_inc_hl(Z80&);
void handle_dec_hl(Z80&);
void handle_inc_rr(Z80&);
void handle_dec_rr(Z80&);
void handle_add_hl_rr(Z80&);

// Special ALU
void handle_daa(Z80&);
void handle_cpl(Z80&);
void handle_ccf(Z80&);
void handle_scf(Z80&);
void handle_neg(Z80&);
void handle_rld(Z80&);
void handle_rrd(Z80&);
void handle_adc_hl_rr(Z80&);
void handle_sbc_hl_rr(Z80&);

// Accumulator rotates
void handle_rlca(Z80&);
void handle_rrca(Z80&);
void handle_rla(Z80&);
void handle_rra(Z80&);

// CB-prefix rotates/shifts/bit-ops
void handle_rlc_r(Z80&);
void handle_rrc_r(Z80&);
void handle_rl_r(Z80&);
void handle_rr_r(Z80&);
void handle_sla_r(Z80&);
void handle_sra_r(Z80&);
void handle_sll_r(Z80&);
void handle_srl_r(Z80&);
void handle_cb_bit(Z80&);
void handle_cb_res(Z80&);
void handle_cb_set(Z80&);

// DDCB/FDCB indexed bit-ops
void handle_ddcb_fdcb_rot(Z80&);
void handle_ddcb_fdcb_bit(Z80&);
void handle_ddcb_fdcb_res(Z80&);
void handle_ddcb_fdcb_set(Z80&);

// Jump / call / return
void handle_jp_nn(Z80&);
void handle_jp_cc_nn(Z80&);
void handle_jp_hl(Z80&);
void handle_jr_e(Z80&);
void handle_jr_cc_e(Z80&);
void handle_djnz_e(Z80&);
void handle_call_nn(Z80&);
void handle_call_cc_nn(Z80&);
void handle_ret(Z80&);
void handle_ret_cc(Z80&);
void handle_reti(Z80&);
void handle_retn(Z80&);
void handle_rst(Z80&);

// I/O
void handle_in_a_n(Z80&);
void handle_out_n_a(Z80&);
void handle_in_r_c(Z80&);
void handle_out_c_r(Z80&);

// Block transfer / search
void handle_ldi(Z80&);
void handle_ldd(Z80&);
void handle_ldir(Z80&);
void handle_lddr(Z80&);
void handle_cpi(Z80&);
void handle_cpd(Z80&);
void handle_cpir(Z80&);
void handle_cpdr(Z80&);

// Block I/O
void handle_ini(Z80&);
void handle_ind(Z80&);
void handle_inir(Z80&);
void handle_indr(Z80&);
void handle_outi(Z80&);
void handle_outd(Z80&);
void handle_otir(Z80&);
void handle_otdr(Z80&);

// DD/FD-prefix — IX/IY load / arithmetic
void handle_dd_fd_ld_ix_nn(Z80&);
void handle_dd_fd_ld_nn_ix(Z80&);
void handle_dd_fd_ld_ix_nn_ind(Z80&);
void handle_dd_fd_inc_ix(Z80&);
void handle_dd_fd_dec_ix(Z80&);
void handle_dd_fd_inc_dec_ixhl(Z80&);  // Unified INC/DEC for IXH/IXL/IYH/IYL
void handle_dd_fd_ld_sp_ix(Z80&);
void handle_dd_fd_push_ix(Z80&);
void handle_dd_fd_pop_ix(Z80&);
void handle_dd_fd_ex_sp_ix(Z80&);
void handle_dd_fd_add_ix_rr(Z80&);
void handle_dd_fd_jp_ix(Z80&);

// DD/FD indexed memory operations — (IX+d)/(IY+d)
void handle_dd_fd_ld_r_ixd(Z80&);
void handle_dd_fd_ld_ixd_r(Z80&);
void handle_dd_fd_ld_ixd_n(Z80&);
void handle_dd_fd_inc_ixd(Z80&);
void handle_dd_fd_dec_ixd(Z80&);

// DD/FD Load operations - LD r,IXH/IXL and LD IXH/L,r
void handle_ld_ixhl_r(Z80&);
void handle_ld_ixhl_n(Z80&);

// DD/FD ALU with IXH/IXL (unified handler for ADD/ADC/SUB/SBC/AND/OR/XOR/CP)
void handle_dd_fd_alu_ixhl(Z80&);

// DD/FD ALU with (IX+d)/(IY+d)
void handle_dd_fd_add_a_ixd(Z80&);
void handle_dd_fd_sub_ixd(Z80&);
void handle_dd_fd_and_ixd(Z80&);
void handle_dd_fd_xor_ixd(Z80&);
void handle_dd_fd_or_ixd(Z80&);
void handle_dd_fd_cp_ixd(Z80&);

} // namespace z80
