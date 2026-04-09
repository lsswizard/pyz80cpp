#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

// ============================================================
// Block Transfer Instructions - CRITICAL TIMING
// ============================================================

// ------------------------------------------------
// LDI - Load and Increment
// (HL) -> (DE), DE++, HL++, BC--
// 16 T-states
// ------------------------------------------------
void handle_ldi(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    cpu.write(cpu.regs.DE(), val);
    
    cpu.regs.set_DE(cpu.regs.DE() + 1);
    cpu.regs.set_HL(cpu.regs.HL() + 1);
    cpu.regs.set_BC(cpu.regs.BC() - 1);
    
    // Flags: N=0, H=0, PV=1 if BC != 0
    cpu.regs.F &= 0xC5;  // Clear N, H, set based on BC
    cpu.regs.F |= (cpu.regs.BC() != 0) ? Flags::PV : 0;
    cpu.regs.F |= (cpu.regs.A + val) & 0x10;  // H from addition
}

// ------------------------------------------------
// LDIR - Load and Increment, Repeat
// First iteration: 21 T-states
// Subsequent: 16 T-states (until BC=0)
// Total when complete: 21 + 16*(n-1) = 16n + 5
// ------------------------------------------------
void handle_ldir(Z80& cpu) {
    // Similar to LDI but repeats until BC=0
    uint8_t val = cpu.read(cpu.regs.HL());
    cpu.write(cpu.regs.DE(), val);
    
    cpu.regs.set_DE(cpu.regs.DE() + 1);
    cpu.regs.set_HL(cpu.regs.HL() + 1);
    cpu.regs.set_BC(cpu.regs.BC() - 1);
    
    // Flags same as LDI
    cpu.regs.F &= 0xC5;
    cpu.regs.F |= (cpu.regs.BC() != 0) ? Flags::PV : 0;
    cpu.regs.F |= (cpu.regs.A + val) & 0x10;
    
    // If BC is not zero, repeat the instruction
    // The CPU will naturally do this since PC wasn't incremented
    // But we need to account for the additional cycles
    if (cpu.regs.BC() != 0) {
        // Additional 16 T-states for repeat
        cpu.add_cycles(16);
    }
}

// ------------------------------------------------
// LDD - Load and Decrement
// (HL) -> (DE), DE--, HL--, BC--
// 16 T-states
// ------------------------------------------------
void handle_ldd(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    cpu.write(cpu.regs.DE(), val);
    
    cpu.regs.set_DE(cpu.regs.DE() - 1);
    cpu.regs.set_HL(cpu.regs.HL() - 1);
    cpu.regs.set_BC(cpu.regs.BC() - 1);
    
    // Flags: similar to LDI
    cpu.regs.F &= 0xC5;
    cpu.regs.F |= (cpu.regs.BC() != 0) ? Flags::PV : 0;
    cpu.regs.F |= (cpu.regs.A + val) & 0x10;
}

// ------------------------------------------------
// LDDR - Load and Decrement, Repeat
// First: 21 T-states, subsequent: 16 T-states
// ------------------------------------------------
void handle_lddr(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    cpu.write(cpu.regs.DE(), val);
    
    cpu.regs.set_DE(cpu.regs.DE() - 1);
    cpu.regs.set_HL(cpu.regs.HL() - 1);
    cpu.regs.set_BC(cpu.regs.BC() - 1);
    
    cpu.regs.F &= 0xC5;
    cpu.regs.F |= (cpu.regs.BC() != 0) ? Flags::PV : 0;
    cpu.regs.F |= (cpu.regs.A + val) & 0x10;
    
    if (cpu.regs.BC() != 0) {
        cpu.add_cycles(16);
    }
}

// ============================================================
// Compare and Search Instructions
// ============================================================

// ------------------------------------------------
// CPI - Compare and Increment
// A - (HL), HL++, BC--
// 16 T-states
// ------------------------------------------------
void handle_cpi(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    uint8_t result = cpu.regs.A - val;
    
    // Set flags
    cpu.regs.F = (result & 0xA8) |  // S, Z, 5, 3
                 ((result >> 8) & 0x01) |  // Carry
                 ((cpu.regs.A ^ val ^ result) & Flags::H) |  // Half borrow
                 Flags::N;  // Set N flag
    
    // P/V = 1 if BC != 0
    cpu.regs.set_HL(cpu.regs.HL() + 1);
    cpu.regs.set_BC(cpu.regs.BC() - 1);
    
    if (cpu.regs.BC() != 0) {
        cpu.regs.F |= Flags::PV;
    }
}

// ------------------------------------------------
// CPIR - Compare, Increment, Repeat
// First: 21 T-states, then 16 T-states per iteration
// ------------------------------------------------
void handle_cpir(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    uint8_t result = cpu.regs.A - val;
    
    cpu.regs.F = (result & 0xA8) |
                 ((result >> 8) & 0x01) |
                 ((cpu.regs.A ^ val ^ result) & Flags::H) |
                 Flags::N;
    
    cpu.regs.set_HL(cpu.regs.HL() + 1);
    cpu.regs.set_BC(cpu.regs.BC() - 1);
    
    // Repeat if not found and BC != 0
    if ((result != 0) && (cpu.regs.BC() != 0)) {
        cpu.add_cycles(16);
    }
}

// ------------------------------------------------
// CPD - Compare and Decrement
// A - (HL), HL--, BC--
// 16 T-states
// ------------------------------------------------
void handle_cpd(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    uint8_t result = cpu.regs.A - val;
    
    cpu.regs.F = (result & 0xA8) |
                 ((result >> 8) & 0x01) |
                 ((cpu.regs.A ^ val ^ result) & Flags::H) |
                 Flags::N;
    
    cpu.regs.set_HL(cpu.regs.HL() - 1);
    cpu.regs.set_BC(cpu.regs.BC() - 1);
    
    if (cpu.regs.BC() != 0) {
        cpu.regs.F |= Flags::PV;
    }
}

// ------------------------------------------------
// CPDR - Compare, Decrement, Repeat
// ------------------------------------------------
void handle_cpdr(Z80& cpu) {
    uint8_t val = cpu.read(cpu.regs.HL());
    uint8_t result = cpu.regs.A - val;
    
    cpu.regs.F = (result & 0xA8) |
                 ((result >> 8) & 0x01) |
                 ((cpu.regs.A ^ val ^ result) & Flags::H) |
                 Flags::N;
    
    cpu.regs.set_HL(cpu.regs.HL() - 1);
    cpu.regs.set_BC(cpu.regs.BC() - 1);
    
    if ((result != 0) && (cpu.regs.BC() != 0)) {
        cpu.add_cycles(16);
    }
}

} // namespace z80