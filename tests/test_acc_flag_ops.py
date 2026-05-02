#!/usr/bin/env python3
"""Accumulator and flag manipulation instructions."""
from conftest import write_program, step_n, flag_set, flag_clear, FLAG_H, FLAG_N, FLAG_C, FLAG_Z, FLAG_PV

class TestAccFlagOps:
    def test_cpl(self, cpu):
        cpu.registers.A = 0x3C
        write_program(cpu, [0x2F])
        cpu.step()
        assert cpu.registers.A == 0xC3
        assert flag_set(cpu, FLAG_H)
        assert flag_set(cpu, FLAG_N)

    def test_cpl_zero(self, cpu):
        cpu.registers.A = 0x00
        write_program(cpu, [0x2F])
        cpu.step()
        assert cpu.registers.A == 0xFF

    def test_cpl_ff(self, cpu):
        cpu.registers.A = 0xFF
        write_program(cpu, [0x2F])
        cpu.step()
        assert cpu.registers.A == 0x00

    def test_neg_positive(self, cpu):
        cpu.registers.A = 0x01
        write_program(cpu, [0xED, 0x44])
        cpu.step()
        assert cpu.registers.A == 0xFF
        assert flag_set(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_N)

    def test_neg_zero(self, cpu):
        cpu.registers.A = 0x00
        write_program(cpu, [0xED, 0x44])
        cpu.step()
        assert cpu.registers.A == 0x00
        assert flag_set(cpu, FLAG_Z)
        assert flag_clear(cpu, FLAG_C)

    def test_neg_0x80_overflow(self, cpu):
        cpu.registers.A = 0x80
        write_program(cpu, [0xED, 0x44])
        cpu.step()
        assert cpu.registers.A == 0x80
        assert flag_set(cpu, FLAG_PV)

    def test_scf(self, cpu):
        cpu.registers.F = 0x00
        write_program(cpu, [0x37])
        cpu.step()
        assert flag_set(cpu, FLAG_C)
        assert flag_clear(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)

    def test_ccf_invert_set(self, cpu):
        cpu.registers.F = FLAG_C
        write_program(cpu, [0x3F])
        cpu.step()
        assert flag_clear(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_H)

    def test_ccf_invert_clear(self, cpu):
        cpu.registers.F = 0x00
        write_program(cpu, [0x3F])
        cpu.step()
        assert flag_set(cpu, FLAG_C)

    def test_daa_add_09_01(self, cpu):
        cpu.registers.A = 0x09
        write_program(cpu, [0xC6, 0x01, 0x27])
        step_n(cpu, 2)
        assert cpu.registers.A == 0x10

    def test_daa_add_09_09(self, cpu):
        cpu.registers.A = 0x09
        write_program(cpu, [0xC6, 0x09, 0x27])
        step_n(cpu, 2)
        assert cpu.registers.A == 0x18

    def test_daa_add_99_01(self, cpu):
        cpu.registers.A = 0x99
        write_program(cpu, [0xC6, 0x01, 0x27])
        step_n(cpu, 2)
        assert cpu.registers.A == 0x00
        assert flag_set(cpu, FLAG_C)

    def test_daa_sub(self, cpu):
        cpu.registers.A = 0x10
        write_program(cpu, [0xD6, 0x01, 0x27])
        step_n(cpu, 2)
        assert cpu.registers.A == 0x09
