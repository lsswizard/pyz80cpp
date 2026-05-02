#!/usr/bin/env python3
"""Undocumented flag tests (F3, F5)."""

from conftest import write_program, flag_set, FLAG_F5, FLAG_F3


class TestUndocumentedFlags:
    def test_cp_sets_f3_f5_from_operand(self, cpu):
        cpu.registers.A = 0x00
        write_program(cpu, [0xFE, 0x28])
        cpu.step()
        assert flag_set(cpu, FLAG_F5)
        assert flag_set(cpu, FLAG_F3)

    def test_and_sets_f3_f5_from_result(self, cpu):
        cpu.registers.A = 0xFF
        write_program(cpu, [0xE6, 0x28])
        cpu.step()
        assert flag_set(cpu, FLAG_F5)
        assert flag_set(cpu, FLAG_F3)

    def test_or_sets_f3_f5_from_result(self, cpu):
        cpu.registers.A = 0x00
        write_program(cpu, [0xF6, 0x28])
        cpu.step()
        assert flag_set(cpu, FLAG_F5)
        assert flag_set(cpu, FLAG_F3)

    def test_xor_sets_f3_f5_from_result(self, cpu):
        cpu.registers.A = 0xFF
        write_program(cpu, [0xEE, 0xD7])
        cpu.step()
        assert flag_set(cpu, FLAG_F5)
        assert flag_set(cpu, FLAG_F3)

    def test_add_sets_f3_f5_from_result(self, cpu):
        cpu.registers.A = 0x10
        write_program(cpu, [0xC6, 0x18])
        cpu.step()
        assert flag_set(cpu, FLAG_F5)
        assert flag_set(cpu, FLAG_F3)

    def test_sub_sets_f3_f5_from_result(self, cpu):
        cpu.registers.A = 0x30
        write_program(cpu, [0xD6, 0x08])
        cpu.step()
        assert flag_set(cpu, FLAG_F5)
        assert flag_set(cpu, FLAG_F3)

    def test_inc_sets_f3_f5_from_result(self, cpu):
        cpu.registers.A = 0x27
        write_program(cpu, [0x3C])
        cpu.step()
        assert flag_set(cpu, FLAG_F5)
        assert flag_set(cpu, FLAG_F3)

    def test_dec_sets_f3_f5_from_result(self, cpu):
        cpu.registers.A = 0x29
        write_program(cpu, [0x3D])
        cpu.step()
        assert flag_set(cpu, FLAG_F5)
        assert flag_set(cpu, FLAG_F3)
