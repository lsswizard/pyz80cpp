#!/usr/bin/env python3
"""Block transfer and compare instruction tests."""

import pytest
from conftest import (
    write_program,
    flag_set,
    flag_clear,
    FLAG_Z,
    FLAG_PV,
    FLAG_C,
    step_n,
)


class TestBlockInstructions:
    def test_ldi(self, cpu):
        cpu.regs.HL = 0x2000
        cpu.regs.DE = 0x3000
        cpu.regs.BC = 0x0001
        cpu.write_byte(0x2000, 0xAB)
        write_program(cpu, [0xED, 0xA0])
        cpu.step()
        assert cpu.read_byte(0x3000) == 0xAB
        assert cpu.regs.HL == 0x2001
        assert cpu.regs.DE == 0x3001
        assert cpu.regs.BC == 0x0000
        assert flag_clear(cpu, FLAG_PV)

    def test_ldd(self, cpu):
        cpu.regs.HL = 0x2000
        cpu.regs.DE = 0x3000
        cpu.regs.BC = 0x0001
        cpu.write_byte(0x2000, 0xAB)
        write_program(cpu, [0xED, 0xA8])
        cpu.step()
        assert cpu.read_byte(0x3000) == 0xAB
        assert cpu.regs.HL == 0x1FFF
        assert cpu.regs.DE == 0x2FFF
        assert cpu.regs.BC == 0x0000

    def test_ldir(self, cpu):
        cpu.regs.HL = 0x2000
        cpu.regs.DE = 0x3000
        cpu.regs.BC = 0x0002
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        write_program(cpu, [0xED, 0xB0])
        step_n(cpu, 2)
        assert cpu.regs.BC == 0x0000
        assert cpu.read_byte(0x3000) == 0xAA
        assert cpu.read_byte(0x3001) == 0xBB

    def test_lddr(self, cpu):
        cpu.regs.HL = 0x2001
        cpu.regs.DE = 0x3001
        cpu.regs.BC = 0x0002
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        write_program(cpu, [0xED, 0xB8])
        step_n(cpu, 2)
        assert cpu.regs.BC == 0x0000
        assert cpu.read_byte(0x3000) == 0xAA
        assert cpu.read_byte(0x3001) == 0xBB

    def test_cpi(self, cpu):
        cpu.regs.A = 0xAA
        cpu.regs.HL = 0x2000
        cpu.regs.BC = 0x0001
        cpu.write_byte(0x2000, 0xAA)
        write_program(cpu, [0xED, 0xA1])
        cpu.step()
        assert flag_set(cpu, FLAG_Z)
        assert cpu.regs.HL == 0x2001
        assert cpu.regs.BC == 0x0000

    def test_cpd(self, cpu):
        cpu.regs.A = 0xAA
        cpu.regs.HL = 0x2000
        cpu.regs.BC = 0x0001
        cpu.write_byte(0x2000, 0xAA)
        write_program(cpu, [0xED, 0xA9])
        cpu.step()
        assert flag_set(cpu, FLAG_Z)
        assert cpu.regs.HL == 0x1FFF
        assert cpu.regs.BC == 0x0000

    def test_cpir(self, cpu):
        cpu.regs.A = 0xBB
        cpu.regs.HL = 0x2000
        cpu.regs.BC = 0x0003
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        cpu.write_byte(0x2002, 0xCC)
        write_program(cpu, [0xED, 0xB1])
        step_n(cpu, 2)
        assert cpu.regs.BC == 0x0001
        assert flag_set(cpu, FLAG_Z)

    def test_cpdr(self, cpu):
        cpu.regs.A = 0xBB
        cpu.regs.HL = 0x2002
        cpu.regs.BC = 0x0003
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        cpu.write_byte(0x2002, 0xCC)
        write_program(cpu, [0xED, 0xB9])
        step_n(cpu, 2)
        assert cpu.regs.BC == 0x0001
