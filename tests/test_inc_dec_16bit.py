#!/usr/bin/env python3
"""16-bit increment and decrement (no flags affected)."""

import pytest
from conftest import write_program, flag_set, FLAG_Z, FLAG_C


class TestIncDec16Bit:
    """16-bit increment and decrement (no flags affected)."""

    def test_inc_bc_wrap(self, cpu):
        """INC BC wraps from 0xFFFF to 0x0000."""
        cpu.regs.BC = 0xFFFF
        write_program(cpu, [0x03])
        cpu.step()
        assert cpu.regs.BC == 0x0000

    def test_dec_de_wrap(self, cpu):
        """DEC DE wraps from 0x0000 to 0xFFFF."""
        cpu.regs.DE = 0x0000
        write_program(cpu, [0x1B])
        cpu.step()
        assert cpu.regs.DE == 0xFFFF

    @pytest.mark.parametrize(
        "pair,inc_op,dec_op",
        [
            ("BC", 0x03, 0x0B),
            ("DE", 0x13, 0x1B),
            ("HL", 0x23, 0x2B),
            ("SP", 0x33, 0x3B),
        ],
    )
    def test_inc_dec_16_all_pairs(self, cpu, pair, inc_op, dec_op):
        """INC/DEC rr — all register pairs."""
        setattr(cpu.regs, pair, 0x1000)
        write_program(cpu, [inc_op])
        cpu.step()
        assert getattr(cpu.regs, pair) == 0x1001

        cpu.reset()
        setattr(cpu.regs, pair, 0x1000)
        write_program(cpu, [dec_op])
        cpu.step()
        assert getattr(cpu.regs, pair) == 0x0FFF

    def test_inc_16_preserves_flags(self, cpu):
        """INC rr — does not modify any flags."""
        cpu.regs.F = FLAG_Z | FLAG_C
        cpu.regs.BC = 0x0001
        write_program(cpu, [0x03])
        cpu.step()
        assert cpu.regs.F == (FLAG_Z | FLAG_C)
