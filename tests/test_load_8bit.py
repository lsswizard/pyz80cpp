#!/usr/bin/env python3
"""Comprehensive 8-bit load instruction tests."""
import pytest
from conftest import (
    write_program
)


class TestLoad8BitImmediate:
    """LD r,n - Load immediate byte into register."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x06), ("C", 0x0E), ("D", 0x16), ("E", 0x1E),
        ("H", 0x26), ("L", 0x2E), ("A", 0x3E),
    ])
    def test_ld_r_n(self, cpu, reg, opcode):
        """LD r,n — load immediate byte into register."""
        write_program(cpu, [opcode, 0xAB])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0xAB

    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x06), ("C", 0x0E), ("D", 0x16), ("E", 0x1E),
        ("H", 0x26), ("L", 0x2E), ("A", 0x3E),
    ])
    def test_ld_r_n_zero(self, cpu, reg, opcode):
        """LD r,n — load 0x00."""
        write_program(cpu, [opcode, 0x00])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0x00

    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x06), ("C", 0x0E), ("D", 0x16), ("E", 0x1E),
        ("H", 0x26), ("L", 0x2E), ("A", 0x3E),
    ])
    def test_ld_r_n_ff(self, cpu, reg, opcode):
        """LD r,n — load 0xFF."""
        write_program(cpu, [opcode, 0xFF])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0xFF


class TestLoad8BitRegister:
    """LD r,r' - Register to register transfers."""
    @pytest.mark.parametrize("dst,src,opcode", [
        ("B", "B", 0x40), ("B", "C", 0x41), ("B", "D", 0x42), ("B", "E", 0x43),
        ("B", "H", 0x44), ("B", "L", 0x45), ("B", "A", 0x47),
        ("C", "B", 0x48), ("C", "C", 0x49), ("C", "D", 0x4A), ("C", "E", 0x4B),
        ("C", "H", 0x4C), ("C", "L", 0x4D), ("C", "A", 0x4F),
        ("D", "B", 0x50), ("D", "C", 0x51), ("D", "D", 0x52), ("D", "E", 0x53),
        ("D", "H", 0x54), ("D", "L", 0x55), ("D", "A", 0x57),
        ("E", "B", 0x58), ("E", "C", 0x59), ("E", "D", 0x5A), ("E", "E", 0x5B),
        ("E", "H", 0x5C), ("E", "L", 0x5D), ("E", "A", 0x5F),
        ("H", "B", 0x60), ("H", "C", 0x61), ("H", "D", 0x62), ("H", "E", 0x63),
        ("H", "H", 0x64), ("H", "L", 0x65), ("H", "A", 0x67),
        ("L", "B", 0x68), ("L", "C", 0x69), ("L", "D", 0x6A), ("L", "E", 0x6B),
        ("L", "H", 0x6C), ("L", "L", 0x6D), ("L", "A", 0x6F),
        ("A", "B", 0x78), ("A", "C", 0x79), ("A", "D", 0x7A), ("A", "E", 0x7B),
        ("A", "H", 0x7C), ("A", "L", 0x7D), ("A", "A", 0x7F),
    ])
    def test_ld_r_r(self, cpu, dst, src, opcode):
        """LD r,r' — register to register transfer."""
        setattr(cpu.registers, src, 0x42)
        write_program(cpu, [opcode])
        cpu.step()
        assert getattr(cpu.registers, dst) == 0x42

    def test_ld_a_a_no_change(self, cpu):
        """LD A,A — A unchanged."""
        cpu.registers.A = 0x55
        write_program(cpu, [0x7F])
        cpu.step()
        assert cpu.registers.A == 0x55


class TestLoad8BitHLIndirect:
    """LD r,(HL) and LD (HL),r - Load/store via HL indirect."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x46), ("C", 0x4E), ("D", 0x56), ("E", 0x5E),
        ("H", 0x66), ("L", 0x6E), ("A", 0x7E),
    ])
    def test_ld_r_hl_indirect(self, cpu, reg, opcode):
        """LD r,(HL) — load from address in HL."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x99)
        write_program(cpu, [opcode])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0x99

    @pytest.mark.parametrize("reg,opcode,hl_val", [
        ("B", 0x70, 0x2000), ("C", 0x71, 0x2000), ("D", 0x72, 0x2000), ("E", 0x73, 0x2000),
        ("H", 0x74, 0x4200), ("L", 0x75, 0x2042), ("A", 0x77, 0x2000),
    ])
    def test_ld_hl_indirect_r(self, cpu, reg, opcode, hl_val):
        """LD (HL),r — store register at address in HL."""
        cpu.registers.HL = hl_val
        setattr(cpu.registers, reg, 0x42)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.read_byte(hl_val) == 0x42

    def test_ld_h_hl(self, cpu):
        """LD H,(HL) — special case loads H from (HL)."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x55)
        write_program(cpu, [0x66])
        cpu.step()
        assert cpu.registers.H == 0x55

    def test_ld_l_hl(self, cpu):
        """LD L,(HL) — special case loads L from (HL)."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0xAA)
        write_program(cpu, [0x6E])
        cpu.step()
        assert cpu.registers.L == 0xAA

    def test_ld_hl_h(self, cpu):
        """LD (HL),H — store H at address in HL."""
        cpu.registers.HL = 0x2000  # H=0x20, L=0x00
        cpu.registers.H = 0x55     # This changes HL to 0x5500
        write_program(cpu, [0x74])
        cpu.step()
        assert cpu.read_byte(0x5500) == 0x55

    def test_ld_hl_l(self, cpu):
        """LD (HL),L — store L at address in HL."""
        cpu.registers.HL = 0x2000  # H=0x20, L=0x00  
        cpu.registers.L = 0xAA    # This changes HL to 0x20AA
        write_program(cpu, [0x75])
        cpu.step()
        assert cpu.read_byte(0x20AA) == 0xAA


class TestLoad8BitBCDEIndirect:
    """LD A,(BC), LD A,(DE), LD (BC),A, LD (DE),A."""
    def test_ld_a_bc_indirect(self, cpu):
        """LD A,(BC) — load from address in BC."""
        cpu.registers.BC = 0x1234
        cpu.write_byte(0x1234, 0x77)
        write_program(cpu, [0x0A])
        cpu.step()
        assert cpu.registers.A == 0x77

    def test_ld_a_de_indirect(self, cpu):
        """LD A,(DE) — load from address in DE."""
        cpu.registers.DE = 0x1234
        cpu.write_byte(0x1234, 0x55)
        write_program(cpu, [0x1A])
        cpu.step()
        assert cpu.registers.A == 0x55

    def test_ld_bc_indirect_a(self, cpu):
        """LD (BC),A — store A at address in BC."""
        cpu.registers.A = 0xEE
        cpu.registers.BC = 0x3000
        write_program(cpu, [0x02])
        cpu.step()
        assert cpu.read_byte(0x3000) == 0xEE

    def test_ld_de_indirect_a(self, cpu):
        """LD (DE),A — store A at address in DE."""
        cpu.registers.A = 0xDD
        cpu.registers.DE = 0x3000
        write_program(cpu, [0x12])
        cpu.step()
        assert cpu.read_byte(0x3000) == 0xDD


class TestLoad8BitAbsolute:
    """LD A,(nn) and LD (nn),A - Absolute address loads/stores."""
    def test_ld_a_nn_indirect(self, cpu):
        """LD A,(nn) — load from absolute address."""
        cpu.write_byte(0x3000, 0x99)
        write_program(cpu, [0x3A, 0x00, 0x30])
        cpu.step()
        assert cpu.registers.A == 0x99

    def test_ld_nn_indirect_a(self, cpu):
        """LD (nn),A — store A at absolute address."""
        cpu.registers.A = 0xBB
        write_program(cpu, [0x32, 0x00, 0x30])
        cpu.step()
        assert cpu.read_byte(0x3000) == 0xBB

    def test_ld_a_nn_wrap(self, cpu):
        """LD A,(nn) — test with address at memory boundary."""
        cpu.write_byte(0xFFFF, 0xAA)
        write_program(cpu, [0x3A, 0xFF, 0xFF])
        cpu.step()
        assert cpu.registers.A == 0xAA

    def test_ld_nn_a_wrap(self, cpu):
        """LD (nn),A — test with address at memory boundary."""
        cpu.registers.A = 0xBB
        write_program(cpu, [0x32, 0xFF, 0xFF])
        cpu.step()
        assert cpu.read_byte(0xFFFF) == 0xBB


class TestLoad8BitHLImmediate:
    """LD (HL),n - Load immediate byte at address in HL."""
    def test_ld_hl_indirect_n(self, cpu):
        """LD (HL),n — store immediate byte at address in HL."""
        cpu.registers.HL = 0x2000
        write_program(cpu, [0x36, 0xCC])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0xCC

    def test_ld_hl_indirect_n_zero(self, cpu):
        """LD (HL),n — store 0x00."""
        cpu.registers.HL = 0x2000
        write_program(cpu, [0x36, 0x00])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x00

    def test_ld_hl_indirect_n_ff(self, cpu):
        """LD (HL),n — store 0xFF."""
        cpu.registers.HL = 0x2000
        write_program(cpu, [0x36, 0xFF])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0xFF


class TestLoad8BitFlags:
    """Verify 8-bit loads don't affect flags."""
    def test_ld_r_n_preserves_flags(self, cpu):
        """LD r,n does not affect flags."""
        cpu.registers.F = 0xFF
        write_program(cpu, [0x3E, 0x42])  # LD A,0x42
        cpu.step()
        assert cpu.registers.F == 0xFF

    def test_ld_r_r_preserves_flags(self, cpu):
        """LD r,r does not affect flags."""
        cpu.registers.F = 0xFF
        cpu.registers.B = 0x42
        write_program(cpu, [0x78])  # LD A,B
        cpu.step()
        assert cpu.registers.F == 0xFF

    def test_ld_hl_indirect_preserves_flags(self, cpu):
        """LD r,(HL) does not affect flags."""
        cpu.registers.F = 0xFF
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x42)
        write_program(cpu, [0x7E])  # LD A,(HL)
        cpu.step()
        assert cpu.registers.F == 0xFF


class TestLoad8BitCycles:
    """Verify cycle counts for 8-bit load instructions."""
    def test_ld_r_n_cycles(self, cpu):
        """LD r,n takes 7 cycles."""
        write_program(cpu, [0x3E, 0x42])
        assert cpu.step() == 7

    def test_ld_r_r_cycles(self, cpu):
        """LD r,r takes 4 cycles."""
        write_program(cpu, [0x78])
        assert cpu.step() == 4

    def test_ld_r_hl_cycles(self, cpu):
        """LD r,(HL) takes 7 cycles."""
        cpu.registers.HL = 0x2000
        write_program(cpu, [0x7E])
        assert cpu.step() == 7

    def test_ld_hl_r_cycles(self, cpu):
        """LD (HL),r takes 7 cycles."""
        cpu.registers.HL = 0x2000
        write_program(cpu, [0x70])
        assert cpu.step() == 7

    def test_ld_a_bc_cycles(self, cpu):
        """LD A,(BC) takes 7 cycles."""
        cpu.registers.BC = 0x1000
        write_program(cpu, [0x0A])
        assert cpu.step() == 7

    def test_ld_a_de_cycles(self, cpu):
        """LD A,(DE) takes 7 cycles."""
        cpu.registers.DE = 0x1000
        write_program(cpu, [0x1A])
        assert cpu.step() == 7

    def test_ld_a_nn_cycles(self, cpu):
        """LD A,(nn) takes 13 cycles."""
        write_program(cpu, [0x3A, 0x00, 0x10])
        assert cpu.step() == 13

    def test_ld_nn_a_cycles(self, cpu):
        """LD (nn),A takes 13 cycles."""
        write_program(cpu, [0x32, 0x00, 0x10])
        assert cpu.step() == 13

    def test_ld_hl_n_cycles(self, cpu):
        """LD (HL),n takes 10 cycles."""
        cpu.registers.HL = 0x2000
        write_program(cpu, [0x36, 0x00])
        assert cpu.step() == 10
