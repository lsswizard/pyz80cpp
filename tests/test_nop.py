#!/usr/bin/env python3
"""NOP instruction tests."""
import pytest
from conftest import write_program

class TestNop:
    def test_nop_does_nothing(self, cpu):
        cpu.regs.A = 0x42
        write_program(cpu, [0x00])
        cpu.step()
        assert cpu.regs.A == 0x42

    def test_nop_timing(self, cpu):
        write_program(cpu, [0x00])
        assert cpu.step() == 4

    def test_nop_pc_advances(self, cpu):
        write_program(cpu, [0x00])
        cpu.step()
        assert cpu.regs.PC == 1
