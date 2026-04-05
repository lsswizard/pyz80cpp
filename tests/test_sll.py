#!/usr/bin/env python3
"""SLL (undocumented) instruction tests."""
import pytest
from conftest import write_program, run_cb_instruction, flag_set, flag_clear, FLAG_C, FLAG_Z

class TestSLL:
    def test_sll_a(self, cpu):
        cpu.regs.A = 0x00
        run_cb_instruction(cpu, 0x37)
        assert cpu.regs.A == 0x01

    def test_sll_a_carry(self, cpu):
        cpu.regs.A = 0x80
        run_cb_instruction(cpu, 0x37)
        assert cpu.regs.A == 0x01
        assert flag_set(cpu, FLAG_C)

    def test_sll_a_no_carry(self, cpu):
        cpu.regs.A = 0x40
        run_cb_instruction(cpu, 0x37)
        assert cpu.regs.A == 0x81
        assert flag_clear(cpu, FLAG_C)

    def test_sll_hl(self, cpu):
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0x00)
        run_cb_instruction(cpu, 0x36)
        assert cpu.read_byte(0x2000) == 0x01
