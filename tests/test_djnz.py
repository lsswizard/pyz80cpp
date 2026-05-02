#!/usr/bin/env python3
"""DJNZ instruction tests."""
from conftest import write_program

class TestDJNZ:
    def test_djnz_branch(self, cpu):
        cpu.registers.B = 2
        write_program(cpu, [0x10, 0x00])
        cpu.step()
        assert cpu.registers.B == 1
        assert cpu.registers.PC == 2

    def test_djnz_no_branch(self, cpu):
        cpu.registers.B = 1
        write_program(cpu, [0x10, 0x04])
        cpu.step()
        assert cpu.registers.B == 0
        assert cpu.registers.PC == 2

    def test_djnz_backward_loop(self, cpu):
        cpu.registers.B = 3
        write_program(cpu, [0x10, 0xFE], 0x0010)
        cpu.step()
        assert cpu.registers.B == 2
        assert cpu.registers.PC == 0x0010

    def test_djnz_count_to_zero(self, cpu):
        cpu.registers.B = 3
        write_program(cpu, [0x10, 0xFE])
        cpu.step()
        assert cpu.registers.B == 2
        cpu.step()
        assert cpu.registers.B == 1
        cpu.step()
        assert cpu.registers.B == 0
        assert cpu.registers.PC == 2
