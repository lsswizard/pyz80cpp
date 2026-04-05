#!/usr/bin/env python3
"""AND, OR, XOR instruction tests."""

import pytest
from conftest import write_program, flag_set, flag_clear, _parity, FLAG_H, FLAG_N, FLAG_C, FLAG_Z, FLAG_S, FLAG_PV


class TestLogical:
    """AND, OR, XOR instruction tests."""

    @pytest.mark.parametrize(
        "a,n,expected",
        [
            (0xFF, 0x0F, 0x0F),
            (0x00, 0xFF, 0x00),
            (0xAA, 0x55, 0x00),
            (0xFF, 0xFF, 0xFF),
            (0x12, 0x34, 0x10),
        ],
    )
    def test_and_n(self, cpu, a, n, expected):
        """AND n — result and flag checks."""
        cpu.regs.A = a
        write_program(cpu, [0xE6, n])
        cpu.step()
        assert cpu.regs.A == expected
        assert flag_set(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)
        assert flag_clear(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_Z) == (expected == 0)
        assert flag_set(cpu, FLAG_S) == bool(expected & 0x80)
        assert flag_set(cpu, FLAG_PV) == _parity(expected)

    @pytest.mark.parametrize(
        "a,n,expected",
        [
            (0x0F, 0xF0, 0xFF),
            (0x00, 0x00, 0x00),
            (0xAA, 0x55, 0xFF),
            (0x80, 0x00, 0x80),
        ],
    )
    def test_or_n(self, cpu, a, n, expected):
        """OR n — result and flag checks."""
        cpu.regs.A = a
        write_program(cpu, [0xF6, n])
        cpu.step()
        assert cpu.regs.A == expected
        assert flag_clear(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)
        assert flag_clear(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_Z) == (expected == 0)
        assert flag_set(cpu, FLAG_S) == bool(expected & 0x80)

    @pytest.mark.parametrize(
        "a,n,expected",
        [
            (0xFF, 0xFF, 0x00),
            (0xAA, 0x55, 0xFF),
            (0x00, 0x80, 0x80),
            (0x12, 0x12, 0x00),
            (0x00, 0x00, 0x00),
        ],
    )
    def test_xor_n(self, cpu, a, n, expected):
        """XOR n — result and flag checks."""
        cpu.regs.A = a
        write_program(cpu, [0xEE, n])
        cpu.step()
        assert cpu.regs.A == expected
        assert flag_clear(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)
        assert flag_clear(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_Z) == (expected == 0)

    @pytest.mark.parametrize(
        "reg,opcode",
        [
            ("B", 0xA0),
            ("C", 0xA1),
            ("D", 0xA2),
            ("E", 0xA3),
            ("H", 0xA4),
            ("L", 0xA5),
        ],
    )
    def test_and_r(self, cpu, reg, opcode):
        """AND r — register operand."""
        cpu.regs.A = 0xFF
        setattr(cpu.regs, reg, 0x0F)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.regs.A == 0x0F

    @pytest.mark.parametrize(
        "reg,opcode",
        [
            ("B", 0xB0),
            ("C", 0xB1),
            ("D", 0xB2),
            ("E", 0xB3),
            ("H", 0xB4),
            ("L", 0xB5),
        ],
    )
    def test_or_r(self, cpu, reg, opcode):
        """OR r — register operand."""
        cpu.regs.A = 0x0F
        setattr(cpu.regs, reg, 0xF0)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.regs.A == 0xFF

    @pytest.mark.parametrize(
        "reg,opcode",
        [
            ("B", 0xA8),
            ("C", 0xA9),
            ("D", 0xAA),
            ("E", 0xAB),
            ("H", 0xAC),
            ("L", 0xAD),
        ],
    )
    def test_xor_r(self, cpu, reg, opcode):
        """XOR r — register operand."""
        cpu.regs.A = 0xFF
        setattr(cpu.regs, reg, 0xFF)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.regs.A == 0x00
        assert flag_set(cpu, FLAG_Z)

    def test_and_hl_indirect(self, cpu):
        """AND (HL) — memory operand."""
        cpu.regs.A = 0xFF
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0x0F)
        write_program(cpu, [0xA6])
        cpu.step()
        assert cpu.regs.A == 0x0F

    def test_or_hl_indirect(self, cpu):
        """OR (HL) — memory operand."""
        cpu.regs.A = 0x0F
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0xF0)
        write_program(cpu, [0xB6])
        cpu.step()
        assert cpu.regs.A == 0xFF

    def test_xor_hl_indirect(self, cpu):
        """XOR (HL) — memory operand."""
        cpu.regs.A = 0xAA
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0xAA)
        write_program(cpu, [0xAE])
        cpu.step()
        assert cpu.regs.A == 0x00
