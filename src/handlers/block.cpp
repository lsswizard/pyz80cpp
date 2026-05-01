#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

    static inline void block_transfer(Z80& cpu, int dir) {
        uint16_t hl = cpu.regs.HL();
        uint16_t de = cpu.regs.DE();
        uint8_t val = cpu.read(hl);
        cpu.write(de, val);
        cpu.wait(2);
        uint16_t new_hl = hl + dir;
        uint16_t new_de = de + dir;
        cpu.regs.set_HL(new_hl);
        cpu.regs.set_DE(new_de);
        cpu.regs.set_BC(cpu.regs.BC() - 1);

        // MEMPTR = new DE for block transfers
        cpu.regs.MEMPTR = new_de;
        // F5/F3 come from MEMPTR high byte (new DE >> 8)
        uint8_t f5_f3 = (new_de >> 8) & (Flags::F5 | Flags::F3);
        cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::C)) | 
                     (cpu.regs.BC() != 0 ? Flags::PV : 0) | f5_f3;
    }

    static inline void block_compare(Z80& cpu, int dir) {
        uint16_t hl = cpu.regs.HL();
        uint8_t val = cpu.read(hl);
        uint8_t a = cpu.regs.A;
        uint8_t res = a - val;
        cpu.wait(5);
        
        // Calculate half-carry from ORIGINAL values before subtraction
        uint8_t hf = ((a & 0x0F) < (val & 0x0F)) ? Flags::H : 0;
        
        uint16_t new_hl = hl + dir;
        cpu.regs.set_HL(new_hl);
        cpu.regs.set_BC(cpu.regs.BC() - 1);

        // MEMPTR = new HL for compare operations
        cpu.regs.MEMPTR = new_hl;
        // F5/F3 come from MEMPTR high byte (new HL >> 8)
        uint8_t f5_f3 = (new_hl >> 8) & (Flags::F5 | Flags::F3);
        
        cpu.regs.F = (res & Flags::S) | (res == 0 ? Flags::Z : 0) | hf | Flags::N |
                    (cpu.regs.BC() != 0 ? Flags::PV : 0) | (cpu.regs.F & Flags::C) | f5_f3;
    }

    void handle_ldi(Z80& cpu)  { block_transfer(cpu, 1); }
    void handle_ldd(Z80& cpu)  { block_transfer(cpu, -1); }
    void handle_cpi(Z80& cpu)  { block_compare(cpu, 1); }
    void handle_cpd(Z80& cpu)  { block_compare(cpu, -1); }

    void handle_ldir(Z80& cpu) {
        handle_ldi(cpu);
        if (cpu.regs.BC() != 0) { cpu.wait(5); cpu.regs.PC -= 2; }
    }
    void handle_lddr(Z80& cpu) {
        handle_ldd(cpu);
        if (cpu.regs.BC() != 0) { cpu.wait(5); cpu.regs.PC -= 2; }
    }
    void handle_cpir(Z80& cpu) {
        handle_cpi(cpu);
        if (!(cpu.regs.F & Flags::Z) && cpu.regs.BC() != 0) { cpu.wait(5); cpu.regs.PC -= 2; }
    }
    void handle_cpdr(Z80& cpu) {
        handle_cpd(cpu);
        if (!(cpu.regs.F & Flags::Z) && cpu.regs.BC() != 0) { cpu.wait(5); cpu.regs.PC -= 2; }
    }

} // namespace z80
