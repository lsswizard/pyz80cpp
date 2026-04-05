#!/usr/bin/env python3
"""CB-prefixed BIT, SET, RES instruction tests."""
import pytest
from conftest import write_program, run_cb_instruction, flag_set, flag_clear, FLAG_Z, FLAG_H, FLAG_C, FLAG_PV, FLAG_S

class TestBitSetRes:
    def test_bit_0_set(self, cpu):
        cpu.regs.A = 0x01
        run_cb_instruction(cpu, 0x47)
        assert flag_clear(cpu, FLAG_Z)
        assert flag_set(cpu, FLAG_H)

    def test_bit_0_clear(self, cpu):
        cpu.regs.A = 0x00
        run_cb_instruction(cpu, 0x47)
        assert flag_set(cpu, FLAG_Z)

    def test_bit_7_set(self, cpu):
        cpu.regs.A = 0x80
        run_cb_instruction(cpu, 0x7F)
        assert flag_set(cpu, FLAG_S)

    def test_bit_hl_indirect(self, cpu):
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0x01)
        run_cb_instruction(cpu, 0x46)
        assert flag_clear(cpu, FLAG_Z)

    def test_set_0_a(self, cpu):
        cpu.regs.A = 0x00
        run_cb_instruction(cpu, 0xC7)
        assert cpu.regs.A == 0x01

    def test_set_7_a(self, cpu):
        cpu.regs.A = 0x00
        run_cb_instruction(cpu, 0xFF)
        assert cpu.regs.A == 0x80

    def test_res_0_a(self, cpu):
        cpu.regs.A = 0x01
        run_cb_instruction(cpu, 0x87)
        assert cpu.regs.A == 0x00

    def test_res_7_a(self, cpu):
        cpu.regs.A = 0xFF
        run_cb_instruction(cpu, 0xBF)
        assert cpu.regs.A == 0x7F

    def test_set_hl_indirect(self, cpu):
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0x00)
        run_cb_instruction(cpu, 0xC6)
        assert cpu.read_byte(0x2000) == 0x01

    def test_res_hl_indirect(self, cpu):
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0xFF)
        run_cb_instruction(cpu, 0x86)
        assert cpu.read_byte(0x2000) == 0xFE
