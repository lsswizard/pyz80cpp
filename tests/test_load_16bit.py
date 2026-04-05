#!/usr/bin/env python3
"""16-bit load instruction tests."""

import pytest
from conftest import write_program


class TestLoad16Bit:
    """16-bit load instruction tests."""

    @pytest.mark.parametrize(
        "pair,opcode",
        [
            ("BC", 0x01),
            ("DE", 0x11),
            ("HL", 0x21),
            ("SP", 0x31),
        ],
    )
    def test_ld_rr_nn(self, cpu, pair, opcode):
        """LD rr,nn — load immediate 16-bit value."""
        write_program(cpu, [opcode, 0xCD, 0xAB])
        cpu.step()
        assert getattr(cpu.regs, pair) == 0xABCD

    def test_ld_hl_nn_indirect(self, cpu):
        """LD HL,(nn) — load HL from memory."""
        cpu.write_byte(0x4000, 0x78)
        cpu.write_byte(0x4001, 0x56)
        write_program(cpu, [0x2A, 0x00, 0x40])
        cpu.step()
        assert cpu.regs.HL == 0x5678

    def test_ld_nn_indirect_hl(self, cpu):
        """LD (nn),HL — store HL to memory."""
        cpu.regs.HL = 0x1234
        write_program(cpu, [0x22, 0x00, 0x40])
        cpu.step()
        assert cpu.read_byte(0x4000) == 0x34
        assert cpu.read_byte(0x4001) == 0x12

    def test_ld_sp_hl(self, cpu):
        """LD SP,HL — copy HL to SP."""
        cpu.regs.HL = 0x1234
        write_program(cpu, [0xF9])
        cpu.step()
        assert cpu.regs.SP == 0x1234
