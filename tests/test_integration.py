#!/usr/bin/env python3
"""Integration tests."""

import pytest
from conftest import write_program, step_n


class TestIntegration:
    def test_memcpy(self, cpu):
        src = 0x1000
        dst = 0x2000
        length = 0x0003
        for i in range(length):
            cpu.write_byte(src + i, 0x40 + i)
        cpu.registers.HL = src
        cpu.registers.DE = dst
        cpu.registers.BC = length
        write_program(cpu, [0xED, 0xB0])
        step_n(cpu, 3)
        assert cpu.registers.BC == 0
        assert cpu.read_byte(dst) == 0x40
        assert cpu.read_byte(dst + 1) == 0x41
        assert cpu.read_byte(dst + 2) == 0x42

    def test_array_sum(self, cpu):
        array_addr = 0x1000
        array = [0x01, 0x02, 0x03]
        for i, val in enumerate(array):
            cpu.write_byte(array_addr + i, val)
        program = [
            0x21,
            0x00,
            0x10,  # LD HL, array
            0x06,
            0x03,  # LD B, 3
            0xAF,  # XOR A (A=0)
            0x86,  # ADD A, (HL)
            0x23,  # INC HL
            0x10,
            0xFB,  # DJNZ -6
            0x76,  # HALT
        ]
        write_program(cpu, program)
        step_n(cpu, 14)
        assert cpu.registers.A == 0x03

    def test_call_ret_chain(self, cpu):
        cpu.registers.SP = 0x2000
        cpu.write_byte(0x0000, 0xCD)
        cpu.write_byte(0x0001, 0x10)
        cpu.write_byte(0x0002, 0x00)
        cpu.write_byte(0x0003, 0x76)
        cpu.write_byte(0x0010, 0xCD)
        cpu.write_byte(0x0011, 0x20)
        cpu.write_byte(0x0012, 0x00)
        cpu.write_byte(0x0013, 0xC9)
        cpu.write_byte(0x0020, 0xC9)
        cpu.step()
        cpu.step()
        cpu.step()
        cpu.step()
        assert cpu.registers.PC == 0x0003

    def test_self_modifying_code(self, cpu):
        cpu.write_byte(0x0000, 0x3E)
        cpu.write_byte(0x0001, 0x00)
        cpu.registers.PC = 0
        cpu.step()
        cpu.write_byte(0x0000, 0x3E)
        cpu.write_byte(0x0001, 0xFF)
        cpu.registers.PC = 0
        cpu.step()
        assert cpu.registers.A == 0xFF

    def test_stack_operations(self, cpu):
        cpu.registers.SP = 0x2000
        cpu.registers.BC = 0x1111
        cpu.registers.DE = 0x2222
        cpu.registers.HL = 0x3333
        cpu.registers.AF = 0x4455
        program = [0xC5, 0xD5, 0xE5, 0xF5, 0xF1, 0xE1, 0xD1, 0xC1]
        write_program(cpu, program)
        step_n(cpu, 8)
        assert cpu.registers.BC == 0x1111
        assert cpu.registers.DE == 0x2222
        assert cpu.registers.HL == 0x3333
        assert cpu.registers.AF == 0x4455
