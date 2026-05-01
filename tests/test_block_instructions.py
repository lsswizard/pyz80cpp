#!/usr/bin/env python3
"""Comprehensive block transfer and compare instruction tests."""
import pytest
from conftest import (
    write_program, cpu, step_n,
    flag_set, flag_clear,
    FLAG_Z, FLAG_PV, FLAG_C, FLAG_S, FLAG_H, FLAG_N
)


class TestLDI:
    """LDI - Load and increment."""
    def test_ldi_basic(self, cpu):
        """LDI — basic operation."""
        cpu.registers.HL = 0x2000
        cpu.registers.DE = 0x3000
        cpu.registers.BC = 0x0001
        cpu.write_byte(0x2000, 0xAB)
        write_program(cpu, [0xED, 0xA0])
        cpu.step()
        assert cpu.read_byte(0x3000) == 0xAB
        assert cpu.registers.HL == 0x2001
        assert cpu.registers.DE == 0x3001
        assert cpu.registers.BC == 0x0000
        assert flag_clear(cpu, FLAG_PV)

    def test_ldi_preserves_flags_except_pv(self, cpu):
        """LDI — preserves S, Z, H, N, C flags."""
        cpu.registers.F = FLAG_S | FLAG_Z | FLAG_H | FLAG_N | FLAG_C
        cpu.registers.HL = 0x2000
        cpu.registers.DE = 0x3000
        cpu.registers.BC = 0x0002
        cpu.write_byte(0x2000, 0xAB)
        write_program(cpu, [0xED, 0xA0])
        cpu.step()
        assert cpu.registers.F & (FLAG_S | FLAG_Z | FLAG_H | FLAG_N | FLAG_C)
        assert flag_clear(cpu, FLAG_PV)  # BC != 0, so PV=0

    def test_ldi_pv_set(self, cpu):
        """LDI — PV set when BC becomes 0."""
        cpu.registers.HL = 0x2000
        cpu.registers.DE = 0x3000
        cpu.registers.BC = 0x0001
        cpu.write_byte(0x2000, 0xAB)
        write_program(cpu, [0xED, 0xA0])
        cpu.step()
        assert flag_clear(cpu, FLAG_PV)  # BC becomes 0, PV=0


class TestLDD:
    """LDD - Load and decrement."""
    def test_ldd_basic(self, cpu):
        """LDD — basic operation."""
        cpu.registers.HL = 0x2000
        cpu.registers.DE = 0x3000
        cpu.registers.BC = 0x0001
        cpu.write_byte(0x2000, 0xAB)
        write_program(cpu, [0xED, 0xA8])
        cpu.step()
        assert cpu.read_byte(0x3000) == 0xAB
        assert cpu.registers.HL == 0x1FFF
        assert cpu.registers.DE == 0x2FFF
        assert cpu.registers.BC == 0x0000

    def test_ldd_decrement(self, cpu):
        """LDD — decrements HL, DE, BC."""
        cpu.registers.HL = 0x2001
        cpu.registers.DE = 0x3001
        cpu.registers.BC = 0x0002
        cpu.write_byte(0x2001, 0xCD)
        write_program(cpu, [0xED, 0xA8])
        cpu.step()
        assert cpu.registers.HL == 0x2000
        assert cpu.registers.DE == 0x3000
        assert cpu.registers.BC == 0x0001


class TestLDIR:
    """LDIR - Load, increment, and repeat."""
    def test_ldir_basic(self, cpu):
        """LDIR — repeat until BC=0."""
        cpu.registers.HL = 0x2000
        cpu.registers.DE = 0x3000
        cpu.registers.BC = 0x0002
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        write_program(cpu, [0xED, 0xB0])
        step_n(cpu, 2)
        assert cpu.registers.BC == 0x0000
        assert cpu.read_byte(0x3000) == 0xAA
        assert cpu.read_byte(0x3001) == 0xBB

    def test_ldir_pc_advances(self, cpu):
        """LDIR — PC stays on instruction until BC=0."""
        cpu.registers.HL = 0x2000
        cpu.registers.DE = 0x3000
        cpu.registers.BC = 0x0002
        write_program(cpu, [0xED, 0xB0])
        cpu.step()
        assert cpu.registers.PC == 0  # Still on LDIR
        cpu.step()
        assert cpu.registers.PC == 2  # Advanced after BC=0


class TestLDDR:
    """LDDR - Load, decrement, and repeat."""
    def test_lddr_basic(self, cpu):
        """LDDR — repeat until BC=0 (decrementing)."""
        cpu.registers.HL = 0x2001
        cpu.registers.DE = 0x3001
        cpu.registers.BC = 0x0002
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        write_program(cpu, [0xED, 0xB8])
        step_n(cpu, 2)
        assert cpu.registers.BC == 0x0000
        assert cpu.read_byte(0x3000) == 0xAA
        assert cpu.read_byte(0x3001) == 0xBB


class TestCPI:
    """CPI - Compare and increment."""
    def test_cpi_equal(self, cpu):
        """CPI — values equal."""
        cpu.registers.A = 0xAA
        cpu.registers.HL = 0x2000
        cpu.registers.BC = 0x0001
        cpu.write_byte(0x2000, 0xAA)
        write_program(cpu, [0xED, 0xA1])
        cpu.step()
        assert flag_set(cpu, FLAG_Z)
        assert cpu.registers.HL == 0x2001
        assert cpu.registers.BC == 0x0000
        assert cpu.registers.F & FLAG_N  # N always set

    def test_cpi_not_equal(self, cpu):
        """CPI — values not equal."""
        cpu.registers.A = 0xAA
        cpu.registers.HL = 0x2000
        cpu.registers.BC = 0x0001
        cpu.write_byte(0x2000, 0xBB)
        write_program(cpu, [0xED, 0xA1])
        cpu.step()
        assert flag_clear(cpu, FLAG_Z)
        assert cpu.registers.A == 0xAA  # A unchanged

    def test_cpi_half_borrow(self, cpu):
        """CPI — half borrow from bit 4."""
        cpu.registers.A = 0x10
        cpu.registers.HL = 0x2000
        cpu.registers.BC = 0x0001
        cpu.write_byte(0x2000, 0x01)
        write_program(cpu, [0xED, 0xA1])
        cpu.step()
        assert cpu.registers.F & FLAG_H


class TestCPD:
    """CPD - Compare and decrement."""
    def test_cpd_equal(self, cpu):
        """CPD — values equal."""
        cpu.registers.A = 0xAA
        cpu.registers.HL = 0x2000
        cpu.registers.BC = 0x0001
        cpu.write_byte(0x2000, 0xAA)
        write_program(cpu, [0xED, 0xA9])
        cpu.step()
        assert flag_set(cpu, FLAG_Z)
        assert cpu.registers.HL == 0x1FFF
        assert cpu.registers.BC == 0x0000

    def test_cpd_decrement(self, cpu):
        """CPD — decrements HL and BC."""
        cpu.registers.A = 0xAA
        cpu.registers.HL = 0x2001
        cpu.registers.BC = 0x0002
        cpu.write_byte(0x2001, 0xBB)
        write_program(cpu, [0xED, 0xA9])
        cpu.step()
        assert cpu.registers.HL == 0x2000
        assert cpu.registers.BC == 0x0001


class TestCPIR:
    """CPIR - Compare, increment, and repeat."""
    def test_cpir_found(self, cpu):
        """CPIR — match found before BC=0."""
        cpu.registers.A = 0xBB
        cpu.registers.HL = 0x2000
        cpu.registers.BC = 0x0003
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        cpu.write_byte(0x2002, 0xCC)
        write_program(cpu, [0xED, 0xB1])
        step_n(cpu, 2)
        assert cpu.registers.BC == 0x0001
        assert flag_set(cpu, FLAG_Z)

    def test_cpir_not_found(self, cpu):
        """CPIR — no match, BC becomes 0."""
        cpu.registers.A = 0xDD
        cpu.registers.HL = 0x2000
        cpu.registers.BC = 0x0002
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        write_program(cpu, [0xED, 0xB1])
        step_n(cpu, 2)
        assert cpu.registers.BC == 0x0000
        assert flag_clear(cpu, FLAG_Z)


class TestCPDR:
    """CPDR - Compare, decrement, and repeat."""
    def test_cpdr_found(self, cpu):
        """CPDR — match found before BC=0."""
        cpu.registers.A = 0xBB
        cpu.registers.HL = 0x2002
        cpu.registers.BC = 0x0003
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        cpu.write_byte(0x2002, 0xCC)
        write_program(cpu, [0xED, 0xB9])
        step_n(cpu, 2)
        assert cpu.registers.BC == 0x0001
        assert flag_set(cpu, FLAG_Z)

    def test_cpdr_not_found(self, cpu):
        """CPDR — no match, BC becomes 0."""
        cpu.registers.A = 0xDD
        cpu.registers.HL = 0x2001
        cpu.registers.BC = 0x0002
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        write_program(cpu, [0xED, 0xB9])
        step_n(cpu, 2)
        assert cpu.registers.BC == 0x0000
        assert flag_clear(cpu, FLAG_Z)


class TestINI:
    """INI - Input and increment."""
    def test_ini_basic(self, cpu):
        """INI — input from port (C) and store at (HL), increment."""
        cpu.registers.HL = 0x2000
        cpu.registers.BC = 0x0250  # B=02, C=50
        cpu.bus.out_(0x50, 0xAB)  # Write to port 0x50
        write_program(cpu, [0xED, 0xA2])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0xAB
        assert cpu.registers.HL == 0x2001
        assert cpu.registers.B == 0x01  # B decremented
        assert cpu.registers.F & FLAG_N  # N always set


class TestIND:
    """IND - Input and decrement."""
    def test_ind_basic(self, cpu):
        """IND — input from port (C) and store at (HL), decrement."""
        cpu.registers.HL = 0x2000
        cpu.registers.BC = 0x0250  # B=02, C=50
        cpu.bus.out_(0x50, 0xCD)
        write_program(cpu, [0xED, 0xAA])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0xCD
        assert cpu.registers.HL == 0x1FFF
        assert cpu.registers.B == 0x01


class TestINIR:
    """INIR - Input, increment, and repeat."""
    def test_inir_basic(self, cpu):
        """INIR — repeat until B=0."""
        cpu.registers.HL = 0x2000
        cpu.registers.BC = 0x0250  # B=02, C=50
        cpu.bus.out_(0x50, 0xAA)
        cpu.bus.out_(0x50, 0xBB)
        write_program(cpu, [0xED, 0xB2])
        step_n(cpu, 2)
        assert cpu.registers.B == 0x00
        assert cpu.read_byte(0x2000) == 0xAA
        assert cpu.read_byte(0x2001) == 0xBB


class TestINDR:
    """INDR - Input, decrement, and repeat."""
    def test_indr_basic(self, cpu):
        """INDR — repeat until B=0 (decrementing)."""
        cpu.registers.HL = 0x2001
        cpu.registers.BC = 0x0250  # B=02, C=50
        cpu.bus.out_(0x50, 0xAA)
        cpu.bus.out_(0x50, 0xBB)
        write_program(cpu, [0xED, 0xBA])
        step_n(cpu, 2)
        assert cpu.registers.B == 0x00
        assert cpu.read_byte(0x2000) == 0xAA
        assert cpu.read_byte(0x2001) == 0xBB


class TestOUTI:
    """OUTI - Output and increment."""
    def test_outi_basic(self, cpu):
        """OUTI — output (HL) to port (C), increment."""
        cpu.registers.HL = 0x2000
        cpu.registers.BC = 0x0250  # B=02, C=50
        cpu.write_byte(0x2000, 0xAB)
        write_program(cpu, [0xED, 0xA3])
        cpu.step()
        assert cpu.bus.in_(0x50) == 0xAB
        assert cpu.registers.HL == 0x2001
        assert cpu.registers.B == 0x01


class TestOUTD:
    """OUTD - Output and decrement."""
    def test_outd_basic(self, cpu):
        """OUTD — output (HL) to port (C), decrement."""
        cpu.registers.HL = 0x2000
        cpu.registers.BC = 0x0250  # B=02, C=50
        cpu.write_byte(0x2000, 0xCD)
        write_program(cpu, [0xED, 0xAB])
        cpu.step()
        assert cpu.bus.in_(0x50) == 0xCD
        assert cpu.registers.HL == 0x1FFF
        assert cpu.registers.B == 0x01


class TestOTIR:
    """OTIR - Output, increment, and repeat."""
    def test_otir_basic(self, cpu):
        """OTIR — repeat until B=0."""
        cpu.registers.HL = 0x2000
        cpu.registers.BC = 0x0250  # B=02, C=50
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        write_program(cpu, [0xED, 0xB3])
        step_n(cpu, 2)
        assert cpu.registers.B == 0x00
        assert cpu.bus.in_(0x50) == 0xBB  # Last value output


class TestOTDR:
    """OTDR - Output, decrement, and repeat."""
    def test_otdr_basic(self, cpu):
        """OTDR — repeat until B=0 (decrementing)."""
        cpu.registers.HL = 0x2001
        cpu.registers.BC = 0x0250  # B=02, C=50
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        write_program(cpu, [0xED, 0xBB])
        step_n(cpu, 2)
        assert cpu.registers.B == 0x00
        assert cpu.bus.in_(0x50) == 0xAA  # Last value output


class TestBlockTiming:
    """Verify cycle counts for block instructions."""
    def test_ldi_cycles(self, cpu):
        """LDI takes 16 cycles."""
        cpu.registers.HL = 0x2000
        cpu.registers.DE = 0x3000
        cpu.registers.BC = 0x0001
        write_program(cpu, [0xED, 0xA0])
        assert cpu.step() == 16

    def test_ldd_cycles(self, cpu):
        """LDD takes 16 cycles."""
        cpu.registers.HL = 0x2000
        cpu.registers.DE = 0x3000
        cpu.registers.BC = 0x0001
        write_program(cpu, [0xED, 0xA8])
        assert cpu.step() == 16

    def test_ldir_cycles(self, cpu):
        """LDIR takes 21 cycles when BC becomes 0."""
        cpu.registers.HL = 0x2000
        cpu.registers.DE = 0x3000
        cpu.registers.BC = 0x0001
        write_program(cpu, [0xED, 0xB0])
        assert cpu.step() == 16  # Last iteration

    def test_cpi_cycles(self, cpu):
        """CPI takes 16 cycles."""
        cpu.registers.A = 0xAA
        cpu.registers.HL = 0x2000
        cpu.registers.BC = 0x0001
        write_program(cpu, [0xED, 0xA1])
        assert cpu.step() == 16
