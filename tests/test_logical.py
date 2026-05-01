#!/usr/bin/env python3
"""Comprehensive AND, OR, XOR, CP instruction tests."""
import pytest
from conftest import (
    write_program, cpu, step_n,
    FLAG_S, FLAG_Z, FLAG_F5, FLAG_H, FLAG_F3, FLAG_PV, FLAG_N, FLAG_C,
    assert_flags, flag_set, flag_clear, _parity
)


class TestAndImmediate:
    """AND n - Logical AND with immediate."""
    @pytest.mark.parametrize("a,n,expected", [
        (0xFF, 0x0F, 0x0F), (0x00, 0xFF, 0x00), (0xAA, 0x55, 0x00),
        (0xFF, 0xFF, 0xFF), (0x12, 0x34, 0x10), (0x80, 0x80, 0x80),
        (0x55, 0x55, 0x55), (0x0F, 0xF0, 0x00),
    ])
    def test_and_n(self, cpu, a, n, expected):
        """AND n — result and flag checks."""
        cpu.registers.A = a
        write_program(cpu, [0xE6, n])
        cpu.step()
        assert cpu.registers.A == expected
        assert flag_set(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)
        assert flag_clear(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_Z) == (expected == 0)
        assert flag_set(cpu, FLAG_S) == bool(expected & 0x80)
        assert flag_set(cpu, FLAG_PV) == _parity(expected)

    def test_and_n_parity_even(self, cpu):
        """AND n — PV = even parity."""
        cpu.registers.A = 0xFF
        write_program(cpu, [0xE6, 0x03])  # 0x03 has 2 (even) 1-bits
        cpu.step()
        assert flag_set(cpu, FLAG_PV)

    def test_and_n_parity_odd(self, cpu):
        """AND n — PV = odd parity."""
        cpu.registers.A = 0xFF
        write_program(cpu, [0xE6, 0x01])  # 0x01 has 1 (odd) 1-bit
        cpu.step()
        assert flag_clear(cpu, FLAG_PV)


class TestAndRegister:
    """AND r - Logical AND with register."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0xA0), ("C", 0xA1), ("D", 0xA2), ("E", 0xA3),
        ("H", 0xA4), ("L", 0xA5), ("A", 0xA7),
    ])
    def test_and_r(self, cpu, reg, opcode):
        """AND r — register operand."""
        cpu.registers.A = 0xFF
        setattr(cpu.registers, reg, 0x0F)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.registers.A == 0x0F

    def test_and_hl(self, cpu):
        """AND (HL) — memory operand."""
        cpu.registers.A = 0xFF
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x0F)
        write_program(cpu, [0xA6])
        cpu.step()
        assert cpu.registers.A == 0x0F


class TestOrImmediate:
    """OR n - Logical OR with immediate."""
    @pytest.mark.parametrize("a,n,expected", [
        (0x0F, 0xF0, 0xFF), (0x00, 0x00, 0x00), (0xAA, 0x55, 0xFF),
        (0x80, 0x00, 0x80), (0x12, 0x21, 0x33), (0x55, 0xAA, 0xFF),
        (0x0F, 0x00, 0x0F), (0xFF, 0xFF, 0xFF),
    ])
    def test_or_n(self, cpu, a, n, expected):
        """OR n — result and flag checks."""
        cpu.registers.A = a
        write_program(cpu, [0xF6, n])
        cpu.step()
        assert cpu.registers.A == expected
        assert flag_clear(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)
        assert flag_clear(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_Z) == (expected == 0)
        assert flag_set(cpu, FLAG_S) == bool(expected & 0x80)
        assert flag_set(cpu, FLAG_PV) == _parity(expected)


class TestOrRegister:
    """OR r - Logical OR with register."""
    @pytest.mark.parametrize("reg,opcode,expected", [
        ("B", 0xB0, 0xFF), ("C", 0xB1, 0xFF), ("D", 0xB2, 0xFF), ("E", 0xB3, 0xFF),
        ("H", 0xB4, 0xFF), ("L", 0xB5, 0xFF), 
        ("A", 0xB7, 0x0F),  # OR A,A = 0x0F | 0x0F = 0x0F (A was set to 0x0F, not overwritten)
    ])
    def test_or_r(self, cpu, reg, opcode, expected):
        """OR r — register operand."""
        cpu.registers.A = 0x0F
        if reg != "A":
            setattr(cpu.registers, reg, 0xF0)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.registers.A == expected

    def test_or_hl(self, cpu):
        """OR (HL) — memory operand."""
        cpu.registers.A = 0x0F
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0xF0)
        write_program(cpu, [0xB6])
        cpu.step()
        assert cpu.registers.A == 0xFF


class TestXorImmediate:
    """XOR n - Logical XOR with immediate."""
    @pytest.mark.parametrize("a,n,expected", [
        (0xFF, 0xFF, 0x00), (0xAA, 0x55, 0xFF), (0x00, 0x80, 0x80),
        (0x12, 0x12, 0x00), (0x00, 0x00, 0x00), (0xFF, 0x00, 0xFF),
        (0xAA, 0xAA, 0x00), (0x55, 0xAA, 0xFF),
    ])
    def test_xor_n(self, cpu, a, n, expected):
        """XOR n — result and flag checks."""
        cpu.registers.A = a
        write_program(cpu, [0xEE, n])
        cpu.step()
        assert cpu.registers.A == expected
        assert flag_clear(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)
        assert flag_clear(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_Z) == (expected == 0)
        assert flag_set(cpu, FLAG_S) == bool(expected & 0x80)
        assert flag_set(cpu, FLAG_PV) == _parity(expected)

    def test_xor_a_zero(self, cpu):
        """XOR A — results in zero and sets Z flag."""
        cpu.registers.A = 0xFF
        write_program(cpu, [0xAF])  # XOR A
        cpu.step()
        assert cpu.registers.A == 0x00
        assert flag_set(cpu, FLAG_Z)


class TestXorRegister:
    """XOR r - Logical XOR with register."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0xA8), ("C", 0xA9), ("D", 0xAA), ("E", 0xAB),
        ("H", 0xAC), ("L", 0xAD), ("A", 0xAF),
    ])
    def test_xor_r(self, cpu, reg, opcode):
        """XOR r — register operand."""
        cpu.registers.A = 0xFF
        setattr(cpu.registers, reg, 0xFF)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.registers.A == 0x00
        assert flag_set(cpu, FLAG_Z)

    def test_xor_hl(self, cpu):
        """XOR (HL) — memory operand."""
        cpu.registers.A = 0xAA
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0xAA)
        write_program(cpu, [0xAE])
        cpu.step()
        assert cpu.registers.A == 0x00
        assert flag_set(cpu, FLAG_Z)


class TestCpImmediate:
    """CP n - Compare with immediate."""
    def test_cp_n_equal(self, cpu):
        """CP n — equal values set Z flag."""
        cpu.registers.A = 0x10
        write_program(cpu, [0xFE, 0x10])
        cpu.step()
        assert cpu.registers.A == 0x10  # A unchanged
        assert flag_set(cpu, FLAG_Z)
        assert flag_set(cpu, FLAG_N)

    def test_cp_n_less(self, cpu):
        """CP n — A < n sets C flag."""
        cpu.registers.A = 0x05
        write_program(cpu, [0xFE, 0x10])
        cpu.step()
        assert flag_set(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_N)

    def test_cp_n_greater(self, cpu):
        """CP n — A > n clears C flag."""
        cpu.registers.A = 0x20
        write_program(cpu, [0xFE, 0x10])
        cpu.step()
        assert flag_clear(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_N)

    def test_cp_n_half_borrow(self, cpu):
        """CP n — half borrow from bit 4."""
        cpu.registers.A = 0x10
        write_program(cpu, [0xFE, 0x01])
        cpu.step()
        assert flag_set(cpu, FLAG_H)

    def test_cp_n_overflow(self, cpu):
        """CP n — overflow condition for signed comparison."""
        cpu.registers.A = 0x80
        write_program(cpu, [0xFE, 0x01])
        cpu.step()
        assert flag_set(cpu, FLAG_PV)

    @pytest.mark.parametrize("a,n,expect_z,expect_c,expect_h,expect_pv,expect_s", [
        (0x00, 0x00, True, False, False, False, False),
        (0xFF, 0x01, False, False, False, False, True),  # 0xFF - 0x01 = 0xFE, no half borrow, no carry
        (0x80, 0x80, True, False, False, False, False),
        (0x7F, 0x01, False, False, False, False, False),  # 0x7F - 0x01 = 0x7E, no overflow, no H
    ])
    def test_cp_n_flags(self, cpu, a, n, expect_z, expect_c, expect_h, expect_pv, expect_s):
        """CP n — comprehensive flag checks."""
        cpu.registers.A = a
        write_program(cpu, [0xFE, n])
        cpu.step()
        assert flag_set(cpu, FLAG_Z) == expect_z
        assert flag_set(cpu, FLAG_C) == expect_c
        assert flag_set(cpu, FLAG_H) == expect_h
        assert flag_set(cpu, FLAG_PV) == expect_pv
        assert flag_set(cpu, FLAG_S) == expect_s
        assert flag_set(cpu, FLAG_N)


class TestCpRegister:
    """CP r - Compare with register."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0xB8), ("C", 0xB9), ("D", 0xBA), ("E", 0xBB),
        ("H", 0xBC), ("L", 0xBD), ("A", 0xBF),
    ])
    def test_cp_r(self, cpu, reg, opcode):
        """CP r — compare A with register."""
        cpu.registers.A = 0x10
        setattr(cpu.registers, reg, 0x10)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.registers.A == 0x10  # A unchanged
        assert flag_set(cpu, FLAG_Z)

    def test_cp_hl(self, cpu):
        """CP (HL) — compare A with memory."""
        cpu.registers.A = 0x10
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x10)
        write_program(cpu, [0xBE])
        cpu.step()
        assert flag_set(cpu, FLAG_Z)


class TestLogicalFlags:
    """Verify flag behavior for logical operations."""
    def test_and_sets_h(self, cpu):
        """AND always sets H flag."""
        cpu.registers.A = 0xFF
        write_program(cpu, [0xE6, 0x0F])
        cpu.step()
        assert flag_set(cpu, FLAG_H)

    def test_or_clears_h(self, cpu):
        """OR always clears H flag."""
        cpu.registers.A = 0x0F
        write_program(cpu, [0xF6, 0xF0])
        cpu.step()
        assert flag_clear(cpu, FLAG_H)

    def test_xor_clears_h(self, cpu):
        """XOR always clears H flag."""
        cpu.registers.A = 0xFF
        write_program(cpu, [0xEE, 0xFF])
        cpu.step()
        assert flag_clear(cpu, FLAG_H)

    def test_cp_sets_n(self, cpu):
        """CP always sets N flag."""
        cpu.registers.A = 0x10
        write_program(cpu, [0xFE, 0x05])
        cpu.step()
        assert flag_set(cpu, FLAG_N)

    def test_logical_clears_c(self, cpu):
        """AND/OR/XOR/CP clear C flag."""
        cpu.registers.F = FLAG_C
        cpu.registers.A = 0xFF
        write_program(cpu, [0xE6, 0x0F])  # AND 0x0F
        cpu.step()
        assert flag_clear(cpu, FLAG_C)  # C cleared by AND


class TestLogicalTiming:
    """Verify cycle counts for logical instructions."""
    def test_and_n_cycles(self, cpu):
        """AND n takes 7 cycles."""
        write_program(cpu, [0xE6, 0x10])
        assert cpu.step() == 7

    def test_and_r_cycles(self, cpu):
        """AND r takes 4 cycles."""
        write_program(cpu, [0xA0])
        assert cpu.step() == 4

    def test_and_hl_cycles(self, cpu):
        """AND (HL) takes 7 cycles."""
        cpu.registers.HL = 0x2000
        write_program(cpu, [0xA6])
        assert cpu.step() == 7

    def test_or_n_cycles(self, cpu):
        """OR n takes 7 cycles."""
        write_program(cpu, [0xF6, 0x10])
        assert cpu.step() == 7

    def test_xor_n_cycles(self, cpu):
        """XOR n takes 7 cycles."""
        write_program(cpu, [0xEE, 0x10])
        assert cpu.step() == 7

    def test_cp_n_cycles(self, cpu):
        """CP n takes 7 cycles."""
        write_program(cpu, [0xFE, 0x10])
        assert cpu.step() == 7
