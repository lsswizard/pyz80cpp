#!/usr/bin/env python3
"""DD/FD prefix fallthrough tests."""
import pytest
from conftest import write_program

class TestDDFDFallthrough:
    def test_dd_nop(self, cpu):
        write_program(cpu, [0xDD, 0x00])
        assert cpu.step() == 8
        assert cpu.regs.PC == 2

    def test_fd_nop(self, cpu):
        write_program(cpu, [0xFD, 0x00])
        assert cpu.step() == 8
        assert cpu.regs.PC == 2

    def test_dd_di(self, cpu):
        cpu.regs.IFF1 = True
        write_program(cpu, [0xDD, 0xF3])
        cpu.step()
        assert not cpu.regs.IFF1

    def test_dd_ld_bc_nn(self, cpu):
        write_program(cpu, [0xDD, 0x01, 0x34, 0x12])
        assert cpu.step() == 14
        assert cpu.regs.BC == 0x1234

    def test_multiple_dd_prefixes(self, cpu):
        write_program(cpu, [0xDD, 0xDD, 0x00])
        assert cpu.step() == 12
        assert cpu.regs.PC == 3

    def test_dd_fd_nop(self, cpu):
        write_program(cpu, [0xDD, 0xFD, 0x00])
        assert cpu.step() == 12
        assert cpu.regs.PC == 3

    def test_dd_inc_a(self, cpu):
        cpu.regs.A = 0x00
        write_program(cpu, [0xDD, 0x3C])
        cpu.step()
        assert cpu.regs.A == 0x01
