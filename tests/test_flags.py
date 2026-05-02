#!/usr/bin/env python3
"""Comprehensive flag calculation tests for all Z80 instructions."""
import pytest
from conftest import (
    write_program, FLAG_S, FLAG_Z, FLAG_F5, FLAG_H, FLAG_F3, FLAG_PV, FLAG_N, FLAG_C,
    flag_set, flag_clear
)


class TestFlagDefinitions:
    """Verify flag constants are correct."""
    def test_flag_constants(self, cpu):
        assert FLAG_S == 0x80
        assert FLAG_Z == 0x40
        assert FLAG_F5 == 0x20
        assert FLAG_H == 0x10
        assert FLAG_F3 == 0x08
        assert FLAG_PV == 0x04
        assert FLAG_N == 0x02
        assert FLAG_C == 0x01


class TestSignFlag:
    """Test Sign flag (S) - set if result bit 7 is set."""
    def test_s_set(self, cpu):
        """S flag set when result has bit 7 = 1."""
        write_program(cpu, [0x3E, 0x80, 0xC6, 0x00])  # LD A,0x80; ADD A,0
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_S)

    def test_s_clear(self, cpu):
        """S flag clear when result has bit 7 = 0."""
        write_program(cpu, [0x3E, 0x7F, 0xC6, 0x00])  # LD A,0x7F; ADD A,0
        cpu.step()
        cpu.step()
        assert flag_clear(cpu, FLAG_S)

    @pytest.mark.parametrize("val", [0x00, 0x01, 0x7F, 0x80, 0xFF])
    def test_s_all_values(self, cpu, val):
        """Test S flag for various values."""
        # Use ADD A,0 to set flags based on A value
        write_program(cpu, [0x3E, val, 0xC6, 0x00])  # LD A,val; ADD A,0
        cpu.step()  # LD A,val
        cpu.step()  # ADD A,0 - this sets flags based on result
        if val & 0x80:
            assert flag_set(cpu, FLAG_S)
        else:
            assert flag_clear(cpu, FLAG_S)


class TestZeroFlag:
    """Test Zero flag (Z) - set if result is zero."""
    def test_z_set(self, cpu):
        """Z flag set when result is 0."""
        write_program(cpu, [0x3E, 0x01, 0xD6, 0x01])  # LD A,1; SUB A,1
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_Z)

    def test_z_clear(self, cpu):
        """Z flag clear when result is non-zero."""
        write_program(cpu, [0x3E, 0x01, 0xD6, 0x00])  # LD A,1; SUB A,0
        cpu.step()
        cpu.step()
        assert flag_clear(cpu, FLAG_Z)


class TestHalfCarryFlag:
    """Test Half-carry flag (H) - set on carry/borrow from bit 3 to bit 4."""
    def test_h_set_add(self, cpu):
        """H flag set on carry from bit 3 in ADD."""
        write_program(cpu, [0x3E, 0x08, 0xC6, 0x08])  # LD A,8; ADD A,8
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_H)

    def test_h_clear_add(self, cpu):
        """H flag clear when no carry from bit 3 in ADD."""
        write_program(cpu, [0x3E, 0x07, 0xC6, 0x01])  # LD A,7; ADD A,1
        cpu.step()
        cpu.step()
        assert flag_clear(cpu, FLAG_H)

    def test_h_set_sub(self, cpu):
        """H flag set on borrow from bit 4 in SUB."""
        write_program(cpu, [0x3E, 0x10, 0xD6, 0x01])  # LD A,0x10; SUB A,1
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_H)

    def test_h_set_inc(self, cpu):
        """H flag set on carry from bit 3 in INC."""
        cpu.registers.B = 0x0F
        write_program(cpu, [0x04])  # INC B
        cpu.step()
        assert flag_set(cpu, FLAG_H)


class TestParityOverflowFlag:
    """Test Parity/Overflow flag (P/V)."""
    def test_pv_parity_even(self, cpu):
        """PV = parity (even number of 1 bits) for logic ops."""
        write_program(cpu, [0x3E, 0x03, 0xE6, 0x03])  # LD A,3; AND 3
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_PV)  # 3 = 0b11 has 2 (even) 1-bits

    def test_pv_parity_odd(self, cpu):
        """PV = parity (odd number of 1 bits) for logic ops."""
        write_program(cpu, [0x3E, 0x01, 0xE6, 0x01])  # LD A,1; AND 1
        cpu.step()
        cpu.step()
        assert flag_clear(cpu, FLAG_PV)  # 1 = 0b01 has 1 (odd) 1-bit

    def test_pv_overflow_add(self, cpu):
        """PV = overflow for signed arithmetic."""
        write_program(cpu, [0x3E, 0x7F, 0xC6, 0x01])  # LD A,0x7F; ADD A,1
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_PV)  # +127 + 1 = +128 (overflow)

    def test_pv_overflow_sub(self, cpu):
        """PV = overflow for signed subtraction."""
        write_program(cpu, [0x3E, 0x80, 0xD6, 0x01])  # LD A,0x80; SUB A,1
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_PV)  # -128 - 1 = -129 (overflow)


class TestAddSubtractFlag:
    """Test Add/Subtract flag (N) - set for subtract operations."""
    def test_n_set_sub(self, cpu):
        """N flag set after SUB."""
        write_program(cpu, [0x3E, 0x10, 0xD6, 0x01])  # LD A,0x10; SUB 1
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_N)

    def test_n_set_dec(self, cpu):
        """N flag set after DEC."""
        write_program(cpu, [0x05])  # DEC B
        cpu.step()
        assert flag_set(cpu, FLAG_N)

    def test_n_clear_add(self, cpu):
        """N flag clear after ADD."""
        write_program(cpu, [0x3E, 0x10, 0xC6, 0x01])  # LD A,0x10; ADD 1
        cpu.step()
        cpu.step()
        assert flag_clear(cpu, FLAG_N)

    def test_n_clear_and(self, cpu):
        """N flag clear after AND."""
        write_program(cpu, [0x3E, 0xFF, 0xE6, 0x0F])  # LD A,0xFF; AND 0x0F
        cpu.step()
        cpu.step()
        assert flag_clear(cpu, FLAG_N)


class TestCarryFlag:
    """Test Carry flag (C)."""
    def test_c_set_add(self, cpu):
        """C flag set on carry from bit 7 in ADD."""
        write_program(cpu, [0x3E, 0x80, 0xC6, 0x80])  # LD A,0x80; ADD A,0x80
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_C)

    def test_c_clear_add(self, cpu):
        """C flag clear when no carry from bit 7 in ADD."""
        write_program(cpu, [0x3E, 0x40, 0xC6, 0x3F])  # LD A,0x40; ADD A,0x3F
        cpu.step()
        cpu.step()
        assert flag_clear(cpu, FLAG_C)

    def test_c_set_sub(self, cpu):
        """C flag set on borrow (A < operand) in SUB."""
        write_program(cpu, [0x3E, 0x00, 0xD6, 0x01])  # LD A,0; SUB 1
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_C)

    def test_c_clear_sub(self, cpu):
        """C flag clear when no borrow in SUB."""
        write_program(cpu, [0x3E, 0x02, 0xD6, 0x01])  # LD A,2; SUB 1
        cpu.step()
        cpu.step()
        assert flag_clear(cpu, FLAG_C)


class TestUndocumentedFlags:
    """Test undocumented F5 and F3 flags - copy of bits 5 and 3 of result."""
    def test_f5_set(self, cpu):
        """F5 flag reflects bit 5 of result."""
        write_program(cpu, [0x3E, 0x20, 0xC6, 0x00])  # LD A,0x20; ADD A,0
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_F5)

    def test_f5_clear(self, cpu):
        """F5 flag clear when bit 5 of result is 0."""
        write_program(cpu, [0x3E, 0x10, 0xC6, 0x00])  # LD A,0x10; ADD A,0
        cpu.step()
        cpu.step()
        assert flag_clear(cpu, FLAG_F5)

    def test_f3_set(self, cpu):
        """F3 flag reflects bit 3 of result."""
        write_program(cpu, [0x3E, 0x08, 0xC6, 0x00])  # LD A,0x08; ADD A,0
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_F3)

    def test_f3_clear(self, cpu):
        """F3 flag clear when bit 3 of result is 0."""
        write_program(cpu, [0x3E, 0x10, 0xC6, 0x00])  # LD A,0x10; ADD A,0
        cpu.step()
        cpu.step()
        assert flag_clear(cpu, FLAG_F3)


class TestFlagPreservation:
    """Test which flags are preserved/affected by various instructions."""
    def test_inc_preserves_c(self, cpu):
        """INC preserves carry flag."""
        cpu.registers.F = FLAG_C
        cpu.registers.B = 0x10
        write_program(cpu, [0x04])  # INC B
        cpu.step()
        assert flag_set(cpu, FLAG_C)

    def test_dec_preserves_c(self, cpu):
        """DEC preserves carry flag."""
        cpu.registers.F = FLAG_C
        cpu.registers.B = 0x10
        write_program(cpu, [0x05])  # DEC B
        cpu.step()
        assert flag_set(cpu, FLAG_C)

    def test_ld_preserves_all_flags(self, cpu):
        """LD does not affect any flags."""
        cpu.registers.F = 0xFF
        cpu.registers.B = 0x00
        write_program(cpu, [0x78])  # LD A,B
        cpu.step()
        assert cpu.registers.F == 0xFF

    def test_pop_af_restores_flags(self, cpu):
        """POP AF restores all flags."""
        cpu.registers.SP = 0xFFFE
        # Stack contains: low byte at FFFE, high byte at FFFF
        cpu.write_byte(0xFFFE, 0xAA)  # This goes to F (low byte)
        cpu.write_byte(0xFFFF, 0x12)  # This goes to A (high byte)
        write_program(cpu, [0xF1])  # POP AF
        cpu.step()
        assert cpu.registers.F == 0xAA  # Low byte goes to F
        assert cpu.registers.A == 0x12  # High byte goes to A
