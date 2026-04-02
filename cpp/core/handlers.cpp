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
#define MEM_RD(a) cpu._bus_read(a, CYCLES)
#define MEM_WR(a,v) cpu._bus_write(a, v, CYCLES)
#define MEM_WR_D(a,v) cpu._bus_write_direct(a, v, CYCLES)
#define IO_RD(p) cpu._bus_io_read(p, CYCLES)
#define IO_WR(p,v) cpu._bus_io_write(p, v, CYCLES)
#define READ_PC(off) (cpu._is_simple_bus ? MEM[(cpu.regs.PC + (off)) & 0xFFFF] : cpu._bus_read((cpu.regs.PC + (off)) & 0xFFFF, CYCLES))

// ============================================================
// NOP handlers for unknown opcodes
// ============================================================
static int op_nop_base(CPU& cpu) { return 4; }
static int op_nop_cb(CPU& cpu) { return 8; }
static int op_nop_ed(CPU& cpu) { return 8; }
static int op_nop_dd(CPU& cpu) { return 4; }
static int op_nop_fd(CPU& cpu) { return 4; }
static int op_nop_ddcb(CPU& cpu) { return 23; }
static int op_nop_fdcb(CPU& cpu) { return 23; }

// ============================================================
// Base instruction handlers
// ============================================================
int op_nop(CPU& cpu) { return 4; }

int op_halt(CPU& cpu) {
    cpu.halted = true;
    cpu._pc_modified = true;
    return 4;
}

int op_di(CPU& cpu) {
    cpu.regs.IFF1 = cpu.regs.IFF2 = false;
    cpu.regs.EI_PENDING = false;
    return 4;
}

int op_ei(CPU& cpu) {
    cpu.regs.EI_PENDING = true;
    cpu.regs.EI_JUST_RESOLVED = false;
    return 4;
}

int op_ld_r_r(CPU& cpu) {
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
    return (dest == 6 || src == 6) ? 7 : 4;
}

int op_ld_r_n(CPU& cpu) {
    uint8_t dest = (OPCODE >> 3) & 7;
    uint8_t val = READ_PC(1);
    switch (dest) {
        case 0: REG_B = val; break;
        case 1: REG_C = val; break;
        case 2: REG_D = val; break;
        case 3: REG_E = val; break;
        case 4: REG_H = val; break;
        case 5: REG_L = val; break;
        case 6: MEM_WR(REG_HL, val); return 10;
        default: REG_A = val; break;
    }
    return 7;
}

int op_ld_r_hl(CPU& cpu) {
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
    return 7;
}

int op_ld_hl_r(CPU& cpu) {
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
    return 7;
}

int op_ld_hl_n(CPU& cpu) {
    uint8_t val = READ_PC(1);
    MEM_WR(REG_HL, val);
    return 10;
}

int op_ld_a_bc(CPU& cpu) {
    uint16_t bc = REG_BC;
    REG_A = MEM_RD(bc);
    cpu.regs.MEMPTR = (bc + 1) & 0xFFFF;
    return 7;
}

int op_ld_a_de(CPU& cpu) {
    uint16_t de = REG_DE;
    REG_A = MEM_RD(de);
    cpu.regs.MEMPTR = (de + 1) & 0xFFFF;
    return 7;
}

int op_ld_a_nn(CPU& cpu) {
    uint16_t addr = READ_PC(1) | ((uint16_t)READ_PC(2) << 8);
    REG_A = MEM_RD(addr);
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
    return 13;
}

int op_ld_bc_a(CPU& cpu) {
    uint16_t bc = REG_BC;
    uint8_t a = REG_A;
    MEM_WR(bc, a);
    cpu.regs.MEMPTR = ((a << 8) | ((bc + 1) & 0xFF)) & 0xFFFF;
    return 7;
}

int op_ld_de_a(CPU& cpu) {
    uint16_t de = REG_DE;
    uint8_t a = REG_A;
    MEM_WR(de, a);
    cpu.regs.MEMPTR = ((a << 8) | ((de + 1) & 0xFF)) & 0xFFFF;
    return 7;
}

int op_ld_nn_a(CPU& cpu) {
    uint16_t addr = READ_PC(1) | ((uint16_t)READ_PC(2) << 8);
    uint8_t a = REG_A;
    MEM_WR(addr, a);
    cpu.regs.MEMPTR = ((a << 8) | ((addr + 1) & 0xFF)) & 0xFFFF;
    return 13;
}

int op_ld_rr_nn(CPU& cpu) {
    int pair = (OPCODE >> 4) & 3;
    uint16_t val = READ_PC(1) | ((uint16_t)READ_PC(2) << 8);
    cpu.regs.set_reg16(pair, val);
    return 10;
}

int op_ld_hl_nn(CPU& cpu) {
    uint16_t addr = READ_PC(1) | ((uint16_t)READ_PC(2) << 8);
    uint8_t lo = MEM_RD(addr);
    uint8_t hi = MEM_RD((addr + 1) & 0xFFFF);
    SET_HL((uint16_t)(lo | (hi << 8)));
    return 16;
}

int op_ld_nn_hl(CPU& cpu) {
    uint16_t addr = READ_PC(1) | ((uint16_t)READ_PC(2) << 8);
    MEM_WR(addr, REG_L);
    MEM_WR((addr + 1) & 0xFFFF, REG_H);
    return 16;
}

int op_ld_sp_hl(CPU& cpu) {
    cpu.regs.SP = REG_HL;
    return 6;
}

int op_push_rr(CPU& cpu) {
    int pair = (OPCODE >> 4) & 3;
    uint16_t val = cpu.regs.get_reg16_push(pair);
    cpu.regs.SP = (cpu.regs.SP - 2) & 0xFFFF;
    MEM_WR_D(cpu.regs.SP, val & 0xFF);
    MEM_WR_D((cpu.regs.SP + 1) & 0xFFFF, val >> 8);
    return 11;
}

int op_pop_rr(CPU& cpu) {
    int pair = (OPCODE >> 4) & 3;
    uint16_t sp = cpu.regs.SP;
    uint8_t lo = MEM_RD(sp);
    uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
    cpu.regs.SP = (sp + 2) & 0xFFFF;
    cpu.regs.set_reg16_push(pair, (uint16_t)(lo | (hi << 8)));
    return 10;
}

int op_ex_de_hl(CPU& cpu) {
    uint16_t t = REG_DE;
    SET_DE(REG_HL);
    SET_HL(t);
    return 4;
}

int op_ex_af_afp(CPU& cpu) {
    cpu.regs.swap_shadow();
    return 4;
}

int op_exx(CPU& cpu) {
    cpu.regs.swap_shadow_all();
    return 4;
}

int op_ex_sp_hl(CPU& cpu) {
    uint16_t sp = cpu.regs.SP;
    uint8_t lo = MEM_RD(sp);
    uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
    uint16_t temp = (uint16_t)(lo | (hi << 8));
    MEM_WR_D(sp, REG_L);
    MEM_WR_D((sp + 1) & 0xFFFF, REG_H);
    SET_HL(temp);
    return 19;
}

// --- 8-bit ALU ---
static inline int _alu_add(CPU& cpu, uint8_t b) {
    uint8_t a = REG_A;
    REG_A = (a + b) & 0xFF;
    REG_F = ADD_FLAGS[(a << 8) | b];
    return 4;
}

static inline int _alu_add_hl(CPU& cpu) {
    uint8_t a = REG_A;
    uint8_t b = MEM_RD(REG_HL);
    REG_A = (a + b) & 0xFF;
    REG_F = ADD_FLAGS[(a << 8) | b];
    return 7;
}

int op_add_a(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) return _alu_add_hl(cpu);
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
    return _alu_add(cpu, b);
}

int op_add_a_n(CPU& cpu) {
    uint8_t b = READ_PC(1);
    uint8_t a = REG_A;
    REG_A = (a + b) & 0xFF;
    REG_F = ADD_FLAGS[(a << 8) | b];
    return 7;
}

static inline int _alu_adc(CPU& cpu, uint8_t b) {
    uint8_t a = REG_A;
    uint8_t c = REG_F & FLAG_C;
    REG_A = (a + b + c) & 0xFF;
    REG_F = (c ? ADC_FLAGS : ADD_FLAGS)[(a << 8) | b];
    return 4;
}

int op_adc_a(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t b = MEM_RD(REG_HL);
        uint8_t a = REG_A;
        uint8_t c = REG_F & FLAG_C;
        REG_A = (a + b + c) & 0xFF;
        REG_F = (c ? ADC_FLAGS : ADD_FLAGS)[(a << 8) | b];
        return 7;
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
    return _alu_adc(cpu, b);
}

int op_adc_a_n(CPU& cpu) {
    uint8_t b = READ_PC(1);
    uint8_t a = REG_A;
    uint8_t c = REG_F & FLAG_C;
    REG_A = (a + b + c) & 0xFF;
    REG_F = (c ? ADC_FLAGS : ADD_FLAGS)[(a << 8) | b];
    return 7;
}

static inline int _alu_sub(CPU& cpu, uint8_t b) {
    uint8_t a = REG_A;
    REG_A = (a - b) & 0xFF;
    REG_F = SUB_FLAGS[(a << 8) | b];
    return 4;
}

int op_sub(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t b = MEM_RD(REG_HL);
        uint8_t a = REG_A;
        REG_A = (a - b) & 0xFF;
        REG_F = SUB_FLAGS[(a << 8) | b];
        return 7;
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
    return _alu_sub(cpu, b);
}

int op_sub_n(CPU& cpu) {
    uint8_t b = READ_PC(1);
    uint8_t a = REG_A;
    REG_A = (a - b) & 0xFF;
    REG_F = SUB_FLAGS[(a << 8) | b];
    return 7;
}

static inline int _alu_sbc(CPU& cpu, uint8_t b) {
    uint8_t a = REG_A;
    uint8_t c = REG_F & FLAG_C;
    REG_A = (a - b - c) & 0xFF;
    REG_F = (c ? SBC_FLAGS : SUB_FLAGS)[(a << 8) | b];
    return 4;
}

int op_sbc_a(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t b = MEM_RD(REG_HL);
        uint8_t a = REG_A;
        uint8_t c = REG_F & FLAG_C;
        REG_A = (a - b - c) & 0xFF;
        REG_F = (c ? SBC_FLAGS : SUB_FLAGS)[(a << 8) | b];
        return 7;
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
    return _alu_sbc(cpu, b);
}

int op_sbc_a_n(CPU& cpu) {
    uint8_t b = READ_PC(1);
    uint8_t a = REG_A;
    uint8_t c = REG_F & FLAG_C;
    REG_A = (a - b - c) & 0xFF;
    REG_F = (c ? SBC_FLAGS : SUB_FLAGS)[(a << 8) | b];
    return 7;
}

static inline int _alu_and(CPU& cpu, uint8_t b) {
    REG_A &= b;
    REG_F = SZHZP_TABLE[REG_A];
    return 4;
}

int op_and(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t b = MEM_RD(REG_HL);
        REG_A &= b;
        REG_F = SZHZP_TABLE[REG_A];
        return 7;
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
    return _alu_and(cpu, b);
}

int op_and_n(CPU& cpu) {
    REG_A &= READ_PC(1);
    REG_F = SZHZP_TABLE[REG_A];
    return 7;
}

static inline int _alu_or(CPU& cpu, uint8_t b) {
    REG_A |= b;
    REG_F = SZ53P_TABLE[REG_A];
    return 4;
}

int op_or(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t b = MEM_RD(REG_HL);
        REG_A |= b;
        REG_F = SZ53P_TABLE[REG_A];
        return 7;
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
    return _alu_or(cpu, b);
}

int op_or_n(CPU& cpu) {
    REG_A |= READ_PC(1);
    REG_F = SZ53P_TABLE[REG_A];
    return 7;
}

static inline int _alu_xor(CPU& cpu, uint8_t b) {
    REG_A ^= b;
    REG_F = SZ53P_TABLE[REG_A];
    return 4;
}

int op_xor(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t b = MEM_RD(REG_HL);
        REG_A ^= b;
        REG_F = SZ53P_TABLE[REG_A];
        return 7;
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
    return _alu_xor(cpu, b);
}

int op_xor_n(CPU& cpu) {
    REG_A ^= READ_PC(1);
    REG_F = SZ53P_TABLE[REG_A];
    return 7;
}

static inline int _alu_cp(CPU& cpu, uint8_t b) {
    uint8_t a = REG_A;
    REG_F = (SUB_FLAGS[(a << 8) | b] & ~(FLAG_F3 | FLAG_F5)) | (b & (FLAG_F3 | FLAG_F5));
    return 4;
}

int op_cp(CPU& cpu) {
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t b = MEM_RD(REG_HL);
        uint8_t a = REG_A;
        REG_F = (SUB_FLAGS[(a << 8) | b] & ~(FLAG_F3 | FLAG_F5)) | (b & (FLAG_F3 | FLAG_F5));
        return 7;
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
    return _alu_cp(cpu, b);
}

int op_cp_n(CPU& cpu) {
    uint8_t b = READ_PC(1);
    uint8_t a = REG_A;
    REG_F = (SUB_FLAGS[(a << 8) | b] & ~(FLAG_F3 | FLAG_F5)) | (b & (FLAG_F3 | FLAG_F5));
    return 7;
}

// INC/DEC
int op_inc_r(CPU& cpu) {
    int dest = (OPCODE >> 3) & 7;
    if (dest == 6) {
        uint8_t val = MEM_RD(REG_HL);
        uint8_t nv = (val + 1) & 0xFF;
        MEM_WR(REG_HL, nv);
        REG_F = (REG_F & FLAG_C) | INC_FLAGS[val];
        return 11;
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
    return 4;
}

int op_dec_r(CPU& cpu) {
    int dest = (OPCODE >> 3) & 7;
    if (dest == 6) {
        uint8_t val = MEM_RD(REG_HL);
        uint8_t nv = (val - 1) & 0xFF;
        MEM_WR(REG_HL, nv);
        REG_F = (REG_F & FLAG_C) | DEC_FLAGS[val];
        return 11;
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
    return 4;
}

// 16-bit ALU
int op_add_hl_rr(CPU& cpu) {
    int pair = (OPCODE >> 4) & 3;
    uint16_t hl = REG_HL;
    uint16_t op = cpu.regs.get_reg16(pair);
    SET_HL((hl + op) & 0xFFFF);
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_PV)) | add16_flags(hl, op, REG_F);
    return 11;
}

int op_inc_rr(CPU& cpu) {
    int pair = (OPCODE >> 4) & 3;
    cpu.regs.set_reg16(pair, (cpu.regs.get_reg16(pair) + 1) & 0xFFFF);
    return 6;
}

int op_dec_rr(CPU& cpu) {
    int pair = (OPCODE >> 4) & 3;
    cpu.regs.set_reg16(pair, (cpu.regs.get_reg16(pair) - 1) & 0xFFFF);
    return 6;
}

// Rotates
int op_rlca(CPU& cpu) {
    uint8_t a = REG_A;
    uint8_t c = (a >> 7) & 1;
    REG_A = ((a << 1) | c) & 0xFF;
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_PV)) | c | (REG_A & (FLAG_F3 | FLAG_F5));
    return 4;
}

int op_rrca(CPU& cpu) {
    uint8_t a = REG_A;
    uint8_t c = a & 1;
    REG_A = ((a >> 1) | (c << 7)) & 0xFF;
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_PV)) | c | (REG_A & (FLAG_F3 | FLAG_F5));
    return 4;
}

int op_rla(CPU& cpu) {
    uint8_t a = REG_A;
    uint8_t oc = REG_F & FLAG_C;
    uint8_t nc = (a >> 7) & 1;
    REG_A = ((a << 1) | oc) & 0xFF;
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_PV)) | nc | (REG_A & (FLAG_F3 | FLAG_F5));
    return 4;
}

int op_rra(CPU& cpu) {
    uint8_t a = REG_A;
    uint8_t oc = REG_F & FLAG_C;
    uint8_t nc = a & 1;
    REG_A = ((a >> 1) | (oc << 7)) & 0xFF;
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_PV)) | nc | (REG_A & (FLAG_F3 | FLAG_F5));
    return 4;
}

// DAA
int op_daa(CPU& cpu) {
    int idx = ((REG_F & FLAG_N) << 9) | ((REG_F & FLAG_H) << 8) | ((REG_F & FLAG_C) << 7) | REG_A;
    idx >>= 1; // (N<<10)|(H<<9)|(C<<8)|A, but N is bit 1, H is bit 4, C is bit 0
    // Correct: index = (N_bit << 10) | (H_bit << 9) | (C_bit << 8) | A
    int n = (REG_F >> 1) & 1;
    int h = (REG_F >> 4) & 1;
    int c = REG_F & 1;
    idx = (n << 10) | (h << 9) | (c << 8) | REG_A;
    REG_A = DAA_FULL_FLAGS[idx * 2];
    REG_F = DAA_FULL_FLAGS[idx * 2 + 1];
    cpu.regs.Q = REG_F;
    return 4;
}

// CPL
int op_cpl(CPU& cpu) {
    REG_A = (~REG_A) & 0xFF;
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_PV | FLAG_C)) | FLAG_H | FLAG_N | (REG_A & (FLAG_F3 | FLAG_F5));
    cpu.regs.Q = REG_F;
    return 4;
}

// CCF
int op_ccf(CPU& cpu) {
    uint8_t old_f = REG_F;
    uint8_t old_c = old_f & FLAG_C;
    uint8_t f = old_f & (FLAG_S | FLAG_Z | FLAG_PV);
    uint8_t result = (cpu.regs.LAST_Q ^ old_f) | REG_A;
    f |= result & (FLAG_F3 | FLAG_F5);
    if (old_c) f |= FLAG_H;
    else f |= FLAG_C;
    REG_F = f;
    cpu.regs.Q = f;
    return 4;
}

// SCF
int op_scf(CPU& cpu) {
    uint8_t old_f = REG_F;
    uint8_t f = (old_f & (FLAG_S | FLAG_Z | FLAG_PV)) | FLAG_C;
    uint8_t result = (cpu.regs.LAST_Q ^ old_f) | REG_A;
    f |= result & (FLAG_F3 | FLAG_F5);
    REG_F = f;
    cpu.regs.Q = f;
    return 4;
}

// Jumps
int op_jp_nn(CPU& cpu) {
    uint16_t addr = READ_PC(1) | ((uint16_t)READ_PC(2) << 8);
    cpu.regs.PC = addr;
    cpu._pc_modified = true;
    return 10;
}

int op_jp_cc_nn(CPU& cpu) {
    int cc = (OPCODE >> 3) & 7;
    if (cpu.check_condition(cc)) {
        uint16_t addr = READ_PC(1) | ((uint16_t)READ_PC(2) << 8);
        cpu.regs.PC = addr;
        cpu._pc_modified = true;
    }
    return 10;
}

int op_jp_hl(CPU& cpu) {
    cpu.regs.PC = REG_HL;
    cpu._pc_modified = true;
    return 4;
}

int op_jr_e(CPU& cpu) {
    int8_t offset = (int8_t)READ_PC(1);
    cpu.regs.PC = (cpu.regs.PC + 2 + offset) & 0xFFFF;
    cpu._pc_modified = true;
    return 12;
}

int op_jr_cc_e(CPU& cpu) {
    int cc = (OPCODE >> 3) & 3;  // JR cc: 4->0(NZ), 5->1(Z), 6->2(NC), 7->3(C)
    if (cpu.check_condition(cc)) {
        int8_t offset = (int8_t)READ_PC(1);
        cpu.regs.PC = (cpu.regs.PC + 2 + offset) & 0xFFFF;
        cpu._pc_modified = true;
        return 12;
    }
    return 7;
}

int op_djnz_e(CPU& cpu) {
    REG_B = (REG_B - 1) & 0xFF;
    if (REG_B != 0) {
        int8_t offset = (int8_t)READ_PC(1);
        cpu.regs.PC = (cpu.regs.PC + 2 + offset) & 0xFFFF;
        cpu._pc_modified = true;
        return 13;
    }
    return 8;
}

// Calls
int op_call_nn(CPU& cpu) {
    uint16_t addr = READ_PC(1) | ((uint16_t)READ_PC(2) << 8);
    uint16_t ret = (cpu.regs.PC + 3) & 0xFFFF;
    cpu.regs.SP = (cpu.regs.SP - 2) & 0xFFFF;
    MEM_WR_D(cpu.regs.SP, ret & 0xFF);
    MEM_WR_D((cpu.regs.SP + 1) & 0xFFFF, ret >> 8);
    cpu.regs.PC = addr;
    cpu._pc_modified = true;
    return 17;
}

int op_call_cc_nn(CPU& cpu) {
    int cc = (OPCODE >> 3) & 7;
    if (cpu.check_condition(cc)) return op_call_nn(cpu);
    return 10;
}

// Returns
int op_ret(CPU& cpu) {
    uint16_t sp = cpu.regs.SP;
    uint8_t lo = MEM_RD(sp);
    uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
    cpu.regs.SP = (sp + 2) & 0xFFFF;
    cpu.regs.PC = (uint16_t)(lo | (hi << 8));
    cpu._pc_modified = true;
    return 10;
}

int op_ret_cc(CPU& cpu) {
    int cc = (OPCODE >> 3) & 7;
    if (cpu.check_condition(cc)) {
        uint16_t sp = cpu.regs.SP;
        uint8_t lo = MEM_RD(sp);
        uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
        cpu.regs.SP = (sp + 2) & 0xFFFF;
        cpu.regs.PC = (uint16_t)(lo | (hi << 8));
        cpu._pc_modified = true;
        return 11;
    }
    return 5;
}

// RST
int op_rst(CPU& cpu) {
    int p = (OPCODE >> 3) & 7;
    uint16_t addr = (uint16_t)(p * 8);
    uint16_t ret = (cpu.regs.PC + 1) & 0xFFFF;
    cpu.regs.SP = (cpu.regs.SP - 2) & 0xFFFF;
    MEM_WR_D(cpu.regs.SP, ret & 0xFF);
    MEM_WR_D((cpu.regs.SP + 1) & 0xFFFF, ret >> 8);
    cpu.regs.PC = addr;
    cpu._pc_modified = true;
    return 11;
}

// I/O
int op_in_a_n(CPU& cpu) {
    uint8_t port = READ_PC(1);
    uint16_t addr = (uint16_t)((REG_A << 8) | port);
    REG_A = IO_RD(addr);
    return 11;
}

int op_out_n_a(CPU& cpu) {
    uint8_t port = READ_PC(1);
    uint16_t addr = (uint16_t)((REG_A << 8) | port);
    IO_WR(addr, REG_A);
    return 11;
}

// ============================================================
// CB handlers (rotates, shifts, bit ops)
// ============================================================
static inline int _cb_rot_r(CPU& cpu, uint8_t dest, uint8_t op_idx) {
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
    return 8;
}

int op_cb_rot(CPU& cpu) {
    uint8_t op_idx = (OPCODE >> 3) & 7;
    uint8_t dest = OPCODE & 7;
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
        MEM_WR(REG_HL, result);
        REG_F = result & (FLAG_S | FLAG_F3 | FLAG_F5);
        if (result == 0) REG_F |= FLAG_Z;
        if (PARITY_TABLE[result]) REG_F |= FLAG_PV;
        if (carry) REG_F |= FLAG_C;
        return 15;
    }
    return _cb_rot_r(cpu, dest, op_idx);
}

int op_cb_bit(CPU& cpu) {
    uint8_t bit = (OPCODE >> 3) & 7;
    uint8_t src = OPCODE & 7;
    if (src == 6) {
        uint8_t val = MEM_RD(REG_HL);
        uint8_t test = val & BIT_MASK[bit];
        cpu.regs.MEMPTR = REG_HL;
        REG_F = FLAG_H | (REG_F & FLAG_C);
        if (test == 0) REG_F |= FLAG_Z | FLAG_PV;
        if (bit == 7 && test) REG_F |= FLAG_S;
        REG_F |= (REG_HL >> 8) & (FLAG_F3 | FLAG_F5);
        return 12;
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
    return 8;
}

int op_cb_set_hl(CPU& cpu) {
    uint8_t bit = (OPCODE >> 3) & 7;
    uint8_t val = MEM_RD(REG_HL);
    MEM_WR(REG_HL, val | BIT_MASK[bit]);
    return 15;
}

int op_cb_res(CPU& cpu) {
    uint8_t bit = (OPCODE >> 3) & 7;
    uint8_t dest = OPCODE & 7;
    if (dest == 6) {
        uint8_t val = MEM_RD(REG_HL);
        MEM_WR(REG_HL, val & RES_MASK[bit]);
        return 15;
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
    return 8;
}

int op_cb_set(CPU& cpu) {
    uint8_t bit = (OPCODE >> 3) & 7;
    uint8_t dest = OPCODE & 7;
    if (dest == 6) {
        uint8_t val = MEM_RD(REG_HL);
        MEM_WR(REG_HL, val | BIT_MASK[bit]);
        return 15;
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
    return 8;
}

int op_cb_res_hl(CPU& cpu) {
    uint8_t bit = (OPCODE >> 3) & 7;
    uint8_t val = MEM_RD(REG_HL);
    MEM_WR(REG_HL, val & RES_MASK[bit]);
    return 15;
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

int op_ldi(CPU& cpu) {
    uint8_t val = MEM_RD(REG_HL);
    MEM_WR(REG_DE, val);
    SET_HL((REG_HL + 1) & 0xFFFF);
    SET_DE((REG_DE + 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _ld_block_flags(cpu, REG_A + val, REG_BC);
    return 16;
}

int op_ldir(CPU& cpu) {
    if (REG_BC == 0) return 16;
    op_ldi(cpu);
    if (REG_BC != 0) { cpu._pc_modified = true; return 21; }
    return 16;
}

int op_ldd(CPU& cpu) {
    uint8_t val = MEM_RD(REG_HL);
    MEM_WR(REG_DE, val);
    SET_HL((REG_HL - 1) & 0xFFFF);
    SET_DE((REG_DE - 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _ld_block_flags(cpu, REG_A + val, REG_BC);
    return 16;
}

int op_lddr(CPU& cpu) {
    if (REG_BC == 0) return 16;
    op_ldd(cpu);
    if (REG_BC != 0) { cpu._pc_modified = true; return 21; }
    return 16;
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

int op_cpi(CPU& cpu) {
    uint8_t val = MEM_RD(REG_HL);
    SET_HL((REG_HL + 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _cpi_flags(cpu, REG_A, val, REG_BC);
    return 16;
}

int op_cpir(CPU& cpu) {
    uint8_t val = MEM_RD(REG_HL);
    uint8_t a = REG_A;
    uint8_t result = (a - val) & 0xFF;
    SET_HL((REG_HL + 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _cpi_flags(cpu, a, val, REG_BC);
    if (REG_BC != 0 && result != 0) { cpu._pc_modified = true; return 21; }
    return 16;
}

int op_cpd(CPU& cpu) {
    uint8_t val = MEM_RD(REG_HL);
    SET_HL((REG_HL - 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _cpi_flags(cpu, REG_A, val, REG_BC);
    return 16;
}

int op_cpdr(CPU& cpu) {
    uint8_t val = MEM_RD(REG_HL);
    uint8_t a = REG_A;
    uint8_t result = (a - val) & 0xFF;
    SET_HL((REG_HL - 1) & 0xFFFF);
    SET_BC((REG_BC - 1) & 0xFFFF);
    _cpi_flags(cpu, a, val, REG_BC);
    if (REG_BC != 0 && result != 0) { cpu._pc_modified = true; return 21; }
    return 16;
}

static inline void _in_out_flags(CPU& cpu, uint8_t value, uint8_t old_b, uint8_t new_b) {
    uint8_t f = REG_F & FLAG_C;
    if (value & 0x80) f |= FLAG_N;
    if (new_b & 0x80) f |= FLAG_S;
    if (new_b == 0) f |= FLAG_Z;
    if ((old_b & 0x0F) == 0) f |= FLAG_H;
    if (old_b == 0x80) f |= FLAG_PV;
    REG_F = f;
}

int op_ini(CPU& cpu) {
    uint8_t val = IO_RD(REG_BC);
    MEM_WR(REG_HL, val);
    uint8_t old_b = REG_B;
    REG_B = (REG_B - 1) & 0xFF;
    SET_HL((REG_HL + 1) & 0xFFFF);
    _in_out_flags(cpu, val, old_b, REG_B);
    return 16;
}

int op_inir(CPU& cpu) {
    if (REG_B == 0) return 16;
    uint8_t old_b = REG_B;
    op_ini(cpu);
    if (old_b != 1) { cpu._pc_modified = true; return 21; }
    return 16;
}

int op_ind(CPU& cpu) {
    uint8_t val = IO_RD(REG_BC);
    MEM_WR(REG_HL, val);
    uint8_t old_b = REG_B;
    REG_B = (REG_B - 1) & 0xFF;
    SET_HL((REG_HL - 1) & 0xFFFF);
    _in_out_flags(cpu, val, old_b, REG_B);
    return 16;
}

int op_indr(CPU& cpu) {
    if (REG_B == 0) return 16;
    uint8_t old_b = REG_B;
    op_ind(cpu);
    if (old_b != 1) { cpu._pc_modified = true; return 21; }
    return 16;
}

int op_outi(CPU& cpu) {
    uint8_t val = MEM_RD(REG_HL);
    uint8_t old_b = REG_B;
    REG_B = (REG_B - 1) & 0xFF;
    IO_WR(REG_BC, val);
    SET_HL((REG_HL + 1) & 0xFFFF);
    _in_out_flags(cpu, val, old_b, REG_B);
    return 16;
}

int op_otir(CPU& cpu) {
    if (REG_B == 0) return 16;
    uint8_t old_b = REG_B;
    op_outi(cpu);
    if (old_b != 1) { cpu._pc_modified = true; return 21; }
    return 16;
}

int op_outd(CPU& cpu) {
    uint8_t val = MEM_RD(REG_HL);
    uint8_t old_b = REG_B;
    REG_B = (REG_B - 1) & 0xFF;
    IO_WR(REG_BC, val);
    SET_HL((REG_HL - 1) & 0xFFFF);
    _in_out_flags(cpu, val, old_b, REG_B);
    return 16;
}

int op_otdr(CPU& cpu) {
    if (REG_B == 0) return 16;
    uint8_t old_b = REG_B;
    op_outd(cpu);
    if (old_b != 1) { cpu._pc_modified = true; return 21; }
    return 16;
}

// 16-bit ADC/SBC
int op_adc_hl_rr(CPU& cpu) {
    int pair = (OPCODE - 0x4A) >> 1;  // 0x4A->0, 0x5A->1, 0x6A->2, 0x7A->3
    uint16_t hl = REG_HL;
    uint8_t carry = REG_F & FLAG_C;
    uint16_t src = cpu.regs.get_reg16(pair);
    SET_HL((hl + src + carry) & 0xFFFF);
    REG_F = adc16_flags(hl, src, carry);
    return 15;
}

int op_sbc_hl_rr(CPU& cpu) {
    int pair = (OPCODE - 0x42) >> 1;  // 0x42->0, 0x52->1, 0x62->2, 0x72->3
    uint16_t hl = REG_HL;
    uint8_t carry = REG_F & FLAG_C;
    uint16_t src = cpu.regs.get_reg16(pair);
    SET_HL((hl - src - carry) & 0xFFFF);
    REG_F = sbc16_flags(hl, src, carry);
    return 15;
}

// ED LD (nn),rr / LD rr,(nn)
int op_ld_rr_nn_ind(CPU& cpu) {
    int pair;
    uint8_t op2 = READ_PC(1);
    if (op2 == 0x4B) pair = 0;
    else if (op2 == 0x5B) pair = 1;
    else if (op2 == 0x6B) pair = 2;
    else pair = 3;
    uint16_t addr = READ_PC(2) | ((uint16_t)READ_PC(3) << 8);
    uint8_t lo = MEM_RD(addr);
    uint8_t hi = MEM_RD((addr + 1) & 0xFFFF);
    cpu.regs.set_reg16(pair, (uint16_t)(lo | (hi << 8)));
    return 20;
}

int op_ld_nn_rr(CPU& cpu) {
    int pair;
    uint8_t op2 = READ_PC(1);
    if (op2 == 0x43) pair = 0;
    else if (op2 == 0x53) pair = 1;
    else if (op2 == 0x63) pair = 2;
    else pair = 3;
    uint16_t addr = READ_PC(2) | ((uint16_t)READ_PC(3) << 8);
    uint16_t val = cpu.regs.get_reg16(pair);
    MEM_WR(addr, val & 0xFF);
    MEM_WR((addr + 1) & 0xFFFF, val >> 8);
    return 20;
}

// NEG - compute flags manually since 0-a is special
int op_neg(CPU& cpu) {
    uint8_t a = REG_A;
    uint8_t result = (-a) & 0xFF;
    REG_A = result;
    // NEG: N=1, Z=(result==0), C=(a!=0), H=(a&0x0F)!=0, PV=(result==0x80)
    REG_F = FLAG_N;
    if (result == 0) REG_F |= FLAG_Z;
    if (a != 0) REG_F |= FLAG_C;
    if ((a & 0x0F) != 0) REG_F |= FLAG_H;
    if (result == 0x80) REG_F |= FLAG_PV;
    if (result & 0x80) REG_F |= FLAG_S;
    return 8;
}

// RETI/RETN
int op_reti(CPU& cpu) {
    uint16_t sp = cpu.regs.SP;
    uint8_t lo = MEM_RD(sp);
    uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
    cpu.regs.SP = (sp + 2) & 0xFFFF;
    cpu.regs.PC = (uint16_t)(lo | (hi << 8));
    cpu._pc_modified = true;
    cpu.regs.IFF1 = cpu.regs.IFF2;
    return 14;
}

int op_retn(CPU& cpu) {
    uint16_t sp = cpu.regs.SP;
    uint8_t lo = MEM_RD(sp);
    uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
    cpu.regs.SP = (sp + 2) & 0xFFFF;
    cpu.regs.PC = (uint16_t)(lo | (hi << 8));
    cpu._pc_modified = true;
    cpu.regs.IFF1 = cpu.regs.IFF2;
    return 14;
}

// IM
int op_im(CPU& cpu) {
    uint8_t op2 = READ_PC(1);
    if (op2 == 0x46 || op2 == 0x66 || op2 == 0x4E || op2 == 0x6E) cpu.regs.IM = 0;
    else if (op2 == 0x56 || op2 == 0x76) cpu.regs.IM = 1;
    else if (op2 == 0x5E || op2 == 0x7E) cpu.regs.IM = 2;
    return 8;
}

// IN r,(C)
int op_in_r_c(CPU& cpu) {
    int reg = (OPCODE >> 3) & 7;
    uint8_t val = IO_RD(REG_BC);
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
    return 12;
}

// OUT (C),r
int op_out_c_r(CPU& cpu) {
    int reg = (OPCODE >> 3) & 7;
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
    IO_WR(REG_BC, val);
    return 12;
}

// LD I,A / LD R,A / LD A,I / LD A,R
int op_ld_i_a(CPU& cpu) { cpu.regs.I = REG_A; return 9; }
int op_ld_r_a(CPU& cpu) { cpu.regs.R = REG_A; return 9; }

int op_ld_a_i(CPU& cpu) {
    uint8_t a = cpu.regs.I;
    REG_A = a;
    REG_F = (REG_F & FLAG_C) | (a & (FLAG_S | FLAG_F3 | FLAG_F5));
    if (a == 0) REG_F |= FLAG_Z;
    if (cpu.regs.IFF2) REG_F |= FLAG_PV;
    cpu._is_ld_a_ir = true;
    return 9;
}

int op_ld_a_r(CPU& cpu) {
    uint8_t a = cpu.regs.R;
    REG_A = a;
    REG_F = (REG_F & FLAG_C) | (a & (FLAG_S | FLAG_F3 | FLAG_F5));
    if (a == 0) REG_F |= FLAG_Z;
    if (cpu.regs.IFF2) REG_F |= FLAG_PV;
    cpu._is_ld_a_ir = true;
    return 9;
}

// RLD/RRD
int op_rld(CPU& cpu) {
    uint8_t val = MEM_RD(REG_HL);
    uint8_t low = val & 0x0F;
    uint8_t high = val & 0xF0;
    uint8_t a_low = REG_A & 0x0F;
    uint8_t new_val = (low << 4) | a_low;
    uint8_t new_a = (REG_A & 0xF0) | (high >> 4);
    MEM_WR(REG_HL, new_val);
    REG_A = new_a;
    REG_F = REG_F & FLAG_C;
    if (REG_A == 0) REG_F |= FLAG_Z;
    if (REG_A & 0x80) REG_F |= FLAG_S;
    if (PARITY_TABLE[REG_A]) REG_F |= FLAG_PV;
    REG_F |= REG_A & (FLAG_F3 | FLAG_F5);
    return 18;
}

int op_rrd(CPU& cpu) {
    uint8_t val = MEM_RD(REG_HL);
    uint8_t low = val & 0x0F;
    uint8_t high = val & 0xF0;
    uint8_t a_low = REG_A & 0x0F;
    uint8_t new_val = (a_low << 4) | (high >> 4);
    uint8_t new_a = (REG_A & 0xF0) | low;
    MEM_WR(REG_HL, new_val);
    REG_A = new_a;
    REG_F = REG_F & FLAG_C;
    if (REG_A == 0) REG_F |= FLAG_Z;
    if (REG_A & 0x80) REG_F |= FLAG_S;
    if (PARITY_TABLE[REG_A]) REG_F |= FLAG_PV;
    REG_F |= REG_A & (FLAG_F3 | FLAG_F5);
    return 18;
}

// ============================================================
// DD/FD indexed handlers
// ============================================================
static inline uint16_t _get_ix_addr(CPU& cpu) {
    uint8_t disp = READ_PC(2);
    int16_t d = (int8_t)disp;
    uint16_t base = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    uint16_t addr = (base + d) & 0xFFFF;
    cpu.regs.MEMPTR = addr;
    return addr;
}

int op_dd_fd_ld_ix_nn(CPU& cpu) {
    uint16_t val = READ_PC(2) | ((uint16_t)READ_PC(3) << 8);
    if (cpu._is_iy) cpu.regs.IY = val;
    else cpu.regs.IX = val;
    return 14;
}

int op_dd_fd_ld_nn_ix(CPU& cpu) {
    uint16_t addr = READ_PC(2) | ((uint16_t)READ_PC(3) << 8);
    uint16_t val = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    MEM_WR(addr, val & 0xFF);
    MEM_WR((addr + 1) & 0xFFFF, val >> 8);
    return 20;
}

int op_dd_fd_ld_ix_nn_ind(CPU& cpu) {
    uint16_t addr = READ_PC(2) | ((uint16_t)READ_PC(3) << 8);
    uint8_t lo = MEM_RD(addr);
    uint8_t hi = MEM_RD((addr + 1) & 0xFFFF);
    if (cpu._is_iy) cpu.regs.IY = (uint16_t)(lo | (hi << 8));
    else cpu.regs.IX = (uint16_t)(lo | (hi << 8));
    return 20;
}

int op_dd_fd_inc_ix(CPU& cpu) {
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY + 1) & 0xFFFF;
    else cpu.regs.IX = (cpu.regs.IX + 1) & 0xFFFF;
    return 10;
}

int op_dd_fd_dec_ix(CPU& cpu) {
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY - 1) & 0xFFFF;
    else cpu.regs.IX = (cpu.regs.IX - 1) & 0xFFFF;
    return 10;
}

int op_dd_fd_ld_sp_ix(CPU& cpu) {
    cpu.regs.SP = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    return 10;
}

int op_dd_fd_push_ix(CPU& cpu) {
    uint16_t val = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    cpu.regs.SP = (cpu.regs.SP - 2) & 0xFFFF;
    MEM_WR_D(cpu.regs.SP, val & 0xFF);
    MEM_WR_D((cpu.regs.SP + 1) & 0xFFFF, val >> 8);
    return 15;
}

int op_dd_fd_pop_ix(CPU& cpu) {
    uint16_t sp = cpu.regs.SP;
    uint8_t lo = MEM_RD(sp);
    uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
    cpu.regs.SP = (sp + 2) & 0xFFFF;
    if (cpu._is_iy) cpu.regs.IY = (uint16_t)(lo | (hi << 8));
    else cpu.regs.IX = (uint16_t)(lo | (hi << 8));
    return 14;
}

int op_dd_fd_ex_sp_ix(CPU& cpu) {
    uint16_t sp = cpu.regs.SP;
    uint8_t lo = MEM_RD(sp);
    uint8_t hi = MEM_RD((sp + 1) & 0xFFFF);
    uint16_t temp = (uint16_t)(lo | (hi << 8));
    MEM_WR_D(sp, (cpu._is_iy ? cpu.regs.IY : cpu.regs.IX) & 0xFF);
    MEM_WR_D((sp + 1) & 0xFFFF, (cpu._is_iy ? cpu.regs.IY : cpu.regs.IX) >> 8);
    cpu.regs.MEMPTR = temp;
    if (cpu._is_iy) cpu.regs.IY = temp;
    else cpu.regs.IX = temp;
    return 23;
}

int op_dd_fd_add_ix_rr(CPU& cpu) {
    int pair = (OPCODE >> 4) & 3;
    uint16_t ix = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    uint16_t op = (pair == 2) ? ix : cpu.regs.get_reg16(pair);
    uint16_t result = (ix + op) & 0xFFFF;
    if (cpu._is_iy) cpu.regs.IY = result;
    else cpu.regs.IX = result;
    REG_F = (REG_F & (FLAG_S | FLAG_Z | FLAG_PV)) | add16_flags(ix, op, REG_F);
    return 15;
}

int op_dd_fd_jp_ix(CPU& cpu) {
    uint16_t target = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    cpu.regs.MEMPTR = target;
    cpu.regs.PC = target;
    cpu._pc_modified = true;
    return 8;
}

int op_dd_fd_ld_r_ixd(CPU& cpu) {
    uint8_t dest = (OPCODE >> 3) & 7;
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t val = MEM_RD(addr);
    switch (dest) {
        case 0: REG_B = val; break;
        case 1: REG_C = val; break;
        case 2: REG_D = val; break;
        case 3: REG_E = val; break;
        case 4: REG_H = val; break;
        case 5: REG_L = val; break;
        default: REG_A = val; break;
    }
    return 19;
}

int op_dd_fd_ld_ixd_r(CPU& cpu) {
    uint8_t src = OPCODE & 7;
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
    MEM_WR(addr, val);
    return 19;
}

int op_dd_fd_ld_ixd_n(CPU& cpu) {
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t val = READ_PC(3);
    MEM_WR(addr, val);
    return 19;
}

int op_dd_fd_inc_ixd(CPU& cpu) {
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t val = MEM_RD(addr);
    uint8_t nv = (val + 1) & 0xFF;
    MEM_WR(addr, nv);
    REG_F = (REG_F & FLAG_C) | INC_FLAGS[val];
    return 23;
}

int op_dd_fd_dec_ixd(CPU& cpu) {
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t val = MEM_RD(addr);
    uint8_t nv = (val - 1) & 0xFF;
    MEM_WR(addr, nv);
    REG_F = (REG_F & FLAG_C) | DEC_FLAGS[val];
    return 23;
}

// IXH/IYH, IXL/IYL operations
int op_dd_fd_inc_ixh(CPU& cpu) {
    uint8_t val = cpu._is_iy ? (cpu.regs.IY >> 8) & 0xFF : (cpu.regs.IX >> 8) & 0xFF;
    uint8_t nv = (val + 1) & 0xFF;
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0x00FF) | ((uint16_t)nv << 8);
    else cpu.regs.IX = (cpu.regs.IX & 0x00FF) | ((uint16_t)nv << 8);
    REG_F = (REG_F & FLAG_C) | INC_FLAGS[val];
    return 8;
}

int op_dd_fd_dec_ixh(CPU& cpu) {
    uint8_t val = cpu._is_iy ? (cpu.regs.IY >> 8) & 0xFF : (cpu.regs.IX >> 8) & 0xFF;
    uint8_t nv = (val - 1) & 0xFF;
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0x00FF) | ((uint16_t)nv << 8);
    else cpu.regs.IX = (cpu.regs.IX & 0x00FF) | ((uint16_t)nv << 8);
    REG_F = (REG_F & FLAG_C) | DEC_FLAGS[val];
    return 8;
}

int op_dd_fd_inc_ixl(CPU& cpu) {
    uint8_t val = cpu._is_iy ? cpu.regs.IY & 0xFF : cpu.regs.IX & 0xFF;
    uint8_t nv = (val + 1) & 0xFF;
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0xFF00) | nv;
    else cpu.regs.IX = (cpu.regs.IX & 0xFF00) | nv;
    REG_F = (REG_F & FLAG_C) | INC_FLAGS[val];
    return 8;
}

int op_dd_fd_dec_ixl(CPU& cpu) {
    uint8_t val = cpu._is_iy ? cpu.regs.IY & 0xFF : cpu.regs.IX & 0xFF;
    uint8_t nv = (val - 1) & 0xFF;
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0xFF00) | nv;
    else cpu.regs.IX = (cpu.regs.IX & 0xFF00) | nv;
    REG_F = (REG_F & FLAG_C) | DEC_FLAGS[val];
    return 8;
}

int op_dd_fd_ld_ixh_n(CPU& cpu) {
    uint8_t val = READ_PC(2);
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0x00FF) | ((uint16_t)val << 8);
    else cpu.regs.IX = (cpu.regs.IX & 0x00FF) | ((uint16_t)val << 8);
    return 11;
}

int op_dd_fd_ld_ixl_n(CPU& cpu) {
    uint8_t val = READ_PC(2);
    if (cpu._is_iy) cpu.regs.IY = (cpu.regs.IY & 0xFF00) | val;
    else cpu.regs.IX = (cpu.regs.IX & 0xFF00) | val;
    return 11;
}

int op_dd_fd_ld_r_ixh(CPU& cpu) {
    uint8_t dest = (OPCODE >> 3) & 7;
    uint8_t val = cpu._is_iy ? (cpu.regs.IY >> 8) & 0xFF : (cpu.regs.IX >> 8) & 0xFF;
    switch (dest) {
        case 0: REG_B = val; break;
        case 1: REG_C = val; break;
        case 2: REG_D = val; break;
        case 3: REG_E = val; break;
        default: REG_A = val; break;
    }
    return 8;
}

int op_dd_fd_ld_r_ixl(CPU& cpu) {
    uint8_t dest = (OPCODE >> 3) & 7;
    uint8_t val = cpu._is_iy ? cpu.regs.IY & 0xFF : cpu.regs.IX & 0xFF;
    switch (dest) {
        case 0: REG_B = val; break;
        case 1: REG_C = val; break;
        case 2: REG_D = val; break;
        case 3: REG_E = val; break;
        default: REG_A = val; break;
    }
    return 8;
}

int op_dd_fd_ld_ixh_r(CPU& cpu) {
    uint8_t src = OPCODE & 7;
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
    return 8;
}

int op_dd_fd_ld_ixl_r(CPU& cpu) {
    uint8_t src = OPCODE & 7;
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
    return 8;
}

int op_dd_fd_ld_ixh_ixh(CPU& cpu) { return 8; }
int op_dd_fd_ld_ixh_ixl(CPU& cpu) {
    uint8_t val;
    if (cpu._is_iy) { val = cpu.regs.IY & 0xFF; cpu.regs.IY = (cpu.regs.IY & 0x00FF) | ((uint16_t)val << 8); }
    else { val = cpu.regs.IX & 0xFF; cpu.regs.IX = (cpu.regs.IX & 0x00FF) | ((uint16_t)val << 8); }
    return 8;
}
int op_dd_fd_ld_ixl_ixh(CPU& cpu) {
    uint8_t val;
    if (cpu._is_iy) { val = (cpu.regs.IY >> 8) & 0xFF; cpu.regs.IY = (cpu.regs.IY & 0xFF00) | val; }
    else { val = (cpu.regs.IX >> 8) & 0xFF; cpu.regs.IX = (cpu.regs.IX & 0xFF00) | val; }
    return 8;
}
int op_dd_fd_ld_ixl_ixl(CPU& cpu) { return 8; }

int op_dd_fd_ld_a_ixh(CPU& cpu) {
    REG_A = cpu._is_iy ? (cpu.regs.IY >> 8) & 0xFF : (cpu.regs.IX >> 8) & 0xFF;
    return 8;
}
int op_dd_fd_ld_a_ixl(CPU& cpu) {
    REG_A = cpu._is_iy ? cpu.regs.IY & 0xFF : cpu.regs.IX & 0xFF;
    return 8;
}

// ALU with IXH/IXL
int op_dd_fd_alu_ixh(CPU& cpu) {
    uint8_t alu_op = (OPCODE >> 3) & 7;
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
    return 8;
}

int op_dd_fd_alu_ixl(CPU& cpu) {
    uint8_t alu_op = (OPCODE >> 3) & 7;
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
    return 8;
}

int op_dd_fd_alu_ixd(CPU& cpu) {
    uint8_t alu_op = (OPCODE >> 3) & 7;
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t b = MEM_RD(addr);
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
    return 19;
}

// ADC/SBC IX/IY,rr
int op_dd_fd_adc_ix_rr(CPU& cpu) {
    int pair = (OPCODE - 0x4A) >> 4;  // 0x4A->0, 0x5A->1, 0x6A->2, 0x7A->3
    uint16_t ix = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    uint8_t carry = REG_F & FLAG_C;
    uint16_t src = (pair == 2) ? ix : cpu.regs.get_reg16(pair);
    uint16_t result = (ix + src + carry) & 0xFFFF;
    if (cpu._is_iy) cpu.regs.IY = result;
    else cpu.regs.IX = result;
    REG_F = adc16_flags(ix, src, carry);
    return 15;
}

int op_dd_fd_sbc_ix_rr(CPU& cpu) {
    int pair = (OPCODE - 0x42) >> 4;  // 0x42->0, 0x52->1, 0x62->2, 0x72->3
    uint16_t ix = cpu._is_iy ? cpu.regs.IY : cpu.regs.IX;
    uint8_t carry = REG_F & FLAG_C;
    uint16_t src = (pair == 2) ? ix : cpu.regs.get_reg16(pair);
    uint16_t result = (ix - src - carry) & 0xFFFF;
    if (cpu._is_iy) cpu.regs.IY = result;
    else cpu.regs.IX = result;
    REG_F = sbc16_flags(ix, src, carry);
    return 15;
}

// ============================================================
// DDCB/FDCB handlers
// ============================================================
int op_ddcb_fdcb_rot(CPU& cpu) {
    uint8_t op_idx = (OPCODE >> 3) & 7;
    uint8_t dest = OPCODE & 7;
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t val = MEM_RD(addr);
    uint8_t old_carry = REG_F & FLAG_C;
    uint8_t result, carry;
    // Use ROT tables for ops 0,1,4,5,6,7; RL/RR need carry
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
    MEM_WR(addr, result);
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
    return 23;
}

int op_ddcb_fdcb_bit(CPU& cpu) {
    uint8_t bit = (OPCODE >> 3) & 7;
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t val = MEM_RD(addr);
    uint8_t test = val & BIT_MASK[bit];
    REG_F = FLAG_H | (REG_F & FLAG_C);
    if (test == 0) REG_F |= FLAG_Z | FLAG_PV;
    if (bit == 7 && test) REG_F |= FLAG_S;
    REG_F |= (addr >> 8) & (FLAG_F3 | FLAG_F5);
    return 20;
}

int op_ddcb_fdcb_res(CPU& cpu) {
    uint8_t bit = (OPCODE >> 3) & 7;
    uint8_t dest = OPCODE & 7;
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t val = MEM_RD(addr);
    uint8_t result = val & RES_MASK[bit];
    MEM_WR(addr, result);
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
    return 23;
}

int op_ddcb_fdcb_set(CPU& cpu) {
    uint8_t bit = (OPCODE >> 3) & 7;
    uint8_t dest = OPCODE & 7;
    uint16_t addr = _get_ix_addr(cpu);
    uint8_t val = MEM_RD(addr);
    uint8_t result = val | BIT_MASK[bit];
    MEM_WR(addr, result);
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
    return 23;
}

