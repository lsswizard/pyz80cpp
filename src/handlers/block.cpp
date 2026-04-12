#include "../../include/z80/z80.h"
#include "../../include/z80/flags.h"

namespace z80 {

    static inline void block_transfer(Z80& cpu, int dir) {
        uint16_t hl = cpu.regs.HL();
        uint16_t de = cpu.regs.DE();
        uint8_t val = cpu.read(hl);
        cpu.write(de, val);
        cpu.wait(2);
        cpu.regs.set_HL(hl + dir);
        cpu.regs.set_DE(de + dir);
        cpu.regs.set_BC(cpu.regs.BC() - 1);

        uint8_t n = val + cpu.regs.A;
        cpu.regs.F = (cpu.regs.F & (Flags::S | Flags::Z | Flags::C)) | (cpu.regs.BC() != 0 ? Flags::PV : 0) | (n & Flags::F3) | ((n & 0x02) ? Flags::F5 : 0);
    }

    static inline void block_compare(Z80& cpu, int dir) {
        uint16_t hl = cpu.regs.HL();
        uint8_t val = cpu.read(hl);
        uint8_t res = cpu.regs.A - val;
        cpu.wait(5);
        cpu.regs.set_HL(hl + dir);
        cpu.regs.set_BC(cpu.regs.BC() - 1);

        uint8_t hf = ((cpu.regs.A & 0x0F) < (val & 0x0F)) ? Flags::H : 0;
        uint8_t n = res - (hf ? 1 : 0);
        cpu.regs.F = (res & Flags::S) | (res == 0 ? Flags::Z : 0) | hf | Flags::N |
        (cpu.regs.BC() != 0 ? Flags::PV : 0) | (cpu.regs.F & Flags::C) |
        (n & Flags::F3) | ((n & 0x02) ? Flags::F5 : 0);
        cpu.regs.MEMPTR = (cpu.regs.MEMPTR + dir) & 0xFFFF;
    }

    void handle_ldi(Z80& cpu)  { block_transfer(cpu, 1); }
    void handle_ldd(Z80& cpu)  { block_transfer(cpu, -1); }
    void handle_cpi(Z80& cpu)  { block_compare(cpu, 1); }
    void handle_cpd(Z80& cpu)  { block_compare(cpu, -1); }

    void handle_ldir(Z80& cpu) {
        handle_ldi(cpu);
        if (cpu.regs.BC() != 0) { cpu.wait(5); cpu.regs.PC -= 2; cpu.regs.MEMPTR = cpu.regs.PC + 1; }
    }
    void handle_lddr(Z80& cpu) {
        handle_ldd(cpu);
        if (cpu.regs.BC() != 0) { cpu.wait(5); cpu.regs.PC -= 2; cpu.regs.MEMPTR = cpu.regs.PC + 1; }
    }
    void handle_cpir(Z80& cpu) {
        handle_cpi(cpu);
        if (!(cpu.regs.F & Flags::Z) && cpu.regs.BC() != 0) { cpu.wait(5); cpu.regs.PC -= 2; cpu.regs.MEMPTR = cpu.regs.PC + 1; }
    }
    void handle_cpdr(Z80& cpu) {
        handle_cpd(cpu);
        if (!(cpu.regs.F & Flags::Z) && cpu.regs.BC() != 0) { cpu.wait(5); cpu.regs.PC -= 2; cpu.regs.MEMPTR = cpu.regs.PC + 1; }
    }

} // namespace z80
