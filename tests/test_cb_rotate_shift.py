#!/usr/bin/env python3
"""CB-prefixed rotate and shift instructions."""
import pytest
from conftest import write_program, run_cb_instruction, flag_set, flag_clear, FLAG_C, FLAG_Z

class TestCBRotateShift:
    def test_rlc_a(self, cpu):
        cpu.regs.A = 0x80
        run_cb_instruction(cpu, 0x07)
        assert cpu.regs.A == 0x01
        assert flag_set(cpu, FLAG_C)

    def test_rl_a(self, cpu):
        cpu.regs.A = 0x80
        cpu.regs.F = 0x00
        run_cb_instruction(cpu, 0x17)
        assert cpu.regs.A == 0x00
        assert flag_set(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_Z)

    def test_rrc_a(self, cpu):
        cpu.regs.A = 0x01
        run_cb_instruction(cpu, 0x0F)
        assert cpu.regs.A == 0x80
        assert flag_set(cpu, FLAG_C)

    def test_rr_a(self, cpu):
        cpu.regs.A = 0x01
        cpu.regs.F = 0x00
        run_cb_instruction(cpu, 0x1F)
        assert cpu.regs.A == 0x00
        assert flag_set(cpu, FLAG_C)

    def test_sla_a(self, cpu):
        cpu.regs.A = 0x80
        run_cb_instruction(cpu, 0x27)
        assert cpu.regs.A == 0x00
        assert flag_set(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_Z)

    def test_sla_a_no_carry(self, cpu):
        cpu.regs.A = 0x40
        run_cb_instruction(cpu, 0x27)
        assert cpu.regs.A == 0x80
        assert flag_clear(cpu, FLAG_C)

    def test_sra_a(self, cpu):
        cpu.regs.A = 0x81
        run_cb_instruction(cpu, 0x2F)
        assert cpu.regs.A == 0xC0
        assert flag_set(cpu, FLAG_C)

    def test_sra_a_positive(self, cpu):
        cpu.regs.A = 0x40
        run_cb_instruction(cpu, 0x2F)
        assert cpu.regs.A == 0x20
        assert flag_clear(cpu, FLAG_C)

    def test_srl_a(self, cpu):
        cpu.regs.A = 0x81
        run_cb_instruction(cpu, 0x3F)
        assert cpu.regs.A == 0x40
        assert flag_set(cpu, FLAG_C)

    def test_srl_a_no_carry(self, cpu):
        cpu.regs.A = 0x80
        run_cb_instruction(cpu, 0x3F)
        assert cpu.regs.A == 0x40
        assert flag_clear(cpu, FLAG_C)

    def test_rlc_hl_indirect(self, cpu):
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0x80)
        run_cb_instruction(cpu, 0x06)
        assert cpu.read_byte(0x2000) == 0x01
        assert flag_set(cpu, FLAG_C)
