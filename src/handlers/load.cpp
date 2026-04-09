#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

// ============================================================
// Load and Exchange Instructions
// ============================================================

// ------------------------------------------------
// LD r, r' - 4 T-states
// ------------------------------------------------
void handle_ld_r_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t dest = (opcode >> 3) & 7;
    uint8_t src = opcode & 7;
    uint8_t val = cpu.read_reg8(src);
    cpu.write_reg8(dest, val);
}

// ------------------------------------------------
// LD r, n - 7 T-states
// ------------------------------------------------
void handle_ld_r_n(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t dest = (opcode >> 3) & 7;
    uint8_t val = cpu.fetch();
    cpu.write_reg8(dest, val);
}

// ------------------------------------------------
// LD r, (HL) - 7 T-states
// ------------------------------------------------
void handle_ld_r_hl(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t dest = (opcode >> 3) & 7;
    uint8_t val = cpu.read(cpu.get_registers().HL());
    cpu.write_reg8(dest, val);
}

// ------------------------------------------------
// LD (HL), r - 7 T-states
// ------------------------------------------------
void handle_ld_hl_r(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint8_t src = opcode & 7;
    uint8_t val = cpu.read_reg8(src);
    cpu.write(cpu.get_registers().HL(), val);
}

// ------------------------------------------------
// LD (HL), n - 10 T-states
// ------------------------------------------------
void handle_ld_hl_n(Z80& cpu) {
    uint8_t val = cpu.fetch();
    cpu.write(cpu.get_registers().HL(), val);
}

// ------------------------------------------------
// LD A, (BC) - 7 T-states
// ------------------------------------------------
void handle_ld_a_bc(Z80& cpu) {
    auto& regs = cpu.get_registers();
    uint16_t addr = regs.BC();
    cpu.regs.A = cpu.read(addr);
    regs.MEMPTR = (addr + 1) & 0xFFFF;
}

// ------------------------------------------------
// LD A, (DE) - 7 T-states
// ------------------------------------------------
void handle_ld_a_de(Z80& cpu) {
    auto& regs = cpu.get_registers();
    uint16_t addr = regs.DE();
    cpu.regs.A = cpu.read(addr);
    regs.MEMPTR = (addr + 1) & 0xFFFF;
}

// ------------------------------------------------
// LD A, (nn) - 13 T-states
// ------------------------------------------------
void handle_ld_a_nn(Z80& cpu) {
    uint16_t addr = cpu.fetch();
    addr |= (cpu.fetch() << 8);
    cpu.regs.A = cpu.read(addr);
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
}

// ------------------------------------------------
// LD (BC), A - 7 T-states
// ------------------------------------------------
void handle_ld_bc_a(Z80& cpu) {
    auto& regs = cpu.get_registers();
    uint16_t addr = regs.BC();
    cpu.write(addr, regs.A);
    regs.MEMPTR = (addr + 1) & 0xFFFF;
}

// ------------------------------------------------
// LD (DE), A - 7 T-states
// ------------------------------------------------
void handle_ld_de_a(Z80& cpu) {
    auto& regs = cpu.get_registers();
    uint16_t addr = regs.DE();
    cpu.write(addr, regs.A);
    regs.MEMPTR = (addr + 1) & 0xFFFF;
}

// ------------------------------------------------
// LD (nn), A - 13 T-states
// ------------------------------------------------
void handle_ld_nn_a(Z80& cpu) {
    uint16_t addr = cpu.fetch();
    addr |= (cpu.fetch() << 8);
    cpu.write(addr, cpu.regs.A);
    cpu.regs.MEMPTR = (addr + 1) & 0xFFFF;
}

// ------------------------------------------------
// LD rr, nn - 10 T-states
// ------------------------------------------------
void handle_ld_rr_nn(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint16_t val = cpu.fetch();
    val |= (cpu.fetch() << 8);
    
    int reg = (opcode >> 4) & 3;  // B=0, D=1, H=2, SP=3
    switch (reg) {
        case 0: cpu.regs.set_BC(val); break;
        case 1: cpu.regs.set_DE(val); break;
        case 2: cpu.regs.set_HL(val); break;
        case 3: cpu.regs.SP = val; break;
    }
}

// ------------------------------------------------
// LD HL, nn - 16 T-states
// ------------------------------------------------
void handle_ld_hl_nn(Z80& cpu) {
    uint16_t val = cpu.fetch();
    val |= (cpu.fetch() << 8);
    cpu.regs.set_HL(val);
}

// ------------------------------------------------
// LD (nn), HL - 16 T-states
// ------------------------------------------------
void handle_ld_nn_hl(Z80& cpu) {
    uint16_t addr = cpu.fetch();
    addr |= (cpu.fetch() << 8);
    cpu.write(addr++, cpu.regs.L);
    cpu.write(addr, cpu.regs.H);
}

// ------------------------------------------------
// LD SP, HL - 6 T-states
// ------------------------------------------------
void handle_ld_sp_hl(Z80& cpu) {
    cpu.regs.SP = cpu.regs.HL();
}

// ------------------------------------------------
// PUSH rr - 11 T-states
// ------------------------------------------------
void handle_push_rr(Z80& cpu) {
    uint8_t opcode = cpu.current_opcode;
    uint16_t val;
    switch ((opcode >> 4) & 3) {
        case 0: val = cpu.regs.BC(); break;
        case 1: val = cpu.regs.DE(); break;
        case 2: val = cpu.regs.HL(); break;
        case 3: val = cpu.regs.AF(); break;
    }
    cpu.push(val);
}

// ------------------------------------------------
// POP rr - 10 T-states
// ------------------------------------------------
void handle_pop_rr(Z80& cpu) {
    uint16_t val = cpu.pop();
    uint8_t opcode = cpu.current_opcode;
    switch ((opcode >> 4) & 3) {
        case 0: cpu.regs.set_BC(val); break;
        case 1: cpu.regs.set_DE(val); break;
        case 2: cpu.regs.set_HL(val); break;
        case 3: cpu.regs.set_AF(val); break;
    }
}

// ============================================================
// EXCHANGE INSTRUCTIONS - CRITICAL TIMING FIXES
// ============================================================

// ------------------------------------------------
// EX DE, HL - 4 T-states
// ------------------------------------------------
void handle_ex_de_hl(Z80& cpu) {
    uint16_t temp = cpu.regs.DE();
    cpu.regs.set_DE(cpu.regs.HL());
    cpu.regs.set_HL(temp);
}

// ------------------------------------------------
// EX AF, AF' - 4 T-states
// ------------------------------------------------
void handle_ex_af_afp(Z80& cpu) {
    cpu.regs.swap_af();
}

// ------------------------------------------------
// EXX - 4 T-states
// ------------------------------------------------
void handle_exx(Z80& cpu) {
    cpu.regs.swap_all();
}

// ------------------------------------------------
// EX (SP), HL - CRITICAL: 19 T-states (NOT 15!)
// Timing breakdown:
//   - Read low byte from stack: 3 T-states
//   - Contention for first read: varies
//   - Read high byte from stack: 3 T-states  
//   - Write low byte to stack: 3 T-states
//   - Write high byte to stack: 3 T-states
//   - Plus overhead
// Total: 19 T-states
// ------------------------------------------------
void handle_ex_sp_hl(Z80& cpu) {
    uint16_t sp = cpu.regs.SP;
    
    // Read from stack (HL is not modified yet)
    // The timing is critical: we read the old value first
    // then write HL to stack
    
    // First read from stack (the value that will go to HL)
    uint8_t lo = cpu.read(sp);
    uint8_t hi = cpu.read(sp + 1);
    uint16_t temp = (hi << 8) | lo;
    
    // Now write HL to stack
    // Write high byte first (at SP+1)
    cpu.write(sp + 1, (cpu.regs.HL() >> 8) & 0xFF);
    // Write low byte (at SP)
    cpu.write(sp, cpu.regs.HL() & 0xFF);
    
    // Update HL with the value read from stack
    cpu.regs.set_HL(temp);
    
    // MEMPTR is set to the address of the low byte read
    cpu.regs.MEMPTR = sp;
}

} // namespace z80