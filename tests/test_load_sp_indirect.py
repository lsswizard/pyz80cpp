#!/usr/bin/env python3
"""LD (nn),SP and LD SP,(nn) tests."""
import pytest
from conftest import write_program

class TestLoadSPIndirect:
    def test_ld_nn_indirect_sp(self, cpu):
        cpu.regs.SP = 0x2000
        write_program(cpu, [0xED, 0x73, 0x00, 0x10])
        cpu.step()
        assert cpu.read_byte(0x1000) == 0x00
        assert cpu.read_byte(0x1001) == 0x20

    def test_ld_sp_nn_indirect(self, cpu):
        cpu.write_byte(0x4000, 0x34)
        cpu.write_byte(0x4001, 0x12)
        write_program(cpu, [0xED, 0x7B, 0x00, 0x40])
        cpu.step()
        assert cpu.regs.SP == 0x1234
