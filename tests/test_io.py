#!/usr/bin/env python3
"""I/O instruction tests."""

import pytest
from conftest import write_program


class TestIO:
    def test_in_a_n(self, cpu):
        cpu.regs.A = 0x50
        # Use bus.out() for I/O port
        cpu.bus.out(0x50, 0xAB)
        write_program(cpu, [0xDB, 0x50])
        cpu.step()
        assert cpu.regs.A == 0xAB

    def test_out_n_a(self, cpu):
        cpu.regs.A = 0xAB
        write_program(cpu, [0xD3, 0x50])
        cpu.step()
        # Port address is A << 8 | n = 0xAB50
        # Use bus.in_() (alias for 'in' to avoid Python keyword)
        assert cpu.bus.in_(0x50) == 0xAB

    def test_in_b_c(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.bus.out(0x50, 0xAB)
        write_program(cpu, [0xED, 0x40])
        cpu.step()
        assert cpu.regs.B == 0xAB

    def test_out_c_b(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.regs.B = 0xAB
        write_program(cpu, [0xED, 0x41])
        cpu.step()
        assert cpu.bus.in_(0x50) == 0xAB

    def test_in_f_c(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.bus.out(0x50, 0xAB)
        write_program(cpu, [0xED, 0x70])
        cpu.step()
        # Check that flags were updated (S flag should be set for 0xAB)
        assert cpu.regs.F & 0x80
