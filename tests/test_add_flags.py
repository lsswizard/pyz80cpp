#!/usr/bin/env python3
"""ADD A,n and ADC A,n flag verification."""

import pytest
from conftest import write_program, _add_flags, flag_set, flag_clear, FLAG_S, FLAG_Z, FLAG_H, FLAG_PV, FLAG_C, FLAG_N


class TestAddFlags:
    """ADD A,n and ADC A,n flag verification."""

    @pytest.mark.parametrize(
        "a,b",
        [
            (0x00, 0x00),
            (0x01, 0x02),
            (0x7F, 0x01),
            (0xFF, 0x01),
            (0x80, 0x80),
            (0x0F, 0x01),
            (0xF0, 0x10),
            (0x55, 0xAA),
            (0x01, 0xFF),
            (0x40, 0x40),
            (0xFE, 0x01),
        ],
    )
    def test_add_a_n_flags(self, cpu, a, b):
        """ADD A,n — verify result and all affected flags."""
        write_program(cpu, [0x3E, a, 0xC6, b])
        cpu.step()
        cpu.step()
        expected_flags = _add_flags(a, b)
        mask = FLAG_S | FLAG_Z | FLAG_H | FLAG_PV | FLAG_C | FLAG_N
        assert cpu.regs.A == (a + b) & 0xFF
        assert (cpu.regs.F & mask) == (expected_flags & mask)

    def test_adc_with_carry(self, cpu):
        """ADC A,n — carry input is included in addition."""
        cpu.regs.A = 0x0F
        cpu.regs.F = FLAG_C
        write_program(cpu, [0xCE, 0x01])
        cpu.step()
        assert cpu.regs.A == 0x11
        assert flag_set(cpu, FLAG_H)

    def test_adc_no_carry(self, cpu):
        """ADC A,n — without carry behaves like ADD."""
        cpu.regs.A = 0x10
        cpu.regs.F = 0
        write_program(cpu, [0xCE, 0x05])
        cpu.step()
        assert cpu.regs.A == 0x15

    def test_adc_carry_causes_overflow(self, cpu):
        """ADC A,n — carry can trigger overflow."""
        cpu.regs.A = 0x7F
        cpu.regs.F = FLAG_C
        write_program(cpu, [0xCE, 0x00])
        cpu.step()
        assert cpu.regs.A == 0x80
        assert flag_clear(cpu, FLAG_PV)
        assert flag_set(cpu, FLAG_S)

    @pytest.mark.parametrize(
        "reg,opcode",
        [
            ("B", 0x80),
            ("C", 0x81),
            ("D", 0x82),
            ("E", 0x83),
            ("H", 0x84),
            ("L", 0x85),
        ],
    )
    def test_add_a_r(self, cpu, reg, opcode):
        """ADD A,r — add register to A."""
        cpu.regs.A = 0x10
        setattr(cpu.regs, reg, 0x05)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.regs.A == 0x15

    def test_add_a_hl_indirect(self, cpu):
        """ADD A,(HL) — add memory byte to A."""
        cpu.regs.A = 0x10
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0x20)
        write_program(cpu, [0x86])
        cpu.step()
        assert cpu.regs.A == 0x30

    def test_add_a_a(self, cpu):
        """ADD A,A — double the accumulator."""
        cpu.regs.A = 0x40
        write_program(cpu, [0x87])
        cpu.step()
        assert cpu.regs.A == 0x80
        assert flag_clear(cpu, FLAG_PV)
