#!/usr/bin/env python3
"""8-bit increment and decrement tests."""

import pytest
from conftest import write_program, flag_set, flag_clear, FLAG_C, FLAG_S, FLAG_Z, FLAG_H, FLAG_PV, FLAG_N


class TestIncDec8Bit:
    """8-bit increment and decrement tests."""

    @pytest.mark.parametrize(
        "val,exp_result,exp_s,exp_z,exp_h,exp_pv",
        [
            (0x00, 0x01, False, False, False, False),
            (0x7F, 0x80, True, False, True, True),
            (0xFF, 0x00, False, True, True, False),
            (0x0F, 0x10, False, False, True, False),
            (0xFE, 0xFF, True, False, False, False),
        ],
    )
    def test_inc_a(self, cpu, val, exp_result, exp_s, exp_z, exp_h, exp_pv):
        """INC A — verify result and flags."""
        cpu.regs.A = val
        cpu.regs.F = FLAG_C
        write_program(cpu, [0x3C])
        cpu.step()
        assert cpu.regs.A == exp_result
        assert flag_set(cpu, FLAG_S) == exp_s
        assert flag_set(cpu, FLAG_Z) == exp_z
        assert flag_set(cpu, FLAG_H) == exp_h
        assert flag_set(cpu, FLAG_PV) == exp_pv
        assert flag_set(cpu, FLAG_C)
        assert flag_clear(cpu, FLAG_N)

    @pytest.mark.parametrize(
        "val,exp_result,exp_s,exp_z,exp_h,exp_pv",
        [
            (0x01, 0x00, False, True, False, False),
            (0x80, 0x7F, False, False, True, True),
            (0x00, 0xFF, True, False, True, False),
            (0x10, 0x0F, False, False, True, False),
            (0x02, 0x01, False, False, False, False),
        ],
    )
    def test_dec_a(self, cpu, val, exp_result, exp_s, exp_z, exp_h, exp_pv):
        """DEC A — verify result and flags."""
        cpu.regs.A = val
        cpu.regs.F = FLAG_C
        write_program(cpu, [0x3D])
        cpu.step()
        assert cpu.regs.A == exp_result
        assert flag_set(cpu, FLAG_S) == exp_s
        assert flag_set(cpu, FLAG_Z) == exp_z
        assert flag_set(cpu, FLAG_H) == exp_h
        assert flag_set(cpu, FLAG_PV) == exp_pv
        assert flag_set(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_N)

    @pytest.mark.parametrize(
        "reg,inc_op,dec_op",
        [
            ("B", 0x04, 0x05),
            ("C", 0x0C, 0x0D),
            ("D", 0x14, 0x15),
            ("E", 0x1C, 0x1D),
            ("H", 0x24, 0x25),
            ("L", 0x2C, 0x2D),
        ],
    )
    def test_inc_dec_registers(self, cpu, reg, inc_op, dec_op):
        """INC r / DEC r — basic operation on all registers."""
        setattr(cpu.regs, reg, 0x10)
        write_program(cpu, [inc_op])
        cpu.step()
        assert getattr(cpu.regs, reg) == 0x11

        cpu.reset()
        setattr(cpu.regs, reg, 0x10)
        write_program(cpu, [dec_op])
        cpu.step()
        assert getattr(cpu.regs, reg) == 0x0F

    def test_inc_hl_indirect(self, cpu):
        """INC (HL) — increment memory byte."""
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0x0F)
        write_program(cpu, [0x34])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x10
        assert flag_set(cpu, FLAG_H)

    def test_dec_hl_indirect(self, cpu):
        """DEC (HL) — decrement memory byte."""
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0x10)
        write_program(cpu, [0x35])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x0F
        assert flag_set(cpu, FLAG_H)
