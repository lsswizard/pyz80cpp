#!/usr/bin/env python3
"""HALT instruction tests."""
from conftest import write_program

class TestHalt:
    def test_halt_sets_halted(self, cpu):
        write_program(cpu, [0x76])
        cpu.step()
        assert cpu.is_halted

    def test_halt_pc_unchanged(self, cpu):
        write_program(cpu, [0x76])
        cpu.step()
        assert cpu.registers.PC == 0

    def test_halt_timing(self, cpu):
        write_program(cpu, [0x76])
        assert cpu.step() == 4

    def test_halt_loop(self, cpu):
        write_program(cpu, [0x76])
        cpu.step()
        assert cpu.is_halted
        assert cpu.step() == 4
        assert cpu.is_halted
