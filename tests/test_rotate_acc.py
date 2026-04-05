#!/usr/bin/env python3
"""Accumulator rotate instructions."""
import pytest
from conftest import write_program, flag_set, flag_clear, FLAG_C, FLAG_H, FLAG_N

class TestRotateAcc:
    def test_rlca_bit7_set(self, cpu):
        cpu.regs.A = 0x88
        write_program(cpu, [0x07])
        cpu.step()
        assert cpu.regs.A == 0x11
        assert flag_set(cpu, FLAG_C)

    def test_rlca_bit7_clear(self, cpu):
        cpu.regs.A = 0x41
        write_program(cpu, [0x07])
        cpu.step()
        assert cpu.regs.A == 0x82
        assert flag_clear(cpu, FLAG_C)

    def test_rrca_bit0_set(self, cpu):
        cpu.regs.A = 0x11
        write_program(cpu, [0x0F])
        cpu.step()
        assert cpu.regs.A == 0x88
        assert flag_set(cpu, FLAG_C)

    def test_rrca_bit0_clear(self, cpu):
        cpu.regs.A = 0x82
        write_program(cpu, [0x0F])
        cpu.step()
        assert cpu.regs.A == 0x41
        assert flag_clear(cpu, FLAG_C)

    def test_rla_with_carry(self, cpu):
        cpu.regs.A = 0x41
        cpu.regs.F = FLAG_C
        write_program(cpu, [0x17])
        cpu.step()
        assert cpu.regs.A == 0x83
        assert flag_clear(cpu, FLAG_C)

    def test_rla_without_carry(self, cpu):
        cpu.regs.A = 0x80
        cpu.regs.F = 0x00
        write_program(cpu, [0x17])
        cpu.step()
        assert cpu.regs.A == 0x00
        assert flag_set(cpu, FLAG_C)

    def test_rra_no_carry(self, cpu):
        cpu.regs.A = 0x82
        cpu.regs.F = 0x00
        write_program(cpu, [0x1F])
        cpu.step()
        assert cpu.regs.A == 0x41
        assert flag_clear(cpu, FLAG_C)

    def test_rra_bit0_to_carry(self, cpu):
        cpu.regs.A = 0x01
        cpu.regs.F = 0x00
        write_program(cpu, [0x1F])
        cpu.step()
        assert cpu.regs.A == 0x00
        assert flag_set(cpu, FLAG_C)

    def test_rra_with_carry(self, cpu):
        cpu.regs.A = 0x00
        cpu.regs.F = FLAG_C
        write_program(cpu, [0x1F])
        cpu.step()
        assert cpu.regs.A == 0x80
        assert flag_clear(cpu, FLAG_C)

    def test_rlca_clears_h_n(self, cpu):
        cpu.regs.A = 0x01
        cpu.regs.F = FLAG_H | FLAG_N
        write_program(cpu, [0x07])
        cpu.step()
        assert flag_clear(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)

    def test_rrca_clears_h_n(self, cpu):
        cpu.regs.A = 0x01
        cpu.regs.F = FLAG_H | FLAG_N
        write_program(cpu, [0x0F])
        cpu.step()
        assert flag_clear(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)
