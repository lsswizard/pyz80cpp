#!/usr/bin/env python3
"""DJNZ instruction tests."""
import pytest
from conftest import write_program

class TestDJNZ:
    def test_djnz_branch(self, cpu):
        cpu.regs.B = 2
        write_program(cpu, [0x10, 0x00])
        cpu.step()
        assert cpu.regs.B == 1
        assert cpu.regs.PC == 2

    def test_djnz_no_branch(self, cpu):
        cpu.regs.B = 1
        write_program(cpu, [0x10, 0x04])
        cpu.step()
        assert cpu.regs.B == 0
        assert cpu.regs.PC == 2

    def test_djnz_backward_loop(self, cpu):
        cpu.regs.B = 3
        write_program(cpu, [0x10, 0xFE], 0x0010)
        cpu.step()
        assert cpu.regs.B == 2
        assert cpu.regs.PC == 0x0010

    def test_djnz_count_to_zero(self, cpu):
        cpu.regs.B = 3
        write_program(cpu, [0x10, 0xFE])
        cpu.step()
        assert cpu.regs.B == 2
        cpu.step()
        assert cpu.regs.B == 1
        cpu.step()
        assert cpu.regs.B == 0
        assert cpu.regs.PC == 2
