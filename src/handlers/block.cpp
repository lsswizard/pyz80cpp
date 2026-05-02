#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

// ============================================================
// Block Transfer (LDI / LDD / LDIR / LDDR)
//
// Timing: 16 T-states (4+4+3+5+?) = 4(M1)+4(M1 ED)+3(read)+3(write)+2(internal)
// Flags after LDI/LDD (Z80 CPU User Manual):
//   S Z C  — unaffected
//   H N    — reset
//   PV     — set if BC ≠ 0 after decrement
//   F5     — bit 1 of (A + transferred byte)  [the n+1 formula]
//   F3     — bit 3 of (A + transferred byte)
// ============================================================
static void block_transfer(Z80& cpu, int dir) {
    uint8_t  val = cpu.read(cpu.regs.HL());
    cpu.write(cpu.regs.DE(), val);
    cpu.wait(2);
    cpu.regs.set_HL(cpu.regs.HL() + dir);
    cpu.regs.set_DE(cpu.regs.DE() + dir);
    cpu.regs.set_BC(cpu.regs.BC() - 1);

    uint8_t n = cpu.regs.A + val;
    cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::C))
               | (cpu.regs.BC() != 0 ? Flags::PV : 0)
               | (n & Flags::F3)                    // bit 3 of A+val
               | ((n & 0x02) ? Flags::F5 : 0);     // bit 1 of A+val → F5
    cpu.regs.Q = cpu.regs.F;
}

// ============================================================
// Block Compare (CPI / CPD / CPIR / CPDR)
//
// Timing: 16 T-states
// Flags:
//   S Z H N — from the comparison (A - mem)
//   C       — unaffected
//   PV      — set if BC ≠ 0 after decrement
//   F5      — bit 1 of (A - mem - H)
//   F3      — bit 3 of (A - mem - H)
// MEMPTR incremented/decremented by dir each iteration.
// ============================================================
static void block_compare(Z80& cpu, int dir) {
    uint8_t  val = cpu.read(cpu.regs.HL());
    cpu.wait(5);
    cpu.regs.set_HL(cpu.regs.HL() + dir);
    cpu.regs.set_BC(cpu.regs.BC() - 1);

    uint8_t res = cpu.regs.A - val;
    uint8_t hf  = ((cpu.regs.A & 0x0F) < (val & 0x0F)) ? Flags::H : 0;
    uint8_t n   = res - (hf ? 1 : 0);   // subtract half-borrow for F3/F5

    cpu.regs.F = (res & Flags::S)
               | (res == 0 ? Flags::Z : 0)
               | hf
               | Flags::N
               | (cpu.regs.BC() != 0 ? Flags::PV : 0)
               | (cpu.regs.F & Flags::C)
               | (n & Flags::F3)
               | ((n & 0x02) ? Flags::F5 : 0);
    cpu.regs.MEMPTR = (cpu.regs.MEMPTR + dir) & 0xFFFF;
    cpu.regs.Q = cpu.regs.F;
}

void handle_ldi(Z80& cpu)  { block_transfer(cpu, +1); }
void handle_ldd(Z80& cpu)  { block_transfer(cpu, -1); }
void handle_cpi(Z80& cpu)  { block_compare(cpu, +1); }
void handle_cpd(Z80& cpu)  { block_compare(cpu, -1); }

// LDIR / LDDR: repeat while BC ≠ 0 (21 T-states looping, 16 when done)
void handle_ldir(Z80& cpu) {
    handle_ldi(cpu);
    if (cpu.regs.BC() != 0) {
        cpu.wait(5);
        cpu.regs.PC    -= 2;
        cpu.regs.MEMPTR = cpu.regs.PC + 1;
    }
}

void handle_lddr(Z80& cpu) {
    handle_ldd(cpu);
    if (cpu.regs.BC() != 0) {
        cpu.wait(5);
        cpu.regs.PC    -= 2;
        cpu.regs.MEMPTR = cpu.regs.PC + 1;
    }
}

// CPIR / CPDR: repeat while BC ≠ 0 AND A ≠ (HL)  (21 T-states looping, 16 when done)
void handle_cpir(Z80& cpu) {
    handle_cpi(cpu);
    if (!(cpu.regs.F & Flags::Z) && cpu.regs.BC() != 0) {
        cpu.wait(5);
        cpu.regs.PC    -= 2;
        cpu.regs.MEMPTR = cpu.regs.PC + 1;
    }
}

void handle_cpdr(Z80& cpu) {
    handle_cpd(cpu);
    if (!(cpu.regs.F & Flags::Z) && cpu.regs.BC() != 0) {
        cpu.wait(5);
        cpu.regs.PC    -= 2;
        cpu.regs.MEMPTR = cpu.regs.PC + 1;
    }
}

} // namespace z80
