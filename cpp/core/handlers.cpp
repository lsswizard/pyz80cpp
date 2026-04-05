#include "handlers.h"
#include "cpu.h"
#include "flags.h"
#include <cstring>

using namespace z80flags;

// ============================================================
// Helper macros for register access
// ============================================================
#define REG_A cpu.regs.A
#define REG_F cpu.regs.F
#define REG_B cpu.regs.B
#define REG_C cpu.regs.C
#define REG_D cpu.regs.D
#define REG_E cpu.regs.E
#define REG_H cpu.regs.H
#define REG_L cpu.regs.L
#define REG_BC cpu.regs.BC()
#define REG_DE cpu.regs.DE()
#define REG_HL cpu.regs.HL()
#define REG_AF cpu.regs.AF()
#define SET_BC(v) cpu.regs.set_BC(v)
#define SET_DE(v) cpu.regs.set_DE(v)
#define SET_HL(v) cpu.regs.set_HL(v)
#define SET_AF(v) cpu.regs.set_AF(v)
#define OPCODE cpu.current_opcode
#define CYCLES cpu.cycles
#define MEM cpu._mem
#define MEM_RD(a) cpu._bus_read(a)
#define MEM_WR(a,v) cpu._bus_write(a, v)
#define IO_RD(p) cpu._bus_io_read(p)
#define IO_WR(p,v) cpu._bus_io_write(p, v)

// ============================================================
// NOP handlers for unknown opcodes
// ============================================================
static void op_nop_base(CPU& cpu) { }
static void op_nop_cb(CPU& cpu) { cpu._wait(4); }
static void op_nop_ed(CPU& cpu) { cpu._wait(4); }
static void op_nop_dd(CPU& cpu) { }
static void op_nop_fd(CPU& cpu) { }
static void op_nop_ddcb(CPU& cpu) { cpu._wait(19); }
static void op_nop_fdcb(CPU& cpu) { cpu._wait(19); }

// ============================================================
// Base instruction handlers
// ============================================================
void op_nop(CPU& cpu) { }

void op_halt(CPU& cpu) {
    cpu.halted = true;
    cpu.regs.PC = (cpu.regs.PC - 1) & 0xFFFF;
    cpu._pc_modified = true;
}

void op_di(CPU& cpu) {
    cpu.regs.IFF1 = cpu.regs.IFF2 = false;
    cpu.regs.EI_PENDING = false;
}

void op_ei(CPU& cpu) {
    cpu.regs.EI_PENDING = true;
    cpu.regs.EI_JUST_RESOLVED = false;
}

void op_ld_r_r(CPU& cpu) {
    uint8_t dest = (OPCODE >> 3) & 7;
    uint8_t src = OPCODE & 7;
    uint8_t val;
    switch (src) {
        case 0: val = REG_B; break;
        case 1: val = REG_C; break;
        case 2: val = REG_D; break;
        case 3: val = REG_E; break;
        case 4: val = REG_H; break;
        case 5: val = REG_L; break;
        case 6: val = MEM_RD(REG_HL); break;
        default: val = REG_A; break;
    }
    switch (dest) {
        case 0: REG_B = val; break;
        case 1: REG_C = val; break;
        case 2: REG_D = val; break;
        case 3: REG_E = val; break;
        case 4: REG_H = val; break;
        case 5: REG_L = val; break;
        case 6: MEM_WR(REG_HL, val); break;
        default: REG_A = val; break;
    }
}

void op_ld_r_n(CPU& cpu) {
    uint8_t dest = (OPCODE >> 3) & 7;
    uint8_t val = cpu._bus_read(cpu.regs.PC++);
    switch (dest) {
        case 0: REG_B = val; break;
        case 1: REG_C = val; break;
        case 2: REG_D = val; break;
        case 3: REG_E = val; break;
        case 4: REG_H = val; break;
        case 5: REG_L = val; break;
        case 6: MEM_WR(REG_HL, val); return;
        default: REG_A = val; break;
    }
}

void op_ld_r_hl(CPU& cpu) {
    uint8_t dest = (OPCODE >> 3) & 7;
    uint8_t val = MEM_RD(REG_HL);
    switch (dest) {
        case 0: REG_B = val; break;
        case 1: REG_C = val; break;
        case 2: REG_D = val; break;
        case 3: REG_E = val; break;
        case 4: REG_H = val; break;
        case 5: REG_L = val; break;
        default: REG_A = val; break;
    }
}

void op_ld_hl_r(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    uint8_t val;
    switch (src) {
        case 0: val = REG_B; break;
        case 1: val = REG_C; break;
        case 2: val = REG_D; break;
        case 3: val = REG_E; break;
        case 4: val = REG_H; break;
        default: val = REG_L; break;
    }
    MEM_WR(REG_HL, val);
}

void op_ld_hl_n(CPU& cpu) {
    uint8_t val = cpu._bus_read(cpu.regs.PC++);
    MEM_WR(REG_HL, val);
}

void op_ld_a_bc(CPU& cpu) {
    uint16_t bc = REG_BC;
    REG_A = MEM_RD(bc);
    cpu.regs.MEMPTR = (bc + 1) & 0xFFFF;
}

void op_ld_a_de(CPU& cpu) {
    uint16_t de = REG_DE;
    REG_A = MEM_RD(de);
    cpu.regs.MEMPTR = (de + 1) & 0xFFFF;
}

void op_ld_a_nn(CPU& cpu) {
    uint8_t lo = cpu._bus_read(cpu.regs.PC++);
    uint8_t hi = cpu._bus_read(cpu.regs.PC++);
    uint16_t addr = lo | (hi << 8);
    REG_A = MEM_RD(addr);
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
}

void op_ld_bc_a(CPU& cpu) {
    uint16_t bc = REG_BC;
    uint8_t a = REG_A;
    MEM_WR(bc, a);
    cpu.regs.MEMPTR = ((a << 8) | ((bc + 1) & 0xFF)) & 0xFFFF;
}

void op_ld_de_a(CPU& cpu) {
    uint16_t de = REG_DE;
    uint8_t a = REG_A;
    MEM_WR(de, a);
    cpu.regs.MEMPTR = ((a << 8) | ((de + 1) & 0xFF)) & 0xFFFF;
}

void op_ld_nn_a(CPU& cpu) {
    uint8_t lo = cpu._bus_read(cpu.regs.PC++);
    uint8_t hi = cpu._bus_read(cpu.regs.PC++);
    uint16_t addr = lo | (hi << 8);
    uint8_t a = REG_A;
    MEM_WR(addr, a);
    cpu.regs.MEMPTR = ((a << 8) | ((addr + 1) & 0xFF)) & 0xFFFF;
}

void op_ld_rr_nn(CPU& cpu) {
    int pair = (OPCODE >> 4) & 3;
    uint8_t lo = cpu._bus_read(cpu.regs.PC++);
    uint8_t hi = cpu._bus_read(cpu.regs.PC++);
    uint16_t val = lo | (hi << 8);
    cpu.regs.MEMPTR = (cpu.regs.PC) & 0xFFFF;
    cpu.regs.set_reg16(pair, val);
}

void op_ld_hl_nn(CPU& cpu) {
    uint8_t pclo = cpu._bus_read(cpu.regs.PC++);
    uint8_t pchi = cpu._bus_read(cpu.regs.PC++);
    uint16_t addr = pclo | (pchi << 8);
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    uint8_t lo = MEM_RD(addr);
    uint8_t hi = MEM_RD((addr + 1) & 0xFFFF);
    SET_HL((uint16_t)(lo | (hi << 8)));
}

void op_ld_nn_hl(CPU& cpu) {
    uint8_t lo = cpu._bus_read(cpu.regs.PC++);
    uint8_t hi = cpu._bus_read(cpu.regs.PC++);
    uint16_t addr = lo | (hi << 8);
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    MEM_WR(addr, REG_L);
    MEM_WR((addr + 1) & 0xFFFF, REG_H);
}

void op_ld_sp_hl(CPU& cpu) {
    cpu.regs.SP = REG_HL;
    cpu._wait(2); // SP=HL takes 6T (4+2)
}

void op_push_rr(CPU& cpu) {
    int pair = (OPCODE >> 4) & 3;
    uint16_t val = cpu.regs.get_reg16_push(pair);
    cpu.regs.MEMPTR = (cpu.regs.SP - 1) & 0xFFFF;
    cpu._wait(1); // Internal execution (1T)
    cpu.regs.SP = (cpu.regs.SP - 1) & 0xFFFF;
    MEM_WR(cpu.regs.SP, val >> 8);
    cpu.regs.SP = (cpu.regs.SP - 1) & 0xFFFF;
    MEM_WR(cpu.regs.SP, val & 0xFF);
}

void op_pop_rr(CPU& cpu) {
    int pair = (OPCODE >> 4) & 3;
    uint16_t sp = cpu.regs.SP;
    cpu.regs.MEMPTR = (sp + 1) & 0xFFFF;
    uint8_t lo = MEM_RD(sp);
    uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
    cpu.regs.SP = (sp + 2) & 0xFFFF;
    cpu.regs.set_reg16_push(pair, (uint16_t)(lo | (hi << 8)));
}

void op_ex_de_hl(CPU& cpu) {
    uint16_t t = REG_DE;
    SET_DE(REG_HL);
    SET_HL(t);
}

void op_ex_af_afp(CPU& cpu) {
    cpu.regs.swap_shadow();
}

void op_exx(CPU& cpu) {
    cpu.regs.swap_shadow_all();
}

void op_ex_sp_hl(CPU& cpu) {
    uint16_t sp = cpu.regs.SP;
    uint8_t lo = MEM_RD(sp);
    uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
    uint16_t temp = (uint16_t)(lo | (hi << 8));
    cpu.regs.MEMPTR = (sp + 1) & 0xFFFF;
    cpu._wait(1); // Internal execution before write (1T)
    MEM_WR((sp + 1) & 0xFFFF, REG_H);
    MEM_WR(sp, REG_L);
    cpu._wait(2); // Internal execution after write (2T)
    SET_HL(temp);
}

// --- 8-bit ALU ---
static inline void _alu_add(CPU& cpu, uint8_t b) {
    uint8_t a = REG_A;
    REG_A = (a + b) & 0xFF;
    REG_F = ADD_FLAGS[(a << 8) | b];
}

static inline void _alu_add_hl(CPU& cpu) {
    uint8_t a = REG_A;
    uint8_t b = MEM_RD(REG_HL);
    REG_A = (a + b) & 0xFF;
    REG_F = ADD_FLAGS[(a << 8) | b];
}

void op_add_a(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) { _alu_add_hl(cpu); return; }
    uint8_t b;
    switch (src) {
        case 0: b = REG_B; break;
        case 1: b = REG_C; break;
        case 2: b = REG_D; break;
        case 3: b = REG_E; break;
        case 4: b = REG_H; break;
        case 5: b = REG_L; break;
        default: b = REG_A; break;
    }
    _alu_add(cpu, b);
}

void op_add_a_n(CPU& cpu) {
    uint8_t b = cpu._bus_read(cpu.regs.PC++);
    _alu_add(cpu, b);
}

static inline void _alu_adc(CPU& cpu, uint8_t b) {
    uint8_t a = REG_A;
    uint8_t c = REG_F & FLAG_C;
    REG_A = (a + b + c) & 0xFF;
    REG_F = (c ? ADC_FLAGS : ADD_FLAGS)[(a << 8) | b];
}

void op_adc_a(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t b = MEM_RD(REG_HL);
        _alu_adc(cpu, b);
        return;
    }
    uint8_t b;
    switch (src) {
        case 0: b = REG_B; break;
        case 1: b = REG_C; break;
        case 2: b = REG_D; break;
        case 3: b = REG_E; break;
        case 4: b = REG_H; break;
        case 5: b = REG_L; break;
        default: b = REG_A; break;
    }
    _alu_adc(cpu, b);
}

void op_adc_a_n(CPU& cpu) {
    uint8_t b = cpu._bus_read(cpu.regs.PC++);
    _alu_adc(cpu, b);
}

static inline void _alu_sub(CPU& cpu, uint8_t b) {
    uint8_t a = REG_A;
    REG_A = (a - b) & 0xFF;
    REG_F = SUB_FLAGS[(a << 8) | b];
}

void op_sub(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t b = MEM_RD(REG_HL);
        _alu_sub(cpu, b);
        return;
    }
    uint8_t b;
    switch (src) {
        case 0: b = REG_B; break;
        case 1: b = REG_C; break;
        case 2: b = REG_D; break;
        case 3: b = REG_E; break;
        case 4: b = REG_H; break;
        case 5: b = REG_L; break;
        default: b = REG_A; break;
    }
    _alu_sub(cpu, b);
}

void op_sub_n(CPU& cpu) {
    uint8_t b = cpu._bus_read(cpu.regs.PC++);
    _alu_sub(cpu, b);
}

static inline void _alu_sbc(CPU& cpu, uint8_t b) {
    uint8_t a = REG_A;
    uint8_t c = REG_F & FLAG_C;
    REG_A = (a - b - c) & 0xFF;
    REG_F = (c ? SBC_FLAGS : SUB_FLAGS)[(a << 8) | b];
}

void op_sbc_a(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t b = MEM_RD(REG_HL);
        _alu_sbc(cpu, b);
        return;
    }
    uint8_t b;
    switch (src) {
        case 0: b = REG_B; break;
        case 1: b = REG_C; break;
        case 2: b = REG_D; break;
        case 3: b = REG_E; break;
        case 4: b = REG_H; break;
        case 5: b = REG_L; break;
        default: b = REG_A; break;
    }
    _alu_sbc(cpu, b);
}

void op_sbc_a_n(CPU& cpu) {
    uint8_t b = cpu._bus_read(cpu.regs.PC++);
    _alu_sbc(cpu, b);
}

static inline void _alu_and(CPU& cpu, uint8_t b) {
    REG_A &= b;
    REG_F = SZHZP_TABLE[REG_A];
}

void op_and(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t b = MEM_RD(REG_HL);
        _alu_and(cpu, b);
        return;
    }
    uint8_t b;
    switch (src) {
        case 0: b = REG_B; break;
        case 1: b = REG_C; break;
        case 2: b = REG_D; break;
        case 3: b = REG_E; break;
        case 4: b = REG_H; break;
        case 5: b = REG_L; break;
        default: b = REG_A; break;
    }
    _alu_and(cpu, b);
}

void op_and_n(CPU& cpu) {
    uint8_t b = cpu._bus_read(cpu.regs.PC++);
    _alu_and(cpu, b);
}

static inline void _alu_or(CPU& cpu, uint8_t b) {
    REG_A |= b;
    REG_F = SZ53P_TABLE[REG_A];
}

void op_or(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t b = MEM_RD(REG_HL);
        _alu_or(cpu, b);
        return;
    }
    uint8_t b;
    switch (src) {
        case 0: b = REG_B; break;
        case 1: b = REG_C; break;
        case 2: b = REG_D; break;
        case 3: b = REG_E; break;
        case 4: b = REG_H; break;
        case 5: b = REG_L; break;
        default: b = REG_A; break;
    }
    _alu_or(cpu, b);
}

void op_or_n(CPU& cpu) {
    uint8_t b = cpu._bus_read(cpu.regs.PC++);
    _alu_or(cpu, b);
}

static inline void _alu_xor(CPU& cpu, uint8_t b) {
    REG_A ^= b;
    REG_F = SZ53P_TABLE[REG_A];
}

void op_xor(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t b = MEM_RD(REG_HL);
        _alu_xor(cpu, b);
        return;
    }
    uint8_t b;
    switch (src) {
        case 0: b = REG_B; break;
        case 1: b = REG_C; break;
        case 2: b = REG_D; break;
        case 3: b = REG_E; break;
        case 4: b = REG_H; break;
        case 5: b = REG_L; break;
        default: b = REG_A; break;
    }
    _alu_xor(cpu, b);
}

void op_xor_n(CPU& cpu) {
    uint8_t b = cpu._bus_read(cpu.regs.PC++);
    _alu_xor(cpu, b);
}

static inline void _alu_cp(CPU& cpu, uint8_t b) {
    uint8_t a = REG_A;
    REG_F = (SUB_FLAGS[(a << 8) | b] & ~(FLAG_F3 | FLAG_F5)) | (b & (FLAG_F3 | FLAG_F5));
}

void op_cp(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        cpu.regs.MEMPTR = REG_HL;
        uint8_t b = MEM_RD(REG_HL);
        _alu_cp(cpu, b);
        return;
    }
    uint8_t b;
    switch (src) {
        case 0: b = REG_B; break;
        case 1: b = REG_C; break;
        case 2: b = REG_D; break;
        case 3: b = REG_E; break;
        case 4: b = REG_H; break;
        case 5: b = REG_L; break;
        default: b = REG_A; break;
    }
    _alu_cp(cpu, b);
}

void op_cp_n(CPU& cpu) {
    uint8_t b = cpu._bus_read(cpu.regs.PC++);
    cpu.regs.MEMPTR = cpu.regs.PC;
    _alu_cp(cpu, b);
}

// INC/DEC
void op_inc_r(CPU& cpu) {
    int dest = (OPCODE >> 3) & 7;
    if (dest == 6) {
        uint8_t val = MEM_RD(REG_HL);
        uint8_t nv = (val + 1) & 0xFF;
        cpu._wait(1); // Internal execution before write (1T)
        MEM_WR(REG_HL, nv);
        REG_F = (REG_F & FLAG_C) | INC_FLAGS[val];
        return;
    }
    uint8_t val;
    switch (dest) {
        case 0: val = REG_B; break;
        case 1: val = REG_C; break;
        case 2: val = REG_D; break;
        case 3: val = REG_E; break;
        case 4: val = REG_H; break;
        case 5: val = REG_L; break;
        default: val = REG_A; break;
    }
    uint8_t nv = (val + 1) & 0xFF;
    switch (dest) {
        case 0: REG_B = nv; break;
        case 1: REG_C = nv; break;
        case 2: REG_D = nv; break;
        case 3: REG_E = nv; break;
        case 4: REG_H = nv; break;
        case 5: REG_L = nv; break;
        default: REG_A = nv; break;
    }
    REG_F = (REG_F & FLAG_C) | INC_FLAGS[val];
}

void op_dec_r(CPU& cpu) {
    int dest = (OPCODE >> 3) & 7;
    if (dest == 6) {
        uint8_t val = MEM_RD(REG_HL);
        uint8_t nv = (val - 1) & 0xFF;
        cpu._wait(1); // Internal execution before write (1T)
        MEM_WR(REG_HL, nv);
        REG_F = (REG_F & FLAG_C) | DEC_FLAGS[val];
        return;
    }
    uint8_t val;
    switch (dest) {
        case 0: val = REG_B; break;
        case 1: val = REG_C; break;
        case 2: val = REG_D; break;
        case 3: val = REG_E; break;
        case 4: val = REG_H; break;
        case 5: val = REG_L; break;
        default: val = REG_A; break;
    }
    uint8_t nv = (val - 1) & 0xFF;
    switch (dest) {
        case 0: REG_B = nv; break;
        case 1: REG_C = nv; break;
        case 2: REG_D = nv; break;
        case 3: REG_E = nv; break;
        case 4: REG_H = nv; break;
        case 5: REG_L = nv; break;
        default: REG_A = nv; break;
    }
    REG_F = (REG_F & FLAG_C) | DEC_FLAGS[val];
}

// 16-bit ALU
void op_add_hl_rr(CPU& cpu) {
    int pair = (OPCODE >> 4) & 3;
    uint16_t hl = REG_HL;
    uint16_t op = cpu.regs.get_reg16(pair);
    cpu._wait(7); // ADD HL,RR takes 11T (4+7)
    SET_HL((hl + op) & 0xFFFF);
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_PV)) | add16_flags(hl, op, REG_F);
}

void op_inc_rr(CPU& cpu) {
    int pair = (OPCODE >> 4) & 3;
    cpu._wait(2); // INC RR takes 6T (4+2)
    cpu.regs.set_reg16(pair, (cpu.regs.get_reg16(pair) + 1) & 0xFFFF);
}

void op_dec_rr(CPU& cpu) {
    int pair = (OPCODE >> 4) & 3;
    cpu._wait(2); // DEC RR takes 6T (4+2)
    cpu.regs.set_reg16(pair, (cpu.regs.get_reg16(pair) - 1) & 0xFFFF);
}

// Rotates
void op_rlca(CPU& cpu) {
    uint8_t a = REG_A;
    uint8_t c = (a >> 7) & 1;
    REG_A = ((a << 1) | c) & 0xFF;
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_PV)) | c | (REG_A & (FLAG_F3 | FLAG_F5));
}

void op_rrca(CPU& cpu) {
    uint8_t a = REG_A;
    uint8_t c = a & 1;
    REG_A = ((a >> 1) | (c << 7)) & 0xFF;
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_PV)) | c | (REG_A & (FLAG_F3 | FLAG_F5));
}

void op_rla(CPU& cpu) {
    uint8_t a = REG_A;
    uint8_t oc = REG_F & FLAG_C;
    uint8_t nc = (a >> 7) & 1;
    REG_A = ((a << 1) | oc) & 0xFF;
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_PV)) | nc | (REG_A & (FLAG_F3 | FLAG_F5));
}

void op_rra(CPU& cpu) {
    uint8_t a = REG_A;
    uint8_t oc = REG_F & FLAG_C;
    uint8_t nc = a & 1;
    REG_A = ((a >> 1) | (oc << 7)) & 0xFF;
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_PV)) | nc | (REG_A & (FLAG_F3 | FLAG_F5));
}

// DAA
void op_daa(CPU& cpu) {
    int n = (REG_F >> 1) & 1;
    int h = (REG_F >> 4) & 1;
    int c = REG_F & 1;
    int idx = (n << 10) | (h << 9) | (c << 8) | REG_A;
    REG_A = DAA_FULL_FLAGS[idx * 2];
    REG_F = DAA_FULL_FLAGS[idx * 2 + 1];
    cpu.regs.Q = REG_F;
}

// CPL
void op_cpl(CPU& cpu) {
    REG_A = (~REG_A) & 0xFF;
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_PV | FLAG_C)) | FLAG_H | FLAG_N | (REG_A & (FLAG_F3 | FLAG_F5));
    cpu.regs.Q = REG_F;
}

// CCF
void op_ccf(CPU& cpu) {
    uint8_t old_f = REG_F;
    uint8_t old_c = old_f & FLAG_C;
    uint8_t f = old_f & (FLAG_S | FLAG_Z | FLAG_PV);
    uint8_t result = (cpu.regs.LAST_Q ^ old_f) | REG_A;
    f |= result & (FLAG_F3 | FLAG_F5);
    if (old_c) f |= FLAG_H;
    else f |= FLAG_C;
    REG_F = f;
    cpu.regs.Q = f;
}

// SCF
void op_scf(CPU& cpu) {
    uint8_t old_f = REG_F;
    uint8_t f = (old_f & (FLAG_S | FLAG_Z | FLAG_PV)) | FLAG_C;
    uint8_t result = (cpu.regs.LAST_Q ^ old_f) | REG_A;
    f |= result & (FLAG_F3 | FLAG_F5);
    REG_F = f;
    cpu.regs.Q = f;
}

// Jumps
void op_jp_nn(CPU& cpu) {
    uint8_t lo = cpu._bus_read(cpu.regs.PC++);
    uint8_t hi = cpu._bus_read(cpu.regs.PC++);
    cpu.regs.MEMPTR = lo | (hi << 8);
    cpu.regs.PC = cpu.regs.MEMPTR;
    cpu._pc_modified = true;
}

void op_jp_cc_nn(CPU& cpu) {
    int cc = (OPCODE >> 3) & 7;
    uint8_t lo = cpu._bus_read(cpu.regs.PC++);
    uint8_t hi = cpu._bus_read(cpu.regs.PC++);
    uint16_t addr = lo | (hi << 8);
    if (cpu.check_condition(cc)) {
        cpu.regs.MEMPTR = addr;
        cpu.regs.PC = addr;
        cpu._pc_modified = true;
    }
}

void op_jp_hl(CPU& cpu) {
    cpu.regs.PC = REG_HL;
    cpu._pc_modified = true;
}

void op_jr_e(CPU& cpu) {
    int8_t offset = (int8_t)cpu._bus_read(cpu.regs.PC++);
    cpu.regs.MEMPTR = (cpu.regs.PC + offset) & 0xFFFF;
    cpu._wait(5); // JR e takes 12T (4+3+5)
    cpu.regs.PC = cpu.regs.MEMPTR;
    cpu._pc_modified = true;
}

void op_jr_cc_e(CPU& cpu) {
    int cc = (OPCODE >> 3) & 3;  // JR cc: 4->0(NZ), 5->1(Z), 6->2(NC), 7->3(C)
    int8_t offset = (int8_t)cpu._bus_read(cpu.regs.PC++);
    if (cpu.check_condition(cc)) {
        cpu.regs.MEMPTR = (cpu.regs.PC + offset) & 0xFFFF;
        cpu._wait(5); // JR cc,e takes 12T (4+3+5)
        cpu.regs.PC = cpu.regs.MEMPTR;
        cpu._pc_modified = true;
    }
}

void op_djnz_e(CPU& cpu) {
    // DJNZ: 13T when branch taken, 8T when not taken
    int8_t offset = (int8_t)cpu._bus_read(cpu.regs.PC++);
    cpu._wait(1);  // Internal operation to decrement B
    REG_B = (REG_B - 1) & 0xFF;
    if (REG_B != 0) {
        cpu.regs.MEMPTR = (cpu.regs.PC + offset) & 0xFFFF;
        cpu._wait(5);  // branch: 5T total for PC update
        cpu.regs.PC = cpu.regs.MEMPTR;
        cpu._pc_modified = true;
    }
}

// Calls
void op_call_nn(CPU& cpu) {
    uint8_t lo = cpu._bus_read(cpu.regs.PC++);
    uint8_t hi = cpu._bus_read(cpu.regs.PC++);
    uint16_t addr = lo | (hi << 8);
    cpu.regs.MEMPTR = addr;
    uint16_t ret = cpu.regs.PC;
    cpu._wait(1); // Internal execution (1T)
    cpu.regs.SP = (cpu.regs.SP - 1) & 0xFFFF;
    MEM_WR(cpu.regs.SP, ret >> 8);
    cpu.regs.SP = (cpu.regs.SP - 1) & 0xFFFF;
    MEM_WR(cpu.regs.SP, ret & 0xFF);
    cpu.regs.PC = addr;
    cpu._pc_modified = true;
}

void op_call_cc_nn(CPU& cpu) {
    int cc = (OPCODE >> 3) & 7;
    uint8_t lo = cpu._bus_read(cpu.regs.PC++);
    uint8_t hi = cpu._bus_read(cpu.regs.PC++);
    uint16_t addr = lo | (hi << 8);
    if (cpu.check_condition(cc)) {
        cpu.regs.MEMPTR = addr;
        uint16_t ret = cpu.regs.PC;
        cpu._wait(1);
        cpu.regs.SP = (cpu.regs.SP - 1) & 0xFFFF;
        MEM_WR(cpu.regs.SP, ret >> 8);
        cpu.regs.SP = (cpu.regs.SP - 1) & 0xFFFF;
        MEM_WR(cpu.regs.SP, ret & 0xFF);
        cpu.regs.PC = addr;
        cpu._pc_modified = true;
    }
}

// Returns
void op_ret(CPU& cpu) {
    uint16_t sp = cpu.regs.SP;
    uint8_t lo = MEM_RD(sp);
    uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
    cpu.regs.SP = (sp + 2) & 0xFFFF;
    cpu.regs.MEMPTR = (uint16_t)(lo | (hi << 8));
    cpu.regs.PC = cpu.regs.MEMPTR;
    cpu._pc_modified = true;
}

void op_ret_cc(CPU& cpu) {
    int cc = (OPCODE >> 3) & 7;
    cpu._wait(1); // RET cc takes 5T if not taken, 11T if taken.
    // Base M1 is 4. + 1 = 5.
    if (cpu.check_condition(cc)) {
        uint16_t sp = cpu.regs.SP;
        uint8_t lo = MEM_RD(sp);
        uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
        cpu.regs.SP = (sp + 2) & 0xFFFF;
        cpu.regs.MEMPTR = (uint16_t)(lo | (hi << 8));
        cpu.regs.PC = cpu.regs.MEMPTR;
        cpu._pc_modified = true;
    }
}

// RST
void op_rst(CPU& cpu) {
    int p = (OPCODE >> 3) & 7;
    uint16_t addr = (uint16_t)(p * 8);
    uint16_t ret = cpu.regs.PC;
    cpu.regs.MEMPTR = addr;
    cpu._wait(1);
    cpu.regs.SP = (cpu.regs.SP - 1) & 0xFFFF;
    MEM_WR(cpu.regs.SP, ret >> 8);
    cpu.regs.SP = (cpu.regs.SP - 1) & 0xFFFF;
    MEM_WR(cpu.regs.SP, ret & 0xFF);
    cpu.regs.PC = addr;
    cpu._pc_modified = true;
}

// I/O
void op_in_a_n(CPU& cpu) {
    uint8_t port = cpu._bus_read(cpu.regs.PC++);
    uint16_t addr = (uint16_t)((REG_A << 8) | port);
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    REG_A = IO_RD(addr);
}

void op_out_n_a(CPU& cpu) {
    uint8_t port = cpu._bus_read(cpu.regs.PC++);
    uint16_t addr = (uint16_t)((REG_A << 8) | port);
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    IO_WR(addr, REG_A);
}

// ============================================================
// CB handlers (rotates, shifts, bit ops)
// ============================================================
static inline void _cb_rot_r(CPU& cpu, uint8_t dest, uint8_t op_idx) {
    uint8_t val;
    switch (dest) {
        case 0: val = REG_B; break;
        case 1: val = REG_C; break;
        case 2: val = REG_D; break;
        case 3: val = REG_E; break;
        case 4: val = REG_H; break;
        case 5: val = REG_L; break;
        default: val = REG_A; break;
    }
    uint8_t old_carry = REG_F & FLAG_C;
    uint8_t result, carry;
    // RL/RR need carry-in; use special tables
    if (op_idx == 2) { // RL
        result = old_carry ? RL_CARRY_1[val] : RL_CARRY_0[val];
        carry = (val >> 7) & 1;
    } else if (op_idx == 3) { // RR
        result = old_carry ? RR_CARRY_1[val] : RR_CARRY_0[val];
        carry = val & 1;
    } else {
        result = ROT_RESULT[op_idx][val];
        carry = ROT_CARRY[op_idx][val];
    }
    switch (dest) {
        case 0: REG_B = result; break;
        case 1: REG_C = result; break;
        case 2: REG_D = result; break;
        case 3: REG_E = result; break;
        case 4: REG_H = result; break;
        case 5: REG_L = result; break;
        default: REG_A = result; break;
    }
    REG_F = result & (FLAG_S | FLAG_F3 | FLAG_F5);
    if (result == 0) REG_F |= FLAG_Z;
    if (PARITY_TABLE[result]) REG_F |= FLAG_PV;
    if (carry) REG_F |= FLAG_C;
}

void op_cb_rot(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++); // Fetch actual opcode
    cpu.current_opcode = opcode;
    uint8_t op_idx = (opcode >> 3) & 7;
    uint8_t dest = opcode & 7;
    if (dest == 6) {
        uint8_t val = MEM_RD(REG_HL);
        uint8_t old_carry = REG_F & FLAG_C;
        uint8_t result, carry;
        // RL/RR need carry-in
        if (op_idx == 2) { // RL
            result = old_carry ? RL_CARRY_1[val] : RL_CARRY_0[val];
            carry = (val >> 7) & 1;
        } else if (op_idx == 3) { // RR
            result = old_carry ? RR_CARRY_1[val] : RR_CARRY_0[val];
            carry = val & 1;
        } else {
            result = ROT_RESULT[op_idx][val];
            carry = ROT_CARRY[op_idx][val];
        }
        cpu._wait(1); // Internal execution (1T)
        MEM_WR(REG_HL, result);
        REG_F = result & (FLAG_S | FLAG_F3 | FLAG_F5);
        if (result == 0) REG_F |= FLAG_Z;
        if (PARITY_TABLE[result]) REG_F |= FLAG_PV;
        if (carry) REG_F |= FLAG_C;
        return;
    }
    _cb_rot_r(cpu, dest, op_idx);
}

void op_cb_bit(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    cpu.current_opcode = opcode;
    uint8_t bit = (opcode >> 3) & 7;
    uint8_t src = opcode & 7;
    if (src == 6) {
        uint8_t val = MEM_RD(REG_HL);
        uint8_t test = val & BIT_MASK[bit];
        cpu._wait(1); // BIT n,(HL) is 12T (4+4+3+1)
        cpu.regs.MEMPTR = REG_HL;
        REG_F = FLAG_H | (REG_F & FLAG_C);
        if (test == 0) REG_F |= FLAG_Z | FLAG_PV;
        if (bit == 7 && test) REG_F |= FLAG_S;
        REG_F |= (REG_HL >> 8) & (FLAG_F3 | FLAG_F5);
        return;
    }
    uint8_t val;
    switch (src) {
        case 0: val = REG_B; break;
        case 1: val = REG_C; break;
        case 2: val = REG_D; break;
        case 3: val = REG_E; break;
        case 4: val = REG_H; break;
        case 5: val = REG_L; break;
        default: val = REG_A; break;
    }
    uint8_t test = val & BIT_MASK[bit];
    REG_F = FLAG_H | (REG_F & FLAG_C);
    if (test == 0) REG_F |= FLAG_Z | FLAG_PV;
    if (bit == 7 && test) REG_F |= FLAG_S;
    REG_F |= val & (FLAG_F3 | FLAG_F5);
}

void op_cb_set_hl(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    cpu.current_opcode = opcode;
    uint8_t bit = (opcode >> 3) & 7;
    uint8_t val = MEM_RD(REG_HL);
    cpu._wait(1);
    MEM_WR(REG_HL, val | BIT_MASK[bit]);
}

void op_cb_res(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    cpu.current_opcode = opcode;
    uint8_t bit = (opcode >> 3) & 7;
    uint8_t dest = opcode & 7;
    if (dest == 6) {
        uint8_t val = MEM_RD(REG_HL);
        cpu._wait(1);
        MEM_WR(REG_HL, val & RES_MASK[bit]);
        return;
    }
    uint8_t val;
    switch (dest) {
        case 0: val = REG_B; break;
        case 1: val = REG_C; break;
        case 2: val = REG_D; break;
        case 3: val = REG_E; break;
        case 4: val = REG_H; break;
        case 5: val = REG_L; break;
        default: val = REG_A; break;
    }
    uint8_t result = val & RES_MASK[bit];
    switch (dest) {
        case 0: REG_B = result; break;
        case 1: REG_C = result; break;
        case 2: REG_D = result; break;
        case 3: REG_E = result; break;
        case 4: REG_H = result; break;
        case 5: REG_L = result; break;
        default: REG_A = result; break;
    }
}

void op_cb_set(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    cpu.current_opcode = opcode;
    uint8_t bit = (opcode >> 3) & 7;
    uint8_t dest = opcode & 7;
    if (dest == 6) {
        uint8_t val = MEM_RD(REG_HL);
        cpu._wait(1);
        MEM_WR(REG_HL, val | BIT_MASK[bit]);
        return;
    }
    uint8_t val;
    switch (dest) {
        case 0: val = REG_B; break;
        case 1: val = REG_C; break;
        case 2: val = REG_D; break;
        case 3: val = REG_E; break;
        case 4: val = REG_H; break;
        case 5: val = REG_L; break;
        default: val = REG_A; break;
    }
    uint8_t result = val | BIT_MASK[bit];
    switch (dest) {
        case 0: REG_B = result; break;
        case 1: REG_C = result; break;
        case 2: REG_D = result; break;
        case 3: REG_E = result; break;
        case 4: REG_H = result; break;
        case 5: REG_L = result; break;
        default: REG_A = result; break;
    }
}

void op_cb_res_hl(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    cpu.current_opcode = opcode;
    uint8_t bit = (opcode >> 3) & 7;
    uint8_t val = MEM_RD(REG_HL);
    cpu._wait(1);
    MEM_WR(REG_HL, val & RES_MASK[bit]);
}

// ============================================================
// ED handlers
// ============================================================
static inline void _ld_block_flags(CPU& cpu, uint8_t a_val, uint16_t bc_after) {
    uint8_t n = a_val & 0xFF;
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_C)) | (n & FLAG_F3);
    if (n & 0x02) REG_F |= FLAG_F5;
    if (bc_after != 0) REG_F |= FLAG_PV;
}

void op_ldi(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++); // Fetch actual opcode
    uint8_t val = MEM_RD(REG_HL);
    MEM_WR(REG_DE, val);
    cpu._wait(2); // 16T total (4+4+3+3+2)
    SET_HL((REG_HL + 1) & 0xFFFF);
    SET_DE((REG_DE + 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _ld_block_flags(cpu, REG_A + val, REG_BC);
}

void op_ldir(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++); // Fetch actual opcode
    uint8_t val = MEM_RD(REG_HL);
    MEM_WR(REG_DE, val);
    SET_HL((REG_HL + 1) & 0xFFFF);
    SET_DE((REG_DE + 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _ld_block_flags(cpu, REG_A + val, REG_BC);
    if (REG_BC != 0) {
        cpu._wait(7); // 21T (4+4+3+3+7)
        cpu.regs.PC = (cpu.regs.PC - 2) & 0xFFFF;
        cpu._pc_modified = true;
    } else {
        cpu._wait(2); // 16T
    }
}

void op_ldd(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = MEM_RD(REG_HL);
    MEM_WR(REG_DE, val);
    cpu._wait(2);
    SET_HL((REG_HL - 1) & 0xFFFF);
    SET_DE((REG_DE - 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _ld_block_flags(cpu, REG_A + val, REG_BC);
}

void op_lddr(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = MEM_RD(REG_HL);
    MEM_WR(REG_DE, val);
    SET_HL((REG_HL - 1) & 0xFFFF);
    SET_DE((REG_DE - 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _ld_block_flags(cpu, REG_A + val, REG_BC);
    if (REG_BC != 0) {
        cpu._wait(7);
        cpu.regs.PC = (cpu.regs.PC - 2) & 0xFFFF;
        cpu._pc_modified = true;
    } else {
        cpu._wait(2);
    }
}

static inline void _cpi_flags(CPU& cpu, uint8_t a, uint8_t val, uint16_t bc_after) {
    uint8_t result = (a - val) & 0xFF;
    bool hc = ((a & 0x0F) - (val & 0x0F)) < 0;
    uint8_t n = (a - val - (hc ? 1 : 0)) & 0xFF;
    REG_F = (REG_F & FLAG_C) | FLAG_N | (n & FLAG_F3);
    if (n & 0x02) REG_F |= FLAG_F5;
    if (result == 0) REG_F |= FLAG_Z;
    if (result & 0x80) REG_F |= FLAG_S;
    if (hc) REG_F |= FLAG_H;
    if (bc_after != 0) REG_F |= FLAG_PV;
}

void op_cpi(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = MEM_RD(REG_HL);
    cpu._wait(5); // 16T (4+4+3+5)
    SET_HL((REG_HL + 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _cpi_flags(cpu, REG_A, val, REG_BC);
}

void op_cpir(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = MEM_RD(REG_HL);
    uint8_t a = REG_A;
    uint8_t result = (a - val) & 0xFF;
    SET_HL((REG_HL + 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _cpi_flags(cpu, a, val, REG_BC);
    if (REG_BC != 0 && result != 0) {
        cpu._wait(10); // 21T (4+4+3+10)
        cpu.regs.PC = (cpu.regs.PC - 2) & 0xFFFF;
        cpu._pc_modified = true;
    } else {
        cpu._wait(5); // 16T
    }
}

void op_cpd(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = MEM_RD(REG_HL);
    cpu._wait(5);
    SET_HL((REG_HL - 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _cpi_flags(cpu, REG_A, val, REG_BC);
}

void op_cpdr(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = MEM_RD(REG_HL);
    uint8_t a = REG_A;
    uint8_t result = (a - val) & 0xFF;
    SET_HL((REG_HL - 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _cpi_flags(cpu, a, val, REG_BC);
    if (REG_BC != 0 && result != 0) {
        cpu._wait(10);
        cpu.regs.PC = (cpu.regs.PC - 2) & 0xFFFF;
        cpu._pc_modified = true;
    } else {
        cpu._wait(5);
    }
}

static inline void _in_out_flags(CPU& cpu, uint8_t value, uint8_t old_b, uint8_t new_b) {
    uint8_t f = REG_F & FLAG_C;
    f |= (value & (FLAG_F3 | FLAG_F5));
    if (value & 0x80) f |= FLAG_N;
    if (new_b & 0x80) f |= FLAG_S;
    if (new_b == 0) f |= FLAG_Z;
    if ((old_b & 0x0F) == 0) f |= FLAG_H;
    if (old_b == 0x80) f |= FLAG_PV;
    REG_F = f;
}

void op_ini(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(1); // 16T total
    uint8_t val = IO_RD(REG_BC);
    MEM_WR(REG_HL, val);
    uint8_t old_b = REG_B;
    REG_B = (REG_B - 1) & 0xFF;
    SET_HL((REG_HL + 1) & 0xFFFF);
    cpu.regs.MEMPTR = ((REG_B + 1) << 8) | ((REG_C + 1) & 0xFF);
    _in_out_flags(cpu, val, old_b, REG_B);
}

void op_inir(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(1);
    uint8_t val = IO_RD(REG_BC);
    MEM_WR(REG_HL, val);
    uint8_t old_b = REG_B;
    REG_B = (REG_B - 1) & 0xFF;
    SET_HL((REG_HL + 1) & 0xFFFF);
    cpu.regs.MEMPTR = ((REG_B + 1) << 8) | ((REG_C + 1) & 0xFF);
    _in_out_flags(cpu, val, old_b, REG_B);
    if (REG_B != 0) {
        cpu._wait(5); // 21T
        cpu.regs.PC = (cpu.regs.PC - 2) & 0xFFFF;
        cpu._pc_modified = true;
    }
}

void op_ind(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(1);
    uint8_t val = IO_RD(REG_BC);
    MEM_WR(REG_HL, val);
    uint8_t old_b = REG_B;
    REG_B = (REG_B - 1) & 0xFF;
    SET_HL((REG_HL - 1) & 0xFFFF);
    cpu.regs.MEMPTR = ((REG_B + 1) << 8) | ((REG_C - 1) & 0xFF);
    _in_out_flags(cpu, val, old_b, REG_B);
}

void op_indr(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(1);
    uint8_t val = IO_RD(REG_BC);
    MEM_WR(REG_HL, val);
    uint8_t old_b = REG_B;
    REG_B = (REG_B - 1) & 0xFF;
    SET_HL((REG_HL - 1) & 0xFFFF);
    cpu.regs.MEMPTR = ((REG_B + 1) << 8) | ((REG_C - 1) & 0xFF);
    _in_out_flags(cpu, val, old_b, REG_B);
    if (REG_B != 0) {
        cpu._wait(5);
        cpu.regs.PC = (cpu.regs.PC - 2) & 0xFFFF;
        cpu._pc_modified = true;
    }
}

void op_outi(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(1);
    uint8_t val = MEM_RD(REG_HL);
    uint8_t old_b = REG_B;
    REG_B = (REG_B - 1) & 0xFF;
    IO_WR(REG_BC, val);
    SET_HL((REG_HL + 1) & 0xFFFF);
    cpu.regs.MEMPTR = ((REG_B + 1) << 8) | ((REG_C + 1) & 0xFF);
    _in_out_flags(cpu, val, old_b, REG_B);
}

void op_otir(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(1);
    uint8_t val = MEM_RD(REG_HL);
    uint8_t old_b = REG_B;
    REG_B = (REG_B - 1) & 0xFF;
    IO_WR(REG_BC, val);
    SET_HL((REG_HL + 1) & 0xFFFF);
    cpu.regs.MEMPTR = ((REG_B + 1) << 8) | ((REG_C + 1) & 0xFF);
    _in_out_flags(cpu, val, old_b, REG_B);
    if (REG_B != 0) {
        cpu._wait(5);
        cpu.regs.PC = (cpu.regs.PC - 2) & 0xFFFF;
        cpu._pc_modified = true;
    }
}

void op_outd(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(1);
    uint8_t val = MEM_RD(REG_HL);
    uint8_t old_b = REG_B;
    REG_B = (REG_B - 1) & 0xFF;
    IO_WR(REG_BC, val);
    SET_HL((REG_HL - 1) & 0xFFFF);
    cpu.regs.MEMPTR = ((REG_B + 1) << 8) | ((REG_C - 1) & 0xFF);
    _in_out_flags(cpu, val, old_b, REG_B);
}

void op_otdr(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(1);
    uint8_t val = MEM_RD(REG_HL);
    uint8_t old_b = REG_B;
    REG_B = (REG_B - 1) & 0xFF;
    IO_WR(REG_BC, val);
    SET_HL((REG_HL - 1) & 0xFFFF);
    cpu.regs.MEMPTR = ((REG_B + 1) << 8) | ((REG_C - 1) & 0xFF);
    _in_out_flags(cpu, val, old_b, REG_B);
    if (REG_B != 0) {
        cpu._wait(5);
        cpu.regs.PC = (cpu.regs.PC - 2) & 0xFFFF;
        cpu._pc_modified = true;
    }
}

// 16-bit ADC/SBC
void op_adc_hl_rr(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    cpu.current_opcode = opcode;
    int pair = (opcode - 0x4A) >> 4;
    uint16_t hl = REG_HL;
    uint8_t carry = REG_F & FLAG_C;
    uint16_t src = cpu.regs.get_reg16(pair);
    cpu._wait(7); // 15T total (4+4+7)
    SET_HL((hl + src + carry) & 0xFFFF);
    REG_F = adc16_flags(hl, src, carry);
}

void op_sbc_hl_rr(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    cpu.current_opcode = opcode;
    int pair = (opcode - 0x42) >> 4;
    uint16_t hl = REG_HL;
    uint8_t carry = REG_F & FLAG_C;
    uint16_t src = cpu.regs.get_reg16(pair);
    cpu._wait(7); // 15T total (4+4+7)
    SET_HL((hl - src - carry) & 0xFFFF);
    REG_F = sbc16_flags(hl, src, carry);
}

// ED LD (nn),rr / LD rr,(nn)
void op_ld_rr_nn_ind(CPU& cpu) {
    uint8_t op2 = cpu._bus_fetch(cpu.regs.PC++);
    int pair;
    if (op2 == 0x4B) pair = 0;
    else if (op2 == 0x5B) pair = 1;
    else if (op2 == 0x6B) pair = 2;
    else pair = 3;
    uint8_t lo_pc = cpu._bus_read(cpu.regs.PC++);
    uint8_t hi_pc = cpu._bus_read(cpu.regs.PC++);
    uint16_t addr = lo_pc | (hi_pc << 8);
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    uint8_t lo = MEM_RD(addr);
    uint8_t hi = MEM_RD((addr + 1) & 0xFFFF);
    cpu.regs.set_reg16(pair, (uint16_t)(lo | (hi << 8)));
}

void op_ld_nn_rr(CPU& cpu) {
    uint8_t op2 = cpu._bus_fetch(cpu.regs.PC++);
    int pair;
    if (op2 == 0x43) pair = 0;
    else if (op2 == 0x53) pair = 1;
    else if (op2 == 0x63) pair = 2;
    else pair = 3;
    uint8_t lo_pc = cpu._bus_read(cpu.regs.PC++);
    uint8_t hi_pc = cpu._bus_read(cpu.regs.PC++);
    uint16_t addr = lo_pc | (hi_pc << 8);
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    uint16_t val = cpu.regs.get_reg16(pair);
    MEM_WR(addr, val & 0xFF);
    MEM_WR((addr + 1) & 0xFFFF, val >> 8);
}

void op_neg(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t a = REG_A;
    uint8_t result = (-a) & 0xFF;
    REG_A = result;
    REG_F = FLAG_N;
    if (result == 0) REG_F |= FLAG_Z;
    if (a != 0) REG_F |= FLAG_C;
    if ((a & 0x0F) != 0) REG_F |= FLAG_H;
    if (result == 0x80) REG_F |= FLAG_PV;
    if (result & 0x80) REG_F |= FLAG_S;
    REG_F |= (result & (FLAG_F3 | FLAG_F5));
}

void op_reti(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint16_t sp = cpu.regs.SP;
    uint8_t lo = MEM_RD(sp);
    uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
    cpu.regs.SP = (sp + 2) & 0xFFFF;
    cpu.regs.MEMPTR = (uint16_t)(lo | (hi << 8));
    cpu.regs.PC = cpu.regs.MEMPTR;
    cpu._pc_modified = true;
    cpu.regs.IFF1 = cpu.regs.IFF2;
}

void op_retn(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint16_t sp = cpu.regs.SP;
    uint8_t lo = MEM_RD(sp);
    uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
    cpu.regs.SP = (sp + 2) & 0xFFFF;
    cpu.regs.MEMPTR = (uint16_t)(lo | (hi << 8));
    cpu.regs.PC = cpu.regs.MEMPTR;
    cpu._pc_modified = true;
    cpu.regs.IFF1 = cpu.regs.IFF2;
}

void op_im(CPU& cpu) {
    uint8_t op2 = cpu._bus_fetch(cpu.regs.PC++);
    if (op2 == 0x46 || op2 == 0x66 || op2 == 0x4E || op2 == 0x6E) cpu.regs.IM = 0;
    else if (op2 == 0x56 || op2 == 0x76) cpu.regs.IM = 1;
    else if (op2 == 0x5E || op2 == 0x7E) cpu.regs.IM = 2;
}

void op_in_r_c(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    int reg = (opcode >> 3) & 7;
    uint8_t val = IO_RD(REG_BC);
    cpu.regs.MEMPTR = (REG_BC + 1) & 0xFFFF;
    if (reg != 6) {
        switch (reg) {
            case 0: REG_B = val; break;
            case 1: REG_C = val; break;
            case 2: REG_D = val; break;
            case 3: REG_E = val; break;
            case 4: REG_H = val; break;
            case 5: REG_L = val; break;
            default: REG_A = val; break;
        }
    }
    REG_F = (REG_F & FLAG_C) | (val & (FLAG_S | FLAG_F3 | FLAG_F5));
    if (val == 0) REG_F |= FLAG_Z;
    if (PARITY_TABLE[val]) REG_F |= FLAG_PV;
}

void op_out_c_r(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    int reg = (opcode >> 3) & 7;
    uint8_t val;
    switch (reg) {
        case 0: val = REG_B; break;
        case 1: val = REG_C; break;
        case 2: val = REG_D; break;
        case 3: val = REG_E; break;
        case 4: val = REG_H; break;
        case 5: val = REG_L; break;
        case 7: val = REG_A; break;
        default: val = 0; break;
    }
    cpu.regs.MEMPTR = (REG_BC + 1) & 0xFFFF;
    IO_WR(REG_BC, val);
}

void op_ld_i_a(CPU& cpu) { cpu._bus_fetch(cpu.regs.PC++); cpu._wait(1); cpu.regs.I = REG_A; }
void op_ld_r_a(CPU& cpu) { cpu._bus_fetch(cpu.regs.PC++); cpu._wait(1); cpu.regs.R = REG_A; }

void op_ld_a_i(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(1);
    uint8_t a = cpu.regs.I;
    REG_A = a;
    REG_F = (REG_F & FLAG_C) | (a & (FLAG_S | FLAG_F3 | FLAG_F5));
    if (a == 0) REG_F |= FLAG_Z;
    if (cpu.regs.IFF2) REG_F |= FLAG_PV;
    cpu._is_ld_a_ir = true;
}

void op_ld_a_r(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(1);
    uint8_t a = cpu.regs.R;
    REG_A = a;
    REG_F = (REG_F & FLAG_C) | (a & (FLAG_S | FLAG_F3 | FLAG_F5));
    if (a == 0) REG_F |= FLAG_Z;
    if (cpu.regs.IFF2) REG_F |= FLAG_PV;
    cpu._is_ld_a_ir = true;
}

void op_rld(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = MEM_RD(REG_HL);
    uint8_t low = val & 0x0F;
    uint8_t high = val & 0xF0;
    uint8_t a_low = REG_A & 0x0F;
    uint8_t new_val = (low << 4) | a_low;
    uint8_t new_a = (REG_A & 0xF0) | (high >> 4);
    cpu._wait(4); // 18T (4+4+3+4+3)
    MEM_WR(REG_HL, new_val);
    REG_A = new_a;
    REG_F = REG_F & FLAG_C;
    if (REG_A == 0) REG_F |= FLAG_Z;
    if (REG_A & 0x80) REG_F |= FLAG_S;
    if (PARITY_TABLE[REG_A]) REG_F |= FLAG_PV;
    REG_F |= REG_A & (FLAG_F3 | FLAG_F5);
}

void op_rrd(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = MEM_RD(REG_HL);
    uint8_t low = val & 0x0F;
    uint8_t high = val & 0xF0;
    uint8_t a_low = REG_A & 0x0F;
    uint8_t new_val = (a_low << 4) | (high >> 4);
    uint8_t new_a = (REG_A & 0xF0) | low;
    cpu._wait(4); // 18T (4+4+3+4+3)
    MEM_WR(REG_HL, new_val);
    REG_A = new_a;
    REG_F = REG_F & FLAG_C;
    if (REG_A == 0) REG_F |= FLAG_Z;
    if (REG_A & 0x80) REG_F |= FLAG_S;
    if (PARITY_TABLE[REG_A]) REG_F |= FLAG_PV;
    REG_F |= REG_A & (FLAG_F3 | FLAG_F5);
}

// ============================================================
// DD/FD indexed handlers
// ============================================================
static inline uint16_t _get_ix_addr(CPU& cpu) {
    uint8_t disp = cpu._bus_read(cpu.regs.PC++);
    int16_t d = (int8_t)disp;
    uint16_t base = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    uint16_t addr = (base + d) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    return addr;
}

void op_dd_fd_ld_ix_nn(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    cpu.current_opcode = opcode;
    uint8_t lo = cpu._bus_read(cpu.regs.PC++);
    uint8_t hi = cpu._bus_read(cpu.regs.PC++);
    uint16_t val = lo | (hi << 8);
    cpu.regs.MEMPTR = (val + 1) & 0xFFFF;
    if (cpu._is_iy) cpu.regs.IY = val;
    else cpu.regs.IX = val;
}

void op_dd_fd_ld_nn_ix(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    cpu.current_opcode = opcode;
    uint8_t lo_pc = cpu._bus_read(cpu.regs.PC++);
    uint8_t hi_pc = cpu._bus_read(cpu.regs.PC++);
    uint16_t addr = lo_pc | (hi_pc << 8);
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    uint16_t val = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    MEM_WR(addr, val & 0xFF);
    MEM_WR((addr + 1) & 0xFFFF, val >> 8);
}

void op_dd_fd_ld_ix_nn_ind(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    cpu.current_opcode = opcode;
    uint8_t lo = cpu._bus_read(cpu.regs.PC++);
    uint8_t hi = cpu._bus_read(cpu.regs.PC++);
    uint16_t addr = lo | (hi << 8);
    uint16_t val = MEM_RD(addr) | ((uint16_t)MEM_RD((addr + 1) & 0xFFFF) << 8);
    if (cpu._is_iy) cpu.regs.IY = val;
    else cpu.regs.IX = val;
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
}

void op_dd_fd_inc_ix(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(2);
    uint16_t ix = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    ix = (ix + 1) & 0xFFFF;
    if (cpu._is_iy) cpu.regs.IY = ix;
    else cpu.regs.IX = ix;
}

void op_dd_fd_dec_ix(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(2);
    uint16_t ix = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    ix = (ix - 1) & 0xFFFF;
    if (cpu._is_iy) cpu.regs.IY = ix;
    else cpu.regs.IX = ix;
}

void op_dd_fd_ld_sp_ix(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(2);
    uint16_t ix = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    cpu.regs.SP = ix;
}

void op_dd_fd_push_ix(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    cpu._wait(1);
    uint16_t ix = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    cpu.regs.MEMPTR = ix;
    cpu.regs.SP = (cpu.regs.SP - 1) & 0xFFFF;
    MEM_WR(cpu.regs.SP, ix >> 8);
    cpu.regs.SP = (cpu.regs.SP - 1) & 0xFFFF;
    MEM_WR(cpu.regs.SP, ix & 0xFF);
}

void op_dd_fd_pop_ix(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint16_t sp = cpu.regs.SP;
    cpu.regs.MEMPTR = (sp + 1) & 0xFFFF;
    uint16_t ix = MEM_RD(sp);
    cpu.regs.SP = (sp + 1) & 0xFFFF;
    ix |= (uint16_t)MEM_RD(cpu.regs.SP) << 8;
    cpu.regs.SP = (cpu.regs.SP + 1) & 0xFFFF;
    if (cpu._is_iy) cpu.regs.IY = ix;
    else cpu.regs.IX = ix;
}

void op_dd_fd_ex_sp_ix(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint16_t ix = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    uint16_t lo = MEM_RD(cpu.regs.SP);
    uint16_t hi = MEM_RD((cpu.regs.SP + 1) & 0xFFFF);
    uint16_t mem_val = lo | (hi << 8);
    cpu._wait(3);
    MEM_WR(cpu.regs.SP, ix & 0xFF);
    MEM_WR((cpu.regs.SP + 1) & 0xFFFF, ix >> 8);
    if (cpu._is_iy) cpu.regs.IY = mem_val;
    else cpu.regs.IX = mem_val;
    cpu.regs.MEMPTR = mem_val;
}

void op_dd_fd_add_ix_rr(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    uint16_t rr;
    switch ((opcode >> 4) & 3) {
        case 0: rr = cpu.regs.BC(); break;
        case 1: rr = cpu.regs.DE(); break;
        case 2: rr = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX; break;
        default: rr = cpu.regs.SP; break;
    }
    uint32_t result = (cpu._is_iy ? cpu.regs.IY : cpu.regs.IX) + rr;
    if (cpu._is_iy) cpu.regs.IY = result & 0xFFFF;
    else cpu.regs.IX = result & 0xFFFF;
    cpu.regs.MEMPTR = (cpu._is_iy ? cpu.regs.IY : cpu.regs.IX) + 1;
    cpu._wait(7);
    REG_F = (REG_F & ~(FLAG_C | FLAG_N | FLAG_H)) | ((result >> 16) & FLAG_C) | (((cpu._is_iy ? cpu.regs.IY : cpu.regs.IX) ^ rr ^ result) & FLAG_H);
}

void op_dd_fd_jp_ix(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint16_t target = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    cpu.regs.MEMPTR = target;
    cpu.regs.PC = target;
    cpu._pc_modified = true;
}

void op_dd_fd_ld_r_ixd(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    cpu.current_opcode = opcode;
    uint8_t dest = (opcode >> 3) & 7;
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t val = MEM_RD(addr);
    cpu._wait(5);
    switch (dest) {
        case 0: REG_B = val; break;
        case 1: REG_C = val; break;
        case 2: REG_D = val; break;
        case 3: REG_E = val; break;
        case 4: REG_H = val; break;
        case 5: REG_L = val; break;
        default: REG_A = val; break;
    }
}

void op_dd_fd_ld_ixd_r(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    cpu.current_opcode = opcode;
    uint8_t src = opcode & 7;
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t val;
    switch (src) {
        case 0: val = REG_B; break;
        case 1: val = REG_C; break;
        case 2: val = REG_D; break;
        case 3: val = REG_E; break;
        case 4: val = REG_H; break;
        case 5: val = REG_L; break;
        default: val = REG_A; break;
    }
    cpu._wait(5);
    MEM_WR(addr, val);
}

void op_dd_fd_ld_ixd_n(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);  // Consume opcode byte (4T M1)
    uint16_t addr = _get_ix_addr(cpu);
    cpu._wait(2);  // Internal delay
    uint8_t val = cpu._bus_read(cpu.regs.PC++);
    cpu._bus_write(addr, val);
}

void op_dd_fd_inc_ixd(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);  // Consume opcode byte (4T M1)
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t val = cpu._bus_read(addr);
    uint8_t nv = (val + 1) & 0xFF;
    cpu._wait(6);  // Internal for INC operation
    cpu._bus_write(addr, nv);
    REG_F = (REG_F & FLAG_C) | INC_FLAGS[val];
}

void op_dd_fd_dec_ixd(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);  // Consume opcode byte (4T M1)
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t val = cpu._bus_read(addr);
    uint8_t nv = (val - 1) & 0xFF;
    cpu._wait(6);  // Internal for DEC operation
    cpu._bus_write(addr, nv);
    REG_F = (REG_F & FLAG_C) | DEC_FLAGS[val];
}

// IXH/IYH, IXL/IYL operations
void op_dd_fd_inc_ixh(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = cpu._is_iy ? (cpu.regs.IY >> 8) & 0xFF : (cpu.regs.IX >> 8) & 0xFF;
    uint8_t nv = (val + 1) & 0xFF;
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0x00FF) | ((uint16_t)nv << 8);
    else cpu.regs.IX = (cpu.regs.IX & 0x00FF) | ((uint16_t)nv << 8);
    REG_F = (REG_F & FLAG_C) | INC_FLAGS[val];
}

void op_dd_fd_dec_ixh(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = cpu._is_iy ? (cpu.regs.IY >> 8) & 0xFF : (cpu.regs.IX >> 8) & 0xFF;
    uint8_t nv = (val - 1) & 0xFF;
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0x00FF) | ((uint16_t)nv << 8);
    else cpu.regs.IX = (cpu.regs.IX & 0x00FF) | ((uint16_t)nv << 8);
    REG_F = (REG_F & FLAG_C) | DEC_FLAGS[val];
}

void op_dd_fd_inc_ixl(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = cpu._is_iy ? cpu.regs.IY & 0xFF : cpu.regs.IX & 0xFF;
    uint8_t nv = (val + 1) & 0xFF;
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0xFF00) | nv;
    else cpu.regs.IX = (cpu.regs.IX & 0xFF00) | nv;
    REG_F = (REG_F & FLAG_C) | INC_FLAGS[val];
}

void op_dd_fd_dec_ixl(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = cpu._is_iy ? cpu.regs.IY & 0xFF : cpu.regs.IX & 0xFF;
    uint8_t nv = (val - 1) & 0xFF;
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0xFF00) | nv;
    else cpu.regs.IX = (cpu.regs.IX & 0xFF00) | nv;
    REG_F = (REG_F & FLAG_C) | DEC_FLAGS[val];
}

void op_dd_fd_ld_ixh_n(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = cpu._bus_read(cpu.regs.PC++);
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0x00FF) | ((uint16_t)val << 8);
    else cpu.regs.IX = (cpu.regs.IX & 0x00FF) | ((uint16_t)val << 8);
}

void op_dd_fd_ld_ixl_n(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val = cpu._bus_read(cpu.regs.PC++);
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0xFF00) | val;
    else cpu.regs.IX = (cpu.regs.IX & 0xFF00) | val;
}

void op_dd_fd_ld_r_ixh(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    uint8_t dest = (opcode >> 3) & 7;
    uint8_t val = cpu._is_iy ? (cpu.regs.IY >> 8) & 0xFF : (cpu.regs.IX >> 8) & 0xFF;
    switch (dest) {
        case 0: REG_B = val; break;
        case 1: REG_C = val; break;
        case 2: REG_D = val; break;
        case 3: REG_E = val; break;
        default: REG_A = val; break;
    }
}

void op_dd_fd_ld_r_ixl(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    uint8_t dest = (opcode >> 3) & 7;
    uint8_t val = cpu._is_iy ? cpu.regs.IY & 0xFF : cpu.regs.IX & 0xFF;
    switch (dest) {
        case 0: REG_B = val; break;
        case 1: REG_C = val; break;
        case 2: REG_D = val; break;
        case 3: REG_E = val; break;
        default: REG_A = val; break;
    }
}

void op_dd_fd_ld_ixh_r(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    uint8_t src = opcode & 7;
    uint8_t val;
    switch (src) {
        case 0: val = REG_B; break;
        case 1: val = REG_C; break;
        case 2: val = REG_D; break;
        case 3: val = REG_E; break;
        default: val = REG_A; break;
    }
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0x00FF) | ((uint16_t)val << 8);
    else cpu.regs.IX = (cpu.regs.IX & 0x00FF) | ((uint16_t)val << 8);
}

void op_dd_fd_ld_ixl_r(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    uint8_t src = opcode & 7;
    uint8_t val;
    switch (src) {
        case 0: val = REG_B; break;
        case 1: val = REG_C; break;
        case 2: val = REG_D; break;
        case 3: val = REG_E; break;
        default: val = REG_A; break;
    }
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0xFF00) | val;
    else cpu.regs.IX = (cpu.regs.IX & 0xFF00) | val;
}

void op_dd_fd_ld_ixh_ixh(CPU& cpu) { cpu._bus_fetch(cpu.regs.PC++); }
void op_dd_fd_ld_ixh_ixl(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val;
    if (cpu._is_iy) { val = cpu.regs.IY & 0xFF; cpu.regs.IY = (cpu.regs.IY & 0x00FF) | ((uint16_t)val << 8); }
    else { val = cpu.regs.IX & 0xFF; cpu.regs.IX = (cpu.regs.IX & 0x00FF) | ((uint16_t)val << 8); }
}
void op_dd_fd_ld_ixl_ixh(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    uint8_t val;
    if (cpu._is_iy) { val = (cpu.regs.IY >> 8) & 0xFF; cpu.regs.IY = (cpu.regs.IY & 0xFF00) | val; }
    else { val = (cpu.regs.IX >> 8) & 0xFF; cpu.regs.IX = (cpu.regs.IX & 0xFF00) | val; }
}
void op_dd_fd_ld_ixl_ixl(CPU& cpu) { cpu._bus_fetch(cpu.regs.PC++); }

void op_dd_fd_ld_a_ixh(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    REG_A = cpu._is_iy ? (cpu.regs.IY >> 8) & 0xFF : (cpu.regs.IX >> 8) & 0xFF;
}
void op_dd_fd_ld_a_ixl(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);
    REG_A = cpu._is_iy ? cpu.regs.IY & 0xFF : cpu.regs.IX & 0xFF;
}

// ALU with IXH/IXL
void op_dd_fd_alu_ixh(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    uint8_t alu_op = (opcode >> 3) & 7;
    uint8_t b = cpu._is_iy ? (cpu.regs.IY >> 8) & 0xFF : (cpu.regs.IX >> 8) & 0xFF;
    uint8_t a = REG_A;
    switch (alu_op) {
        case 0: REG_A = (a + b) & 0xFF; REG_F = ADD_FLAGS[(a << 8) | b]; break;
        case 1: { uint8_t c = REG_F & FLAG_C; REG_A = (a + b + c) & 0xFF; REG_F = (c ? ADC_FLAGS : ADD_FLAGS)[(a << 8) | b]; break; }
        case 2: REG_A = (a - b) & 0xFF; REG_F = SUB_FLAGS[(a << 8) | b]; break;
        case 3: { uint8_t c = REG_F & FLAG_C; REG_A = (a - b - c) & 0xFF; REG_F = (c ? SBC_FLAGS : SUB_FLAGS)[(a << 8) | b]; break; }
        case 4: REG_A &= b; REG_F = SZHZP_TABLE[REG_A]; break;
        case 5: REG_A ^= b; REG_F = SZ53P_TABLE[REG_A]; break;
        case 6: REG_A |= b; REG_F = SZ53P_TABLE[REG_A]; break;
        case 7: REG_F = (SUB_FLAGS[(a << 8) | b] & ~(FLAG_F3 | FLAG_F5)) | (b & (FLAG_F3 | FLAG_F5)); break;
    }
}

void op_dd_fd_alu_ixl(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    uint8_t alu_op = (opcode >> 3) & 7;
    uint8_t b = cpu._is_iy ? cpu.regs.IY & 0xFF : cpu.regs.IX & 0xFF;
    uint8_t a = REG_A;
    switch (alu_op) {
        case 0: REG_A = (a + b) & 0xFF; REG_F = ADD_FLAGS[(a << 8) | b]; break;
        case 1: { uint8_t c = REG_F & FLAG_C; REG_A = (a + b + c) & 0xFF; REG_F = (c ? ADC_FLAGS : ADD_FLAGS)[(a << 8) | b]; break; }
        case 2: REG_A = (a - b) & 0xFF; REG_F = SUB_FLAGS[(a << 8) | b]; break;
        case 3: { uint8_t c = REG_F & FLAG_C; REG_A = (a - b - c) & 0xFF; REG_F = (c ? SBC_FLAGS : SUB_FLAGS)[(a << 8) | b]; break; }
        case 4: REG_A &= b; REG_F = SZHZP_TABLE[REG_A]; break;
        case 5: REG_A ^= b; REG_F = SZ53P_TABLE[REG_A]; break;
        case 6: REG_A |= b; REG_F = SZ53P_TABLE[REG_A]; break;
        case 7: REG_F = (SUB_FLAGS[(a << 8) | b] & ~(FLAG_F3 | FLAG_F5)) | (b & (FLAG_F3 | FLAG_F5)); break;
    }
}

void op_dd_fd_alu_ixd(CPU& cpu) {
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);
    cpu.current_opcode = opcode;
    uint8_t alu_op = (opcode >> 3) & 7;
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t b = MEM_RD(addr);
    uint8_t a = REG_A;
    cpu._wait(5);
    switch (alu_op) {
        case 0: REG_A = (a + b) & 0xFF; REG_F = ADD_FLAGS[(a << 8) | b]; break;
        case 1: { uint8_t c = REG_F & FLAG_C; REG_A = (a + b + c) & 0xFF; REG_F = (c ? ADC_FLAGS : ADD_FLAGS)[(a << 8) | b]; break; }
        case 2: REG_A = (a - b) & 0xFF; REG_F = SUB_FLAGS[(a << 8) | b]; break;
        case 3: { uint8_t c = REG_F & FLAG_C; REG_A = (a - b - c) & 0xFF; REG_F = (c ? SBC_FLAGS : SUB_FLAGS)[(a << 8) | b]; break; }
        case 4: REG_A &= b; REG_F = SZHZP_TABLE[REG_A]; break;
        case 5: REG_A ^= b; REG_F = SZ53P_TABLE[REG_A]; break;
        case 6: REG_A |= b; REG_F = SZ53P_TABLE[REG_A]; break;
        case 7: REG_F = (SUB_FLAGS[(a << 8) | b] & ~(FLAG_F3 | FLAG_F5)) | (b & (FLAG_F3 | FLAG_F5)); break;
    }
}

// ADC/SBC IX/IY,rr
void op_dd_fd_adc_ix_rr(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);  // Consume ED byte (4T M1)
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);  // Consume actual opcode (4T M1)
    cpu.current_opcode = opcode;
    int pair = (opcode - 0x4A) >> 4;
    uint16_t ix = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    uint8_t carry = REG_F & FLAG_C;
    uint16_t src = (pair == 2) ? ix : cpu.regs.get_reg16(pair);
    cpu._wait(3); // 15T total: 4(DD) + 4(ED) + 4(opcode) + 3(internal)
    uint16_t result = (ix + src + carry) & 0xFFFF;
    if (cpu._is_iy) cpu.regs.IY = result;
    else cpu.regs.IX = result;
    REG_F = adc16_flags(ix, src, carry);
}

void op_dd_fd_sbc_ix_rr(CPU& cpu) {
    cpu._bus_fetch(cpu.regs.PC++);  // Consume ED byte (4T M1)
    uint8_t opcode = cpu._bus_fetch(cpu.regs.PC++);  // Consume actual opcode (4T M1)
    cpu.current_opcode = opcode;
    int pair = (opcode - 0x42) >> 4;
    uint16_t ix = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    uint8_t carry = REG_F & FLAG_C;
    uint16_t src = (pair == 2) ? ix : cpu.regs.get_reg16(pair);
    cpu._wait(3); // 15T total: 4(DD) + 4(ED) + 4(opcode) + 3(internal)
    uint16_t result = (ix - src - carry) & 0xFFFF;
    if (cpu._is_iy) cpu.regs.IY = result;
    else cpu.regs.IX = result;
    REG_F = sbc16_flags(ix, src, carry);
}

// ============================================================
// DDCB/FDCB handlers
// ============================================================
void op_ddcb_fdcb_rot(CPU& cpu) {
    // DDCB/FDCB ROTATE: 23T = 4(DD) + 3(CB) + 3(displacement) + 3(opcode) + 4(internal) + 3(read) + 3(write)
    cpu._bus_read(cpu.regs.PC++);  // skip CB prefix, 3T
    uint8_t disp = cpu._bus_read(cpu.regs.PC++);  // displacement, 3T
    uint8_t opcode = cpu._bus_read(cpu.regs.PC++);  // actual opcode, 3T
    cpu.current_opcode = opcode;
    uint8_t op_idx = (opcode >> 3) & 7;
    uint8_t dest = opcode & 7;
    int16_t d = (int8_t)disp;
    uint16_t base = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    uint16_t addr = (base + d) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    cpu._wait(4);  // Internal operations, 4T
    uint8_t val = cpu._bus_read(addr);  // Read memory, 3T
    uint8_t old_carry = REG_F & FLAG_C;
    uint8_t result, carry;
    if (op_idx == 2) { // RL
        result = old_carry ? RL_CARRY_1[val] : RL_CARRY_0[val];
        carry = (val >> 7) & 1;
    } else if (op_idx == 3) { // RR
        result = old_carry ? RR_CARRY_1[val] : RR_CARRY_0[val];
        carry = val & 1;
    } else {
        result = ROT_RESULT[op_idx][val];
        carry = ROT_CARRY[op_idx][val];
    }
    cpu._bus_write(addr, result);  // Write memory, 3T
    if (dest != 6) {
        switch (dest) {
            case 0: REG_B = result; break;
            case 1: REG_C = result; break;
            case 2: REG_D = result; break;
            case 3: REG_E = result; break;
            case 4: REG_H = result; break;
            case 5: REG_L = result; break;
            default: REG_A = result; break;
        }
    }
    REG_F = result & (FLAG_S | FLAG_F3 | FLAG_F5);
    if (result == 0) REG_F |= FLAG_Z;
    if (PARITY_TABLE[result]) REG_F |= FLAG_PV;
    if (carry) REG_F |= FLAG_C;
}

void op_ddcb_fdcb_bit(CPU& cpu) {
    // DDCB/FDCB BIT: 20T = 4(DD) + 3(CB) + 3(displacement) + 3(opcode) + 3(read) + 4(internal)
    cpu._bus_read(cpu.regs.PC++);  // skip CB prefix, 3T
    uint8_t disp = cpu._bus_read(cpu.regs.PC++);  // displacement, 3T
    uint8_t opcode = cpu._bus_read(cpu.regs.PC++);  // actual opcode, 3T
    cpu.current_opcode = opcode;
    uint8_t bit = (opcode >> 3) & 7;
    int16_t d = (int8_t)disp;
    uint16_t base = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    uint16_t addr = (base + d) & 0xFFFF;
    uint8_t val = cpu._bus_read(addr);  // Read memory, 3T
    uint8_t test = val & BIT_MASK[bit];
    cpu._wait(4);  // Internal for BIT, 4T
    REG_F = FLAG_H | (REG_F & FLAG_C);
    if (test == 0) REG_F |= FLAG_Z | FLAG_PV;
    if (bit == 7 && test) REG_F |= FLAG_S;
    REG_F |= (addr >> 8) & (FLAG_F3 | FLAG_F5);
}

void op_ddcb_fdcb_res(CPU& cpu) {
    // DDCB/FDCB RES: 23T = 4(DD) + 3(CB) + 3(displacement) + 3(opcode) + 4(internal) + 3(read) + 3(write)
    cpu._bus_read(cpu.regs.PC++);  // skip CB prefix, 3T
    uint8_t disp = cpu._bus_read(cpu.regs.PC++);  // displacement, 3T
    uint8_t opcode = cpu._bus_read(cpu.regs.PC++);  // actual opcode, 3T
    cpu.current_opcode = opcode;
    uint8_t bit = (opcode >> 3) & 7;
    uint8_t dest = opcode & 7;
    int16_t d = (int8_t)disp;
    uint16_t base = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    uint16_t addr = (base + d) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    cpu._wait(4);  // Internal operations, 4T
    uint8_t val = cpu._bus_read(addr);  // Read memory, 3T
    uint8_t result = val & RES_MASK[bit];
    cpu._bus_write(addr, result);  // Write memory, 3T
    if (dest != 6) {
        switch (dest) {
            case 0: REG_B = result; break;
            case 1: REG_C = result; break;
            case 2: REG_D = result; break;
            case 3: REG_E = result; break;
            case 4: REG_H = result; break;
            case 5: REG_L = result; break;
            default: REG_A = result; break;
        }
    }
}

void op_ddcb_fdcb_set(CPU& cpu) {
    // DDCB/FDCB SET: 23T = 4(DD) + 3(CB) + 3(displacement) + 3(opcode) + 4(internal) + 3(read) + 3(write)
    cpu._bus_read(cpu.regs.PC++);  // skip CB prefix, 3T
    uint8_t disp = cpu._bus_read(cpu.regs.PC++);  // displacement, 3T
    uint8_t opcode = cpu._bus_read(cpu.regs.PC++);  // actual opcode, 3T
    cpu.current_opcode = opcode;
    
    // Calculate address
    int16_t d = (int8_t)disp;
    uint16_t base = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    uint16_t addr = (base + d) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    
    cpu._wait(4);  // Internal operations: 4T
    uint8_t val = cpu._bus_read(addr);  // Read memory, 3T
    uint8_t bit = (opcode >> 3) & 7;
    uint8_t dest = opcode & 7;
    uint8_t result = val | BIT_MASK[bit];
    cpu._bus_write(addr, result);  // Write memory, 3T
    
    if (dest != 6) {
        switch (dest) {
            case 0: REG_B = result; break;
            case 1: REG_C = result; break;
            case 2: REG_D = result; break;
            case 3: REG_E = result; break;
            case 4: REG_H = result; break;
            case 5: REG_L = result; break;
            default: REG_A = result; break;
        }
    }
}

