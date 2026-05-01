#!/usr/bin/env python3
"""Comprehensive ADD/ADC/SUB/SBC instruction tests."""
import pytest
from conftest import (
    write_program, cpu, step_n,
    FLAG_S, FLAG_Z, FLAG_F5, FLAG_H, FLAG_F3, FLAG_PV, FLAG_N, FLAG_C,
    assert_flags, _add_flags, _sub_flags, flag_set, flag_clear
)


class TestAddImmediate:
    """ADD A,n - Add immediate to accumulator."""
    @pytest.mark.parametrize("a,b,expected", [
        (0x00, 0x00, 0x00), (0x01, 0x02, 0x03), (0x7F, 0x01, 0x80),
        (0xFF, 0x01, 0x00), (0x80, 0x80, 0x00), (0x0F, 0x01, 0x10),
        (0xF0, 0x10, 0x00), (0x55, 0xAA, 0xFF), (0x01, 0xFF, 0x00),
        (0x40, 0x40, 0x80), (0xFE, 0x01, 0xFF), (0x0A, 0x0B, 0x15),
    ])
    def test_add_a_n(self, cpu, a, b, expected):
        """ADD A,n — verify result and flags."""
        write_program(cpu, [0x3E, a, 0xC6, b])
        cpu.step()
        cpu.step()
        assert cpu.registers.A == expected
        expected_flags = _add_flags(a, b)
        assert_flags(cpu, expected_flags)

    def test_add_a_n_carry(self, cpu):
        """ADD A,n — carry set when result > 0xFF."""
        write_program(cpu, [0x3E, 0x80, 0xC6, 0x80])
        cpu.step()
        cpu.step()
        assert cpu.registers.A == 0x00
        assert flag_set(cpu, FLAG_C)

    def test_add_a_n_half_carry(self, cpu):
        """ADD A,n — half carry from bit 3."""
        write_program(cpu, [0x3E, 0x08, 0xC6, 0x08])
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_H)

    def test_add_a_n_overflow_positive(self, cpu):
        """ADD A,n — overflow when adding two positives gives negative."""
        write_program(cpu, [0x3E, 0x7F, 0xC6, 0x01])
        cpu.step()
        cpu.step()
        assert cpu.registers.A == 0x80
        assert flag_set(cpu, FLAG_PV)
        assert flag_set(cpu, FLAG_S)

    def test_add_a_n_overflow_negative(self, cpu):
        """ADD A,n — overflow when adding two negatives gives positive."""
        write_program(cpu, [0x3E, 0x80, 0xC6, 0x80])
        cpu.step()
        cpu.step()
        assert cpu.registers.A == 0x00
        assert flag_set(cpu, FLAG_PV)


class TestAddRegister:
    """ADD A,r - Add register to accumulator."""
    @pytest.mark.parametrize("reg,opcode,expected", [
        ("B", 0x80, 0x30), ("C", 0x81, 0x30), ("D", 0x82, 0x30), ("E", 0x83, 0x30),
        ("H", 0x84, 0x30), ("L", 0x85, 0x30), 
        ("A", 0x87, 0x20),  # ADD A,A = 0x10 + 0x10 = 0x20
    ])
    def test_add_a_r(self, cpu, reg, opcode, expected):
        """ADD A,r — add register to A."""
        cpu.registers.A = 0x10
        if reg == "HL":
            cpu.registers.HL = 0x2000
            cpu.write_byte(0x2000, 0x20)
        elif reg != "A":
            setattr(cpu.registers, reg, 0x20)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.registers.A == expected

    def test_add_a_hl(self, cpu):
        """ADD A,(HL) — add memory to A."""
        cpu.registers.A = 0x10
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x20)
        write_program(cpu, [0x86])
        cpu.step()
        assert cpu.registers.A == 0x30


class TestAdcImmediate:
    """ADC A,n - Add with carry immediate."""
    def test_adc_a_n_with_carry(self, cpu):
        """ADC A,n — carry input is included."""
        cpu.registers.A = 0x0F
        cpu.registers.F = FLAG_C
        write_program(cpu, [0xCE, 0x01])
        cpu.step()
        assert cpu.registers.A == 0x11
        assert flag_set(cpu, FLAG_H)

    def test_adc_a_n_no_carry(self, cpu):
        """ADC A,n — without carry behaves like ADD."""
        cpu.registers.A = 0x10
        cpu.registers.F = 0
        write_program(cpu, [0xCE, 0x05])
        cpu.step()
        assert cpu.registers.A == 0x15

    def test_adc_a_n_carry_overflow(self, cpu):
        """ADC A,n — carry can trigger overflow."""
        cpu.registers.A = 0x7F
        cpu.registers.F = FLAG_C
        write_program(cpu, [0xCE, 0x00])
        cpu.step()
        assert cpu.registers.A == 0x80
        assert flag_set(cpu, FLAG_PV)
        assert flag_set(cpu, FLAG_S)

    @pytest.mark.parametrize("a,b,carry,expected", [
        (0x00, 0x00, 0, 0x00), (0xFF, 0x00, 1, 0x00),
        (0x80, 0x7F, 1, 0x00), (0x7F, 0x00, 1, 0x80),
    ])
    def test_adc_a_n_variations(self, cpu, a, b, carry, expected):
        """ADC A,n — various test cases."""
        cpu.registers.A = a
        cpu.registers.F = FLAG_C if carry else 0
        write_program(cpu, [0xCE, b])
        cpu.step()
        assert cpu.registers.A == expected


class TestAdcRegister:
    """ADC A,r - Add with carry register."""
    @pytest.mark.parametrize("reg,opcode,expected", [
        ("B", 0x88, 0x31), ("C", 0x89, 0x31), ("D", 0x8A, 0x31), ("E", 0x8B, 0x31),
        ("H", 0x8C, 0x31), ("L", 0x8D, 0x31),
        ("A", 0x8F, 0x21),  # ADC A,A = 0x10 + 0x10 + 1 = 0x21 (A NOT overwritten)
    ])
    def test_adc_a_r_with_carry(self, cpu, reg, opcode, expected):
        """ADC A,r — with carry set."""
        cpu.registers.A = 0x10
        if reg != "A":
            setattr(cpu.registers, reg, 0x20)
        cpu.registers.F = FLAG_C
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.registers.A == expected

    def test_adc_a_hl_with_carry(self, cpu):
        """ADC A,(HL) — with carry set."""
        cpu.registers.A = 0x10
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x20)
        cpu.registers.F = FLAG_C
        write_program(cpu, [0x8E])
        cpu.step()
        assert cpu.registers.A == 0x31


class TestSubImmediate:
    """SUB n - Subtract immediate from accumulator."""
    @pytest.mark.parametrize("a,b,expected", [
        (0x10, 0x05, 0x0B), (0x00, 0x01, 0xFF), (0x80, 0x01, 0x7F),
        (0x10, 0x10, 0x00), (0x0F, 0x10, 0xFF), (0x80, 0x80, 0x00),
        (0x90, 0x80, 0x10),
    ])
    def test_sub_a_n(self, cpu, a, b, expected):
        """SUB n — verify result and flags."""
        write_program(cpu, [0x3E, a, 0xD6, b])
        cpu.step()
        cpu.step()
        assert cpu.registers.A == expected
        expected_flags = _sub_flags(a, b)
        assert_flags(cpu, expected_flags)
        assert flag_set(cpu, FLAG_N)

    def test_sub_a_n_zero(self, cpu):
        """SUB n — result is zero."""
        write_program(cpu, [0x3E, 0x10, 0xD6, 0x10])
        cpu.step()
        cpu.step()
        assert cpu.registers.A == 0x00
        assert flag_set(cpu, FLAG_Z)

    def test_sub_a_n_borrow(self, cpu):
        """SUB n — borrow (carry) set."""
        write_program(cpu, [0x3E, 0x00, 0xD6, 0x01])
        cpu.step()
        cpu.step()
        assert cpu.registers.A == 0xFF
        assert flag_set(cpu, FLAG_C)

    def test_sub_a_n_overflow(self, cpu):
        """SUB n — overflow when subtracting negative from positive."""
        write_program(cpu, [0x3E, 0x80, 0xD6, 0x01])
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_PV)


class TestSubRegister:
    """SUB r - Subtract register from accumulator."""
    @pytest.mark.parametrize("reg,opcode,expected", [
        ("B", 0x90, 0x20), ("C", 0x91, 0x20), ("D", 0x92, 0x20), ("E", 0x93, 0x20),
        ("H", 0x94, 0x20), ("L", 0x95, 0x20),
        ("A", 0x97, 0x00),  # SUB A,A = 0x10 - 0x10 = 0x00 (A overwritten to 0x10)
    ])
    def test_sub_a_r(self, cpu, reg, opcode, expected):
        """SUB A,r — subtract register from A."""
        cpu.registers.A = 0x30
        if reg == "HL":
            cpu.registers.HL = 0x2000
            cpu.write_byte(0x2000, 0x10)
        elif reg != "A":
            setattr(cpu.registers, reg, 0x10)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.registers.A == expected
        assert flag_set(cpu, FLAG_N)


class TestSbcImmediate:
    """SBC A,n - Subtract with carry immediate."""
    def test_sbc_a_n_with_carry(self, cpu):
        """SBC A,n — carry included in subtraction."""
        cpu.registers.A = 0x10
        cpu.registers.F = FLAG_C
        write_program(cpu, [0xDE, 0x05])
        cpu.step()
        assert cpu.registers.A == 0x0A  # 0x10 - 0x05 - 1

    def test_sbc_a_n_no_carry(self, cpu):
        """SBC A,n — without carry behaves like SUB."""
        cpu.registers.A = 0x10
        cpu.registers.F = 0
        write_program(cpu, [0xDE, 0x05])
        cpu.step()
        assert cpu.registers.A == 0x0B  # 0x10 - 0x05

    def test_sbc_a_n_overflow(self, cpu):
        """SBC A,n — overflow condition."""
        cpu.registers.A = 0x80
        cpu.registers.F = FLAG_C
        write_program(cpu, [0xDE, 0x00])
        cpu.step()
        assert cpu.registers.A == 0x7F
        assert flag_set(cpu, FLAG_PV)


class TestSbcRegister:
    """SBC A,r - Subtract with carry register."""
    @pytest.mark.parametrize("reg,opcode,expected", [
        ("B", 0x98, 0x0F), ("C", 0x99, 0x0F), ("D", 0x9A, 0x0F), ("E", 0x9B, 0x0F),
        ("H", 0x9C, 0x0F), ("L", 0x9D, 0x0F),
        ("A", 0x9F, 0xFF),  # SBC A,A = 0x10 - 0x10 - 1 = -1 = 0xFF (A overwritten to 0x10)
    ])
    def test_sbc_a_r_with_carry(self, cpu, reg, opcode, expected):
        """SBC A,r — with carry set."""
        cpu.registers.A = 0x20
        if reg != "A":
            setattr(cpu.registers, reg, 0x10)
        cpu.registers.F = FLAG_C
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.registers.A == expected  # 0x20 - 0x10 - 1 = 0x0F

    def test_sbc_a_hl_with_carry(self, cpu):
        """SBC A,(HL) — with carry set."""
        cpu.registers.A = 0x20
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x10)
        cpu.registers.F = FLAG_C
        write_program(cpu, [0x9E])
        cpu.step()
        assert cpu.registers.A == 0x0F


class TestAddFlags:
    """Verify all flags for ADD instructions."""
    def test_add_preserves_f5_f3(self, cpu):
        """ADD preserves F5 and F3 from result."""
        write_program(cpu, [0x3E, 0x20, 0xC6, 0x00])  # 0x20 has bit 5=1, bit 3=0
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_F5)
        assert flag_clear(cpu, FLAG_F3)

    def test_add_clears_n(self, cpu):
        """ADD clears N flag."""
        cpu.registers.F = FLAG_N
        write_program(cpu, [0x3E, 0x10, 0xC6, 0x10])
        cpu.step()
        cpu.step()
        assert flag_clear(cpu, FLAG_N)


class TestSubFlags:
    """Verify all flags for SUB/SBC instructions."""
    def test_sub_sets_n(self, cpu):
        """SUB sets N flag."""
        write_program(cpu, [0x3E, 0x10, 0xD6, 0x05])
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_N)

    def test_sub_preserves_f5_f3(self, cpu):
        """SUB preserves F5 and F3 from result."""
        write_program(cpu, [0x3E, 0x30, 0xD6, 0x10])  # Result 0x20: bit5=1, bit3=0
        cpu.step()
        cpu.step()
        assert flag_set(cpu, FLAG_F5)
        assert flag_clear(cpu, FLAG_F3)


class TestAddTiming:
    """Verify cycle counts for ADD/ADC/SUB/SBC."""
    def test_add_a_n_cycles(self, cpu):
        """ADD A,n takes 7 cycles."""
        write_program(cpu, [0x3E, 0x10, 0xC6, 0x10])
        cpu.step()
        assert cpu.step() == 7

    def test_add_a_r_cycles(self, cpu):
        """ADD A,r takes 4 cycles."""
        write_program(cpu, [0x80])
        assert cpu.step() == 4

    def test_add_a_hl_cycles(self, cpu):
        """ADD A,(HL) takes 7 cycles."""
        cpu.registers.HL = 0x2000
        write_program(cpu, [0x86])
        assert cpu.step() == 7

    def test_adc_a_n_cycles(self, cpu):
        """ADC A,n takes 7 cycles."""
        write_program(cpu, [0xCE, 0x10])
        assert cpu.step() == 7

    def test_sub_a_n_cycles(self, cpu):
        """SUB A,n takes 7 cycles."""
        write_program(cpu, [0xD6, 0x10])
        assert cpu.step() == 7

    def test_sbc_a_n_cycles(self, cpu):
        """SBC A,n takes 7 cycles."""
        write_program(cpu, [0xDE, 0x10])
        assert cpu.step() == 7
