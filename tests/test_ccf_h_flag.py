#!/usr/bin/env python3
"""CCF H flag tests."""

from conftest import write_program, flag_set, flag_clear, FLAG_C, FLAG_H, FLAG_N


class TestCCFHFlag:
    def test_ccf_h_gets_old_carry_set(self, cpu):
        cpu.registers.F = FLAG_C
        write_program(cpu, [0x3F])
        cpu.step()
        assert flag_set(cpu, FLAG_H)

    def test_ccf_h_gets_old_carry_clear(self, cpu):
        cpu.registers.F = 0
        write_program(cpu, [0x3F])
        cpu.step()
        assert flag_set(cpu, FLAG_C)
        assert flag_clear(cpu, FLAG_H)

    def test_ccf_n_always_clear(self, cpu):
        cpu.registers.F = FLAG_N | FLAG_C
        write_program(cpu, [0x3F])
        cpu.step()
        assert flag_clear(cpu, FLAG_N)

    def test_ccf_preserves_s_z_pv(self, cpu):
        cpu.registers.F = 0xC4
        write_program(cpu, [0x3F])
        cpu.step()
        assert flag_set(cpu, 0x80)
        assert flag_set(cpu, 0x40)
        assert flag_set(cpu, 0x04)
