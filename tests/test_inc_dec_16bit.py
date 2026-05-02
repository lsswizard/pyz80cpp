#!/usr/bin/env python3
"""Comprehensive 16-bit increment and decrement tests."""
import pytest
from conftest import (
    write_program, FLAG_S, FLAG_Z, FLAG_C
)


class TestInc16Bit:
    """INC ss - 16-bit increment."""
    @pytest.mark.parametrize("pair,inc_op", [
        ("BC", 0x03), ("DE", 0x13), ("HL", 0x23), ("SP", 0x33),
    ])
    def test_inc_rr_basic(self, cpu, pair, inc_op):
        """INC ss — basic increment."""
        setattr(cpu.registers, pair, 0x1000)
        write_program(cpu, [inc_op])
        cpu.step()
        assert getattr(cpu.registers, pair) == 0x1001

    @pytest.mark.parametrize("pair,inc_op", [
        ("BC", 0x03), ("DE", 0x13), ("HL", 0x23), ("SP", 0x33),
    ])
    def test_inc_rr_wrap(self, cpu, pair, inc_op):
        """INC ss — wraps from 0xFFFF to 0x0000."""
        setattr(cpu.registers, pair, 0xFFFF)
        write_program(cpu, [inc_op])
        cpu.step()
        assert getattr(cpu.registers, pair) == 0x0000

    @pytest.mark.parametrize("pair,inc_op", [
        ("BC", 0x03), ("DE", 0x13), ("HL", 0x23), ("SP", 0x33),
    ])
    def test_inc_rr_flags(self, cpu, pair, inc_op):
        """INC ss — does not modify any flags."""
        cpu.registers.F = FLAG_Z | FLAG_C | FLAG_S
        setattr(cpu.registers, pair, 0x1000)
        write_program(cpu, [inc_op])
        cpu.step()
        assert cpu.registers.F == (FLAG_Z | FLAG_C | FLAG_S)


class TestDec16Bit:
    """DEC ss - 16-bit decrement."""
    @pytest.mark.parametrize("pair,dec_op", [
        ("BC", 0x0B), ("DE", 0x1B), ("HL", 0x2B), ("SP", 0x3B),
    ])
    def test_dec_rr_basic(self, cpu, pair, dec_op):
        """DEC ss — basic decrement."""
        setattr(cpu.registers, pair, 0x1000)
        write_program(cpu, [dec_op])
        cpu.step()
        assert getattr(cpu.registers, pair) == 0x0FFF

    @pytest.mark.parametrize("pair,dec_op", [
        ("BC", 0x0B), ("DE", 0x1B), ("HL", 0x2B), ("SP", 0x3B),
    ])
    def test_dec_rr_wrap(self, cpu, pair, dec_op):
        """DEC ss — wraps from 0x0000 to 0xFFFF."""
        setattr(cpu.registers, pair, 0x0000)
        write_program(cpu, [dec_op])
        cpu.step()
        assert getattr(cpu.registers, pair) == 0xFFFF

    @pytest.mark.parametrize("pair,dec_op", [
        ("BC", 0x0B), ("DE", 0x1B), ("HL", 0x2B), ("SP", 0x3B),
    ])
    def test_dec_rr_flags(self, cpu, pair, dec_op):
        """DEC ss — does not modify any flags."""
        cpu.registers.F = FLAG_Z | FLAG_C | FLAG_S
        setattr(cpu.registers, pair, 0x1000)
        write_program(cpu, [dec_op])
        cpu.step()
        assert cpu.registers.F == (FLAG_Z | FLAG_C | FLAG_S)


class TestIncDecIXIY:
    """INC/DEC IX/IY - Index register increment/decrement."""
    @pytest.mark.parametrize("prefix,inc_op,dec_op,ix_reg", [
        (0xDD, 0x23, 0x2B, "IX"),
        (0xFD, 0x23, 0x2B, "IY"),
    ])
    def test_inc_dec_ix_iy(self, cpu, prefix, inc_op, dec_op, ix_reg):
        """INC/DEC IX/IY."""
        setattr(cpu.registers, ix_reg, 0x1000)
        # INC
        write_program(cpu, [prefix, inc_op])
        cpu.step()
        assert getattr(cpu.registers, ix_reg) == 0x1001
        # DEC
        setattr(cpu.registers, ix_reg, 0x1000)
        write_program(cpu, [prefix, dec_op])
        cpu.step()
        assert getattr(cpu.registers, ix_reg) == 0x0FFF

    @pytest.mark.parametrize("prefix,inc_op,ix_reg", [
        (0xDD, 0x23, "IX"),
        (0xFD, 0x23, "IY"),
    ])
    def test_inc_ix_iy_wrap(self, cpu, prefix, inc_op, ix_reg):
        """INC IX/IY — wraps from 0xFFFF to 0x0000."""
        setattr(cpu.registers, ix_reg, 0xFFFF)
        write_program(cpu, [prefix, inc_op])
        cpu.step()
        assert getattr(cpu.registers, ix_reg) == 0x0000

    @pytest.mark.parametrize("prefix,dec_op,ix_reg", [
        (0xDD, 0x2B, "IX"),
        (0xFD, 0x2B, "IY"),
    ])
    def test_dec_ix_iy_wrap(self, cpu, prefix, dec_op, ix_reg):
        """DEC IX/IY — wraps from 0x0000 to 0xFFFF."""
        setattr(cpu.registers, ix_reg, 0x0000)
        write_program(cpu, [prefix, dec_op])
        cpu.step()
        assert getattr(cpu.registers, ix_reg) == 0xFFFF


class TestIncDecIndexedRegs:
    """INC/DEC IXH, IXL, IYH, IYL - Index register half operations."""
    @pytest.mark.parametrize("prefix,reg,inc_op,dec_op,getter", [
        (0xDD, "IXH", 0x24, 0x25, lambda c: (c.regs.IX >> 8) & 0xFF),
        (0xDD, "IXL", 0x2C, 0x2D, lambda c: c.regs.IX & 0xFF),
        (0xFD, "IYH", 0x24, 0x25, lambda c: (c.regs.IY >> 8) & 0xFF),
        (0xFD, "IYL", 0x2C, 0x2D, lambda c: c.regs.IY & 0xFF),
    ])
    def test_inc_dec_ix_iy_h_l(self, cpu, prefix, reg, inc_op, dec_op, getter):
        """INC/DEC IXH, IXL, IYH, IYL."""
        cpu.registers.IX = 0x1234
        cpu.registers.IY = 0x5678
        # INC
        write_program(cpu, [prefix, inc_op])
        cpu.step()
        # DEC
        write_program(cpu, [prefix, dec_op])
        cpu.step()


class TestIncDecTiming:
    """Verify cycle counts for 16-bit INC/DEC."""
    @pytest.mark.parametrize("opcode", [0x03, 0x13, 0x23, 0x33])
    def test_inc_rr_cycles(self, cpu, opcode):
        """INC ss takes 6 cycles."""
        write_program(cpu, [opcode])
        assert cpu.step() == 6

    @pytest.mark.parametrize("opcode", [0x0B, 0x1B, 0x2B, 0x3B])
    def test_dec_rr_cycles(self, cpu, opcode):
        """DEC ss takes 6 cycles."""
        write_program(cpu, [opcode])
        assert cpu.step() == 6

    def test_inc_ix_cycles(self, cpu):
        """INC IX takes 10 cycles."""
        write_program(cpu, [0xDD, 0x23])
        assert cpu.step() == 10

    def test_dec_ix_cycles(self, cpu):
        """DEC IX takes 10 cycles."""
        write_program(cpu, [0xDD, 0x2B])
        assert cpu.step() == 10

    def test_inc_iy_cycles(self, cpu):
        """INC IY takes 10 cycles."""
        write_program(cpu, [0xFD, 0x23])
        assert cpu.step() == 10

    def test_dec_iy_cycles(self, cpu):
        """DEC IY takes 10 cycles."""
        write_program(cpu, [0xFD, 0x2B])
        assert cpu.step() == 10
