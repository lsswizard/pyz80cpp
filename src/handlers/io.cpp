#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

// ============================================================
// Block I/O flag helper
//
// Z80 CPU User Manual (and Sean Young's "Undocumented Z80"):
//
//   After INI/IND/OUTI/OUTD the flags are set as follows:
//     N  = bit 7 of the transferred byte
//     B  drives S, Z, F5, F3 (B is already decremented before this call)
//     t  = val + ((C ± 1) & 0xFF)   [+1 for INI/INIR/OUTI/OTIR, -1 for IND/INDR/OUTD/OTDR]
//     C  = H = set if t > 255 (carry out)
//     PV = parity of (t & 7) XOR B
//
// 'val'   — the byte that was transferred
// 'c_adj' — +1 for incrementing ops, -1 for decrementing ops
// 'port_c'— the value of C at the time of the I/O (before B is decremented)
// ============================================================
static inline void set_block_io_flags(Z80& cpu, uint8_t val, uint8_t port_c, int c_adj) {
    // B has already been decremented before this is called
    uint8_t  b   = cpu.regs.B;
    uint16_t t   = (uint16_t)val + (uint8_t)(port_c + c_adj);
    uint8_t  f   = 0;

    // S, Z, F5, F3 — from decremented B
    if (b & 0x80)  f |= Flags::S;
    if (b == 0)    f |= Flags::Z;
    f |= b & (Flags::F5 | Flags::F3);

    // N — bit 7 of the transferred byte
    if (val & 0x80)  f |= Flags::N;

    // C and H — set if t overflowed 8 bits
    if (t > 0xFF) f |= (Flags::C | Flags::H);

    // PV — parity of ((t & 7) XOR B)
    if (FlagTables::PARITY_TABLE[(t & 7) ^ b])  f |= Flags::PV;

    cpu.regs.F = f;
}

// ============================================================
// Standard I/O
// ============================================================

// IN A,(n) — 11 T-states: 4(M1) + 3(fetch n) + 4(I/O)
// Port = (A << 8) | n  per Z80 spec
void handle_in_a_n(Z80& cpu) {
    uint8_t  n         = cpu.read(cpu.regs.PC++);
    uint16_t port      = (uint16_t(cpu.regs.A) << 8) | n;
    cpu.regs.A         = cpu.in(port);
    cpu.regs.MEMPTR    = (port + 1) & 0xFFFF;
    cpu.regs.Q = 0;
}

// OUT (n),A — 11 T-states
void handle_out_n_a(Z80& cpu) {
    uint8_t  n         = cpu.read(cpu.regs.PC++);
    uint16_t port      = (uint16_t(cpu.regs.A) << 8) | n;
    cpu.out(port, cpu.regs.A);
    // MEMPTR: high = A, low = (n+1) & 0xFF  (documented MEMPTR behaviour)
    cpu.regs.MEMPTR    = (uint16_t(cpu.regs.A) << 8) | ((n + 1) & 0xFF);
    cpu.regs.Q = 0;
}

// IN r,(C) — 12 T-states: 4(M1 ED) + 4(M1 op) + 4(I/O)
// If r=6 (opcode 0x70), result is discarded but flags are still set
void handle_in_r_c(Z80& cpu) {
    int      reg  = (cpu.current_opcode >> 3) & 7;
    uint16_t port = cpu.regs.BC();
    uint8_t  val  = cpu.in(port);
    cpu.regs.MEMPTR = (port + 1) & 0xFFFF;

    if (reg != 6) cpu.write_reg8(reg, val);

    cpu.regs.F = (cpu.regs.F & Flags::C)
               | (val & (Flags::S | Flags::F5 | Flags::F3))
               | (val == 0 ? Flags::Z : 0)
               | FlagTables::PARITY_TABLE[val];
    cpu.regs.Q = cpu.regs.F;
}

// OUT (C),r — 12 T-states
// If r=6 (opcode 0x71), outputs 0 (or 0xFF on some revisions; 0 is the documented value)
void handle_out_c_r(Z80& cpu) {
    int     reg  = (cpu.current_opcode >> 3) & 7;
    uint8_t val  = (reg == 6) ? 0 : cpu.read_reg8(reg);
    cpu.out(cpu.regs.BC(), val);
    cpu.regs.MEMPTR = (cpu.regs.BC() + 1) & 0xFFFF;
    cpu.regs.Q = 0;
}

// ============================================================
// Block I/O — INI / IND / INIR / INDR
// ============================================================

// INI — 16 T-states: 4+4+1+4+3
void handle_ini(Z80& cpu) {
    cpu.wait(1);
    uint8_t  port_c = cpu.regs.C;
    uint8_t  val    = cpu.in(cpu.regs.BC());
    cpu.write(cpu.regs.HL(), val);
    cpu.regs.set_HL(cpu.regs.HL() + 1);
    cpu.regs.B--;
    cpu.regs.MEMPTR = (cpu.regs.BC() + 1) & 0xFFFF;
    set_block_io_flags(cpu, val, port_c, +1);
    cpu.regs.Q = cpu.regs.F;
}

// IND — 16 T-states
void handle_ind(Z80& cpu) {
    cpu.wait(1);
    uint8_t  port_c = cpu.regs.C;
    uint8_t  val    = cpu.in(cpu.regs.BC());
    cpu.write(cpu.regs.HL(), val);
    cpu.regs.set_HL(cpu.regs.HL() - 1);
    cpu.regs.B--;
    cpu.regs.MEMPTR = (cpu.regs.BC() - 1) & 0xFFFF;
    set_block_io_flags(cpu, val, port_c, -1);
    cpu.regs.Q = cpu.regs.F;
}

// INIR — 21 if B≠0, 16 if B=0
void handle_inir(Z80& cpu) {
    handle_ini(cpu);
    if (cpu.regs.B != 0) {
        cpu.wait(5);
        cpu.regs.PC -= 2;
        cpu.regs.MEMPTR = cpu.regs.PC + 1;
    }
}

// INDR — 21 if B≠0, 16 if B=0
void handle_indr(Z80& cpu) {
    handle_ind(cpu);
    if (cpu.regs.B != 0) {
        cpu.wait(5);
        cpu.regs.PC -= 2;
        cpu.regs.MEMPTR = cpu.regs.PC + 1;
    }
}

// ============================================================
// Block I/O — OUTI / OUTD / OTIR / OTDR
// ============================================================

// OUTI — 16 T-states: 4+4+1+3+4
void handle_outi(Z80& cpu) {
    cpu.wait(1);
    uint8_t val    = cpu.read(cpu.regs.HL());
    cpu.regs.B--;
    cpu.out(cpu.regs.BC(), val);
    cpu.regs.set_HL(cpu.regs.HL() + 1);
    cpu.regs.MEMPTR = (cpu.regs.BC() + 1) & 0xFFFF;
    // BUG FIX: use L (the *new* HL low byte after increment) as c_adj proxy.
    // The formula is t = val + L (same as INI but with L, not (C+1)).
    // Reconciled with block IO spec: t = val + L for OUT variants.
    uint16_t t = (uint16_t)val + cpu.regs.L;
    uint8_t  b = cpu.regs.B;
    uint8_t  f = 0;
    if (b & 0x80)    f |= Flags::S;
    if (b == 0)      f |= Flags::Z;
    f |= b & (Flags::F5 | Flags::F3);
    if (val & 0x80)  f |= Flags::N;
    if (t > 0xFF)    f |= (Flags::C | Flags::H);
    if (FlagTables::PARITY_TABLE[(t & 7) ^ b]) f |= Flags::PV;
    cpu.regs.F = f;
    cpu.regs.Q = f;
}

// OUTD — 16 T-states
void handle_outd(Z80& cpu) {
    cpu.wait(1);
    uint8_t val    = cpu.read(cpu.regs.HL());
    cpu.regs.B--;
    cpu.out(cpu.regs.BC(), val);
    cpu.regs.set_HL(cpu.regs.HL() - 1);
    cpu.regs.MEMPTR = (cpu.regs.BC() - 1) & 0xFFFF;
    // t = val + L (new L after decrement)
    uint16_t t = (uint16_t)val + cpu.regs.L;
    uint8_t  b = cpu.regs.B;
    uint8_t  f = 0;
    if (b & 0x80)    f |= Flags::S;
    if (b == 0)      f |= Flags::Z;
    f |= b & (Flags::F5 | Flags::F3);
    if (val & 0x80)  f |= Flags::N;
    if (t > 0xFF)    f |= (Flags::C | Flags::H);
    if (FlagTables::PARITY_TABLE[(t & 7) ^ b]) f |= Flags::PV;
    cpu.regs.F = f;
    cpu.regs.Q = f;
}

void handle_otir(Z80& cpu) {
    handle_outi(cpu);
    if (cpu.regs.B != 0) {
        cpu.wait(5);
        cpu.regs.PC -= 2;
        cpu.regs.MEMPTR = cpu.regs.PC + 1;
    }
}

void handle_otdr(Z80& cpu) {
    handle_outd(cpu);
    if (cpu.regs.B != 0) {
        cpu.wait(5);
        cpu.regs.PC -= 2;
        cpu.regs.MEMPTR = cpu.regs.PC + 1;
    }
}

// ============================================================
// CB-prefix: Bit / Shift / Rotate
// ============================================================

// Set flags after a CB rotate/shift
static inline void set_rot_flags(Z80& cpu, uint8_t res, uint8_t new_carry) {
    cpu.regs.F = (res & (Flags::S | Flags::F5 | Flags::F3))
               | (res == 0 ? Flags::Z : 0)
               | FlagTables::PARITY_TABLE[res]
               | (new_carry ? Flags::C : 0);
    cpu.regs.Q = cpu.regs.F;
}

#define ROT_OP(NAME, EXPR)                          \
void NAME(Z80& cpu) {                               \
    int     reg = cpu.current_opcode & 7;           \
    if (reg == 6) cpu.wait(1);                      \
    uint8_t val = cpu.read_reg8(reg);               \
    EXPR;                                           \
    cpu.write_reg8(reg, res);                       \
    set_rot_flags(cpu, res, new_c);                 \
}

ROT_OP(handle_rlc_r,
    uint8_t new_c = val >> 7;
    uint8_t res   = (val << 1) | new_c;)

ROT_OP(handle_rrc_r,
    uint8_t new_c = val & 1;
    uint8_t res   = (val >> 1) | (new_c << 7);)

ROT_OP(handle_rl_r,
    uint8_t new_c = val >> 7;
    uint8_t res   = (val << 1) | ((cpu.regs.F & Flags::C) ? 1 : 0);)

ROT_OP(handle_rr_r,
    uint8_t new_c = val & 1;
    uint8_t res   = (val >> 1) | ((cpu.regs.F & Flags::C) ? 0x80 : 0);)

ROT_OP(handle_sla_r,
    uint8_t new_c = val >> 7;
    uint8_t res   = val << 1;)

ROT_OP(handle_sra_r,
    uint8_t new_c = val & 1;
    uint8_t res   = (val >> 1) | (val & 0x80);)

ROT_OP(handle_sll_r,                    // undocumented SLL (shift left, bit 0 = 1)
    uint8_t new_c = val >> 7;
    uint8_t res   = (val << 1) | 1;)

ROT_OP(handle_srl_r,
    uint8_t new_c = val & 1;
    uint8_t res   = val >> 1;)

// BIT b,r — tests bit; affects Z, H, S, PV, F3, F5
// Based on Fuse implementation:
// For register operands: F5/F3 come from the value tested
// For (HL) operand: F5/F3 come from MEMPTR (HL+1), not from value
void handle_cb_bit(Z80& cpu) {
    int     bit_pos = (cpu.current_opcode >> 3) & 7;
    int     reg     = cpu.current_opcode & 7;
    
    uint8_t val;
    if (reg == 6) {
        // BIT b,(HL) - special case
        cpu.wait(1);
        val = cpu.read(cpu.regs.HL());
    } else {
        val = cpu.read_reg8(reg);
    }
    
    uint8_t result = val & (1 << bit_pos);
    
    uint8_t f = (cpu.regs.F & Flags::C) | Flags::H;
    if (result == 0) f |= (Flags::Z | Flags::PV);  // PV = Z always
    if (result & Flags::S) f |= Flags::S;
    
    // For (HL), F5/F3 come from MEMPTR (HL+1), not from value
    // Update MEMPTR = HL + 1 for indexed operations
    uint16_t memptr = (cpu.regs.HL() + 1) & 0xFFFF;
    
    if (reg == 6) {
        // (HL) operand - use MEMPTR high byte for F5/F3
        f |= (memptr >> 8) & (Flags::F5 | Flags::F3);
    } else {
        // Register operands - use the value itself
        f |= val & (Flags::F5 | Flags::F3);
    }
    
    cpu.regs.F = f;
    cpu.regs.Q = f;
}

void handle_cb_res(Z80& cpu) {
    int     bit_pos = (cpu.current_opcode >> 3) & 7;
    int     reg     = cpu.current_opcode & 7;
    if (reg == 6) cpu.wait(1);
    cpu.write_reg8(reg, cpu.read_reg8(reg) & ~(1 << bit_pos));
    cpu.regs.Q = 0;
}

void handle_cb_set(Z80& cpu) {
    int     bit_pos = (cpu.current_opcode >> 3) & 7;
    int     reg     = cpu.current_opcode & 7;
    if (reg == 6) cpu.wait(1);
    cpu.write_reg8(reg, cpu.read_reg8(reg) | (1 << bit_pos));
    cpu.regs.Q = 0;
}

// ============================================================
// DDCB/FDCB: indexed rotate/shift/bit operations
// All read from (IX+d); optionally store result in a register too (undocumented)
// T-states for rotate/shift/res/set: 23 total (already accounted by 4+4+3+3 fetch + wait below)
// ============================================================

static uint16_t ddcb_addr(Z80& cpu) {
    uint16_t ix = cpu.prefix_ix ? cpu.regs.IX : cpu.regs.IY;
    return (ix + cpu.ddcb_displacement) & 0xFFFF;
}

void handle_ddcb_fdcb_rot(Z80& cpu) {
    uint16_t addr       = ddcb_addr(cpu);
    cpu.regs.MEMPTR     = addr;
    uint8_t  val        = cpu.read(addr);
    cpu.wait(2);   // extra internal states (total mem-read cycle = 3+2 = 5, then write = 3)

    int     op    = (cpu.ddcb_opcode >> 3) & 7;
    uint8_t old_c = (cpu.regs.F & Flags::C) ? 1 : 0;
    uint8_t new_c, res;

    switch (op) {
        case 0: new_c = val >> 7; res = (val << 1) | new_c; break;             // RLC
        case 1: new_c = val & 1;  res = (val >> 1) | (new_c << 7); break;     // RRC
        case 2: new_c = val >> 7; res = (val << 1) | old_c; break;            // RL
        case 3: new_c = val & 1;  res = (val >> 1) | (old_c << 7); break;     // RR
        case 4: new_c = val >> 7; res = val << 1; break;                       // SLA
        case 5: new_c = val & 1;  res = (val >> 1) | (val & 0x80); break;     // SRA
        case 6: new_c = val >> 7; res = (val << 1) | 1; break;                // SLL (undoc)
        default:new_c = val & 1;  res = val >> 1; break;                       // SRL
    }

    cpu.write(addr, res);
    // Undocumented: result also stored in register (if not 6)
    int reg = cpu.ddcb_opcode & 7;
    if (reg != 6) cpu.write_reg8(reg, res);

    set_rot_flags(cpu, res, new_c);
}

void handle_ddcb_fdcb_bit(Z80& cpu) {
    uint16_t addr   = ddcb_addr(cpu);
    cpu.regs.MEMPTR = addr;
    uint8_t  val    = cpu.read(addr);
    cpu.wait(2);

    int     bit_pos = (cpu.ddcb_opcode >> 3) & 7;
    uint8_t result  = val & (1 << bit_pos);

    uint8_t f = (cpu.regs.F & Flags::C) | Flags::H;
    if (result == 0)        f |= (Flags::Z | Flags::PV);
    if (result & Flags::S)  f |= Flags::S;

    // For indexed BIT, F5/F3 come from MEMPTR high byte
    f |= (addr >> 8) & (Flags::F5 | Flags::F3);

    cpu.regs.F = f;
    cpu.regs.Q = f;
}

void handle_ddcb_fdcb_res(Z80& cpu) {
    uint16_t addr   = ddcb_addr(cpu);
    cpu.regs.MEMPTR = addr;
    uint8_t  val    = cpu.read(addr);
    cpu.wait(2);
    val &= ~(1 << ((cpu.ddcb_opcode >> 3) & 7));
    cpu.write(addr, val);
    int reg = cpu.ddcb_opcode & 7;
    if (reg != 6) cpu.write_reg8(reg, val);
    cpu.regs.Q = 0;
}

void handle_ddcb_fdcb_set(Z80& cpu) {
    uint16_t addr   = ddcb_addr(cpu);
    cpu.regs.MEMPTR = addr;
    uint8_t  val    = cpu.read(addr);
    cpu.wait(2);
    val |= (1 << ((cpu.ddcb_opcode >> 3) & 7));
    cpu.write(addr, val);
    int reg = cpu.ddcb_opcode & 7;
    if (reg != 6) cpu.write_reg8(reg, val);
    cpu.regs.Q = 0;
}

} // namespace z80
