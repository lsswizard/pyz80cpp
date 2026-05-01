#!/usr/bin/env python3
"""Comprehensive DAA tests."""

import pytest
from conftest import (
    write_program,
    step_n,
    flag_set,
    flag_clear,
    FLAG_C,
    FLAG_Z,
    FLAG_H,
    FLAG_N,
)


class TestDAAComprehensive:
    def test_daa_00_after_add(self, cpu):
        cpu.registers.A = 0x00
        write_program(cpu, [0x27])
        cpu.step()
        assert cpu.registers.A == 0x00
        assert flag_set(cpu, FLAG_Z)

    def test_daa_09_01(self, cpu):
        cpu.registers.A = 0x0A
        write_program(cpu, [0x27])
        cpu.step()
        assert cpu.registers.A == 0x10

    def test_daa_99_01(self, cpu):
        cpu.registers.A = 0x9A
        write_program(cpu, [0x27])
        cpu.step()
        assert cpu.registers.A == 0x00
        assert flag_set(cpu, FLAG_C)

    def test_daa_after_sub(self, cpu):
        cpu.registers.A = 0x09
        cpu.registers.B = 0x09
        write_program(cpu, [0x90, 0x27])  # SUB B, then DAA
        step_n(cpu, 2)
        assert cpu.registers.A == 0x00
        assert flag_set(cpu, FLAG_Z)
