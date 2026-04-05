#!/usr/bin/env python3
"""ED instruction tests."""

import pytest
from conftest import (
    write_program,
    flag_set,
    flag_clear,
    FLAG_Z,
    FLAG_C,
    FLAG_PV,
    FLAG_N,
    FLAG_S,
    FLAG_H,
)


class TestEDInstructions:
    def test_ld_i_a(self, cpu):
        cpu.regs.A = 0x42
        write_program(cpu, [0xED, 0x47])
        cpu.step()
        assert cpu.regs.I == 0x42

    def test_ld_r_a(self, cpu):
        cpu.regs.A = 0x42
        write_program(cpu, [0xED, 0x4F])
        cpu.step()
        assert cpu.regs.R == 0x42

    def test_ld_a_i(self, cpu):
        cpu.regs.I = 0x42
        cpu.regs.IFF2 = True
        write_program(cpu, [0xED, 0x57])
        cpu.step()
        assert cpu.regs.A == 0x42
        assert flag_set(cpu, FLAG_PV)

    def test_ld_a_r(self, cpu):
        cpu.regs.R = 0x42
        cpu.regs.IFF2 = False
        write_program(cpu, [0xED, 0x5F])
        cpu.step()
        assert cpu.regs.A == 0x44
        assert flag_clear(cpu, FLAG_PV)

    def test_rld(self, cpu):
        cpu.regs.A = 0x12
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0x34)
        write_program(cpu, [0xED, 0x6F])
        cpu.step()
        assert cpu.regs.A == 0x13
        assert cpu.read_byte(0x2000) == 0x42

    def test_rrd(self, cpu):
        cpu.regs.A = 0x12
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0x34)
        write_program(cpu, [0xED, 0x67])
        cpu.step()
        assert cpu.regs.A == 0x14
        assert cpu.read_byte(0x2000) == 0x23

    def test_neg(self, cpu):
        cpu.regs.A = 0x01
        write_program(cpu, [0xED, 0x44])
        cpu.step()
        assert cpu.regs.A == 0xFF
        assert flag_set(cpu, FLAG_C)

    def test_neg_zero(self, cpu):
        cpu.regs.A = 0x00
        write_program(cpu, [0xED, 0x44])
        cpu.step()
        assert cpu.regs.A == 0x00
        assert flag_set(cpu, FLAG_Z)
        assert flag_clear(cpu, FLAG_C)

    def test_neg_0x80(self, cpu):
        cpu.regs.A = 0x80
        write_program(cpu, [0xED, 0x44])
        cpu.step()
        assert cpu.regs.A == 0x80
        assert flag_set(cpu, FLAG_PV)

    def test_im_0(self, cpu):
        write_program(cpu, [0xED, 0x46])
        cpu.step()
        assert cpu.regs.IM == 0

    def test_im_1(self, cpu):
        write_program(cpu, [0xED, 0x56])
        cpu.step()
        assert cpu.regs.IM == 1

    def test_im_2(self, cpu):
        write_program(cpu, [0xED, 0x5E])
        cpu.step()
        assert cpu.regs.IM == 2
