#!/usr/bin/env python3
"""8-bit load instruction tests."""

import pytest
from conftest import write_program


class TestLoad8Bit:
    """8-bit load instruction tests."""

    @pytest.mark.parametrize(
        "reg,opcode",
        [
            ("A", 0x3E),
            ("B", 0x06),
            ("C", 0x0E),
            ("D", 0x16),
            ("E", 0x1E),
            ("H", 0x26),
            ("L", 0x2E),
        ],
    )
    def test_ld_r_n(self, cpu, reg, opcode):
        """LD r,n — load immediate byte into register."""
        write_program(cpu, [opcode, 0xAB])
        cpu.step()
        assert getattr(cpu.regs, reg) == 0xAB

    def test_ld_a_bc_indirect(self, cpu):
        """LD A,(BC) — load from address in BC."""
        cpu.regs.BC = 0x1234
        cpu.write_byte(0x1234, 0x77)
        write_program(cpu, [0x0A])
        cpu.step()
        assert cpu.regs.A == 0x77

    def test_ld_a_de_indirect(self, cpu):
        """LD A,(DE) — load from address in DE."""
        cpu.regs.DE = 0x1234
        cpu.write_byte(0x1234, 0x55)
        write_program(cpu, [0x1A])
        cpu.step()
        assert cpu.regs.A == 0x55

    def test_ld_hl_indirect_n(self, cpu):
        """LD (HL),n — store immediate byte at address in HL."""
        cpu.regs.HL = 0x2000
        write_program(cpu, [0x36, 0xCC])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0xCC

    def test_ld_a_nn_indirect(self, cpu):
        """LD A,(nn) — load from absolute address."""
        cpu.write_byte(0x3000, 0x99)
        write_program(cpu, [0x3A, 0x00, 0x30])
        cpu.step()
        assert cpu.regs.A == 0x99

    def test_ld_nn_indirect_a(self, cpu):
        """LD (nn),A — store A at absolute address."""
        cpu.regs.A = 0xBB
        write_program(cpu, [0x32, 0x00, 0x30])
        cpu.step()
        assert cpu.read_byte(0x3000) == 0xBB

    @pytest.mark.parametrize(
        "opcode,src,dst_getter",
        [
            (0x78, "B", "A"),
            (0x79, "C", "A"),
            (0x7A, "D", "A"),
            (0x7B, "E", "A"),
            (0x7C, "H", "A"),
            (0x7D, "L", "A"),
            (0x47, "A", "B"),
            (0x48, "B", "C"),
            (0x50, "B", "D"),
            (0x58, "B", "E"),
            (0x60, "B", "H"),
            (0x68, "B", "L"),
        ],
    )
    def test_ld_r_r(self, cpu, opcode, src, dst_getter):
        """LD r,r' — register to register transfer."""
        setattr(cpu.regs, src, 0x42)
        write_program(cpu, [opcode])
        cpu.step()
        assert getattr(cpu.regs, dst_getter) == 0x42

    def test_ld_bc_indirect_a(self, cpu):
        """LD (BC),A — store A at address in BC."""
        cpu.regs.A = 0xEE
        cpu.regs.BC = 0x3000
        write_program(cpu, [0x02])
        cpu.step()
        assert cpu.read_byte(0x3000) == 0xEE

    def test_ld_de_indirect_a(self, cpu):
        """LD (DE),A — store A at address in DE."""
        cpu.regs.A = 0xDD
        cpu.regs.DE = 0x3000
        write_program(cpu, [0x12])
        cpu.step()
        assert cpu.read_byte(0x3000) == 0xDD

    def test_ld_hl_indirect_r(self, cpu):
        """LD (HL),r — store register at address in HL."""
        cpu.regs.HL = 0x2000
        cpu.regs.B = 0x42
        write_program(cpu, [0x70])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x42

    def test_ld_r_hl_indirect(self, cpu):
        """LD r,(HL) — load register from address in HL."""
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0x99)
        write_program(cpu, [0x46])
        cpu.step()
        assert cpu.regs.B == 0x99
