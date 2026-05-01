#!/usr/bin/env python3
"""Comprehensive 8-bit increment and decrement tests."""
import pytest
from conftest import (
    write_program, cpu, step_n,
    FLAG_S, FLAG_Z, FLAG_F5, FLAG_H, FLAG_F3, FLAG_PV, FLAG_N, FLAG_C,
    assert_flags, flag_set, flag_clear
)


class TestIncRegister:
    """INC r - Increment register."""
    @pytest.mark.parametrize("reg,opcode", [
        ("A", 0x3C), ("B", 0x04), ("C", 0x0C), ("D", 0x14),
        ("E", 0x1C), ("H", 0x24), ("L", 0x2C),
    ])
    def test_inc_r_basic(self, cpu, reg, opcode):
        """INC r — basic increment."""
        setattr(cpu.registers, reg, 0x10)
        write_program(cpu, [opcode])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0x11

    @pytest.mark.parametrize("val,exp_result,exp_s,exp_z,exp_h,exp_pv", [
        (0x00, 0x01, False, False, False, False),
        (0x7F, 0x80, True, False, True, True),
        (0xFF, 0x00, False, True, True, False),
        (0x0F, 0x10, False, False, True, False),
        (0xFE, 0xFF, True, False, False, False),
        (0x80, 0x81, True, False, False, False),
    ])
    def test_inc_r_flags(self, cpu, val, exp_result, exp_s, exp_z, exp_h, exp_pv):
        """INC r — verify result and flags."""
        cpu.registers.A = val
        cpu.registers.F = FLAG_C  # Preserve carry
        write_program(cpu, [0x3C])  # INC A
        cpu.step()
        assert cpu.registers.A == exp_result
        assert flag_set(cpu, FLAG_S) == exp_s
        assert flag_set(cpu, FLAG_Z) == exp_z
        assert flag_set(cpu, FLAG_H) == exp_h
        assert flag_set(cpu, FLAG_PV) == exp_pv
        assert flag_set(cpu, FLAG_C)  # Carry preserved
        assert flag_clear(cpu, FLAG_N)

    def test_inc_r_overflow(self, cpu):
        """INC r — overflow when 0x7F -> 0x80."""
        cpu.registers.B = 0x7F
        write_program(cpu, [0x04])
        cpu.step()
        assert cpu.registers.B == 0x80
        assert flag_set(cpu, FLAG_PV)
        assert flag_set(cpu, FLAG_S)

    def test_inc_r_zero(self, cpu):
        """INC r — sets Z flag when result is zero."""
        cpu.registers.B = 0xFF
        write_program(cpu, [0x04])
        cpu.step()
        assert cpu.registers.B == 0x00
        assert flag_set(cpu, FLAG_Z)

    def test_inc_r_half_carry(self, cpu):
        """INC r — half carry from bit 3."""
        cpu.registers.B = 0x0F
        write_program(cpu, [0x04])
        cpu.step()
        assert cpu.registers.B == 0x10
        assert flag_set(cpu, FLAG_H)

    def test_inc_preserves_c(self, cpu):
        """INC r — preserves carry flag."""
        cpu.registers.F = FLAG_C
        cpu.registers.B = 0x10
        write_program(cpu, [0x04])
        cpu.step()
        assert flag_set(cpu, FLAG_C)


class TestIncHLIndirect:
    """INC (HL) - Increment memory at HL."""
    def test_inc_hl_basic(self, cpu):
        """INC (HL) — basic increment."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x10)
        write_program(cpu, [0x34])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x11

    def test_inc_hl_zero(self, cpu):
        """INC (HL) — result is zero."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0xFF)
        write_program(cpu, [0x34])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x00
        assert flag_set(cpu, FLAG_Z)

    def test_inc_hl_overflow(self, cpu):
        """INC (HL) — overflow condition."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x7F)
        write_program(cpu, [0x34])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x80
        assert flag_set(cpu, FLAG_PV)

    def test_inc_hl_half_carry(self, cpu):
        """INC (HL) — half carry from bit 3."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x0F)
        write_program(cpu, [0x34])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x10
        assert flag_set(cpu, FLAG_H)

    def test_inc_hl_sets_f5_f3(self, cpu):
        """INC (HL) — F5 and F3 reflect result bits."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x20)  # 0x20 + 1 = 0x21: bit5=1, bit3=0
        write_program(cpu, [0x34])
        cpu.step()
        assert flag_set(cpu, FLAG_F5)
        assert flag_clear(cpu, FLAG_F3)


class TestDecRegister:
    """DEC r - Decrement register."""
    @pytest.mark.parametrize("reg,opcode", [
        ("A", 0x3D), ("B", 0x05), ("C", 0x0D), ("D", 0x15),
        ("E", 0x1D), ("H", 0x25), ("L", 0x2D),
    ])
    def test_dec_r_basic(self, cpu, reg, opcode):
        """DEC r — basic decrement."""
        setattr(cpu.registers, reg, 0x10)
        write_program(cpu, [opcode])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0x0F
        assert flag_set(cpu, FLAG_N)

    @pytest.mark.parametrize("val,exp_result,exp_s,exp_z,exp_h,exp_pv", [
        (0x01, 0x00, False, True, False, False),
        (0x80, 0x7F, False, False, True, True),
        (0x00, 0xFF, True, False, True, False),
        (0x10, 0x0F, False, False, True, False),
        (0x02, 0x01, False, False, False, False),
        (0x01, 0x00, False, True, False, False),
    ])
    def test_dec_r_flags(self, cpu, val, exp_result, exp_s, exp_z, exp_h, exp_pv):
        """DEC r — verify result and flags."""
        cpu.registers.A = val
        cpu.registers.F = FLAG_C  # Preserve carry
        write_program(cpu, [0x3D])  # DEC A
        cpu.step()
        assert cpu.registers.A == exp_result
        assert flag_set(cpu, FLAG_S) == exp_s
        assert flag_set(cpu, FLAG_Z) == exp_z
        assert flag_set(cpu, FLAG_H) == exp_h
        assert flag_set(cpu, FLAG_PV) == exp_pv
        assert flag_set(cpu, FLAG_C)  # Carry preserved
        assert flag_set(cpu, FLAG_N)

    def test_dec_r_overflow(self, cpu):
        """DEC r — overflow when 0x80 -> 0x7F."""
        cpu.registers.B = 0x80
        write_program(cpu, [0x05])
        cpu.step()
        assert cpu.registers.B == 0x7F
        assert flag_set(cpu, FLAG_PV)
        assert flag_clear(cpu, FLAG_S)

    def test_dec_r_zero(self, cpu):
        """DEC r — sets Z flag when result is zero (1->0)."""
        cpu.registers.B = 0x01
        write_program(cpu, [0x05])
        cpu.step()
        assert cpu.registers.B == 0x00
        assert flag_set(cpu, FLAG_Z)

    def test_dec_r_borrow(self, cpu):
        """DEC r — half borrow from bit 4."""
        cpu.registers.B = 0x10
        write_program(cpu, [0x05])
        cpu.step()
        assert flag_set(cpu, FLAG_H)

    def test_dec_r_underflow(self, cpu):
        """DEC r — 0x00 -> 0xFF."""
        cpu.registers.B = 0x00
        write_program(cpu, [0x05])
        cpu.step()
        assert cpu.registers.B == 0xFF
        assert flag_set(cpu, FLAG_S)

    def test_dec_preserves_c(self, cpu):
        """DEC r — preserves carry flag."""
        cpu.registers.F = FLAG_C
        cpu.registers.B = 0x10
        write_program(cpu, [0x05])
        cpu.step()
        assert flag_set(cpu, FLAG_C)


class TestDecHLIndirect:
    """DEC (HL) - Decrement memory at HL."""
    def test_dec_hl_basic(self, cpu):
        """DEC (HL) — basic decrement."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x10)
        write_program(cpu, [0x35])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x0F

    def test_dec_hl_zero(self, cpu):
        """DEC (HL) — result is zero."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x01)
        write_program(cpu, [0x35])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x00
        assert flag_set(cpu, FLAG_Z)

    def test_dec_hl_overflow(self, cpu):
        """DEC (HL) — overflow condition."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x80)
        write_program(cpu, [0x35])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x7F
        assert flag_set(cpu, FLAG_PV)

    def test_dec_hl_borrow(self, cpu):
        """DEC (HL) — half borrow from bit 4."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x10)
        write_program(cpu, [0x35])
        cpu.step()
        assert flag_set(cpu, FLAG_H)

    def test_dec_hl_sets_f5_f3(self, cpu):
        """DEC (HL) — F5 and F3 reflect result bits."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x20)  # 0x20 - 1 = 0x1F: bit5=0, bit3=1
        write_program(cpu, [0x35])
        cpu.step()
        assert flag_clear(cpu, FLAG_F5)
        assert flag_set(cpu, FLAG_F3)


class TestIncDecIndexed:
    """INC/DEC for indexed registers (IXH, IXL, IYH, IYL)."""
    @pytest.mark.parametrize("prefix,inc_op,dec_op", [
        (0xDD, 0x24, 0x25),  # IXH
        (0xDD, 0x2C, 0x2D),  # IXL (Note: these are actually L and H with DD prefix)
        (0xFD, 0x24, 0x25),  # IYH
        (0xFD, 0x2C, 0x2D),  # IYL
    ])
    def test_inc_dec_ix_iy_h_l(self, cpu, prefix, inc_op, dec_op):
        """INC/DEC IXH, IXL, IYH, IYL."""
        cpu.registers.IX = 0x1234  # For DD prefix
        cpu.registers.IY = 0x5678  # For FD prefix
        # INC
        write_program(cpu, [prefix, inc_op])
        cpu.step()
        # DEC
        write_program(cpu, [prefix, dec_op])
        cpu.step()


class TestIncDecTiming:
    """Verify cycle counts for INC/DEC instructions."""
    def test_inc_r_cycles(self, cpu):
        """INC r takes 4 cycles."""
        write_program(cpu, [0x3C])  # INC A
        assert cpu.step() == 4

    def test_dec_r_cycles(self, cpu):
        """DEC r takes 4 cycles."""
        write_program(cpu, [0x3D])  # DEC A
        assert cpu.step() == 4

    def test_inc_hl_cycles(self, cpu):
        """INC (HL) takes 11 cycles."""
        cpu.registers.HL = 0x2000
        write_program(cpu, [0x34])
        assert cpu.step() == 11

    def test_dec_hl_cycles(self, cpu):
        """DEC (HL) takes 11 cycles."""
        cpu.registers.HL = 0x2000
        write_program(cpu, [0x35])
        assert cpu.step() == 11
