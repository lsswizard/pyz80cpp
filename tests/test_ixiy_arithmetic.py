#!/usr/bin/env python3
"""IX/IY arithmetic with IXH/IXL tests."""
import pytest
from conftest import write_program, flag_set, flag_clear, FLAG_Z, FLAG_C, FLAG_S

class TestIXIYArithmetic:
    def test_add_a_ixh(self, cpu):
        cpu.registers.A = 0x10
        cpu.registers.IX = 0x2000
        write_program(cpu, [0xDD, 0x84])
        cpu.step()
        assert cpu.registers.A == 0x30

    def test_add_a_ixl(self, cpu):
        cpu.registers.A = 0x10
        cpu.registers.IX = 0x0020
        write_program(cpu, [0xDD, 0x85])
        cpu.step()
        assert cpu.registers.A == 0x30

    def test_sub_a_ixh(self, cpu):
        cpu.registers.A = 0x30
        cpu.registers.IX = 0x2000
        write_program(cpu, [0xDD, 0x94])
        cpu.step()
        assert cpu.registers.A == 0x10

    def test_sub_a_ixl(self, cpu):
        cpu.registers.A = 0x30
        cpu.registers.IX = 0x0020
        write_program(cpu, [0xDD, 0x95])
        cpu.step()
        assert cpu.registers.A == 0x10

    def test_and_a_ixh(self, cpu):
        cpu.registers.A = 0xFF
        cpu.registers.IX = 0x0F00
        write_program(cpu, [0xDD, 0xA4])
        cpu.step()
        assert cpu.registers.A == 0x0F

    def test_xor_a_ixh(self, cpu):
        cpu.registers.A = 0xFF
        cpu.registers.IX = 0xF000
        write_program(cpu, [0xDD, 0xAC])
        cpu.step()
        assert cpu.registers.A == 0x0F

    def test_or_a_ixh(self, cpu):
        cpu.registers.A = 0x00
        cpu.registers.IX = 0x0F00
        write_program(cpu, [0xDD, 0xB4])
        cpu.step()
        assert cpu.registers.A == 0x0F

    def test_cp_a_ixh(self, cpu):
        cpu.registers.A = 0x20
        cpu.registers.IX = 0x2000
        write_program(cpu, [0xDD, 0xBC])
        cpu.step()
        assert flag_set(cpu, FLAG_Z)
        assert cpu.registers.A == 0x20

    def test_iy_arithmetic(self, cpu):
        cpu.registers.A = 0x10
        cpu.registers.IY = 0x2000
        write_program(cpu, [0xFD, 0x84])
        cpu.step()
        assert cpu.registers.A == 0x30
