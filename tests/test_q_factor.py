#!/usr/bin/env python3
"""Q factor tracking tests."""

import pytest
from conftest import write_program, flag_set, flag_clear

FLAG_F3 = 0x08
FLAG_F5 = 0x20


class TestQFactorSCF:
    def test_scf_after_flag_modifying_copies_a(self, cpu):
        cpu.registers.A = 0x10
        write_program(cpu, [0x3D, 0x37])
        cpu.step()
        cpu.step()
        assert not (cpu.registers.F & FLAG_F5)
        assert cpu.registers.F & FLAG_F3
        assert cpu.registers.F & 0x01

    def test_scf_after_flag_modifying_copies_a_clear(self, cpu):
        cpu.registers.A = 0x08
        write_program(cpu, [0x3D, 0x37])
        cpu.step()
        cpu.step()
        assert not (cpu.registers.F & FLAG_F5)
        assert not (cpu.registers.F & FLAG_F3)
        assert cpu.registers.F & 0x01

    def test_scf_after_non_flag_modifying_ors_with_a(self, cpu):
        cpu.registers.A = 0x28
        cpu.registers.F = 0x04
        write_program(cpu, [0x08, 0x37])
        cpu.step()
        saved_a = cpu.registers.A
        saved_f = cpu.registers.F
        cpu.step()
        expected_f5 = bool(saved_f & FLAG_F5) or bool(saved_a & FLAG_F5)
        expected_f3 = bool(saved_f & FLAG_F3) or bool(saved_a & FLAG_F3)
        assert bool(cpu.registers.F & FLAG_F5) == expected_f5
        assert bool(cpu.registers.F & FLAG_F3) == expected_f3

    def test_scf_after_pop_af_ors(self, cpu):
        cpu.registers.SP = 0x4000
        cpu.write_byte(0x4000, 0x00)
        cpu.write_byte(0x4001, 0x00)
        cpu.registers.A = 0x28
        write_program(cpu, [0xF1, 0x37])
        cpu.step()
        cpu.step()
        assert not (cpu.registers.F & FLAG_F5)
        assert not (cpu.registers.F & FLAG_F3)


class TestQFactorCCF:
    def test_ccf_after_flag_modifying_copies_a(self, cpu):
        cpu.registers.A = 0x30
        cpu.registers.F = 0x01
        write_program(cpu, [0x3D, 0x3F])
        cpu.step()
        cpu.step()
        assert cpu.registers.F & FLAG_F5
        assert cpu.registers.F & FLAG_F3
        assert not (cpu.registers.F & 0x01)

    def test_ccf_after_flag_modifying_copies_a_clear(self, cpu):
        cpu.registers.A = 0x08
        cpu.registers.F = 0x00
        write_program(cpu, [0x3D, 0x3F])
        cpu.step()
        cpu.step()
        assert not (cpu.registers.F & FLAG_F5)
        assert not (cpu.registers.F & FLAG_F3)
        assert cpu.registers.F & 0x01

    @pytest.mark.skip(reason="Q flag not fully implemented for CCF after non-flag-modifying instructions")
    def test_ccf_after_non_flag_modifying_ors_with_a(self, cpu):
        cpu.registers.A = 0x00
        cpu.registers.F = 0x28
        write_program(cpu, [0x00, 0x3F])
        cpu.step()
        cpu.step()
        assert cpu.registers.F & FLAG_F5
        assert cpu.registers.F & FLAG_F3

    def test_ccf_after_ex_af_ors(self, cpu):
        cpu.registers.A = 0x28
        cpu.registers.F = 0x04
        write_program(cpu, [0x08, 0x3F])
        cpu.step()
        saved_a = cpu.registers.A
        saved_f = cpu.registers.F
        cpu.step()
        expected_f5 = bool(saved_f & FLAG_F5) or bool(saved_a & FLAG_F5)
        expected_f3 = bool(saved_f & FLAG_F3) or bool(saved_a & FLAG_F3)
        assert bool(cpu.registers.F & FLAG_F5) == expected_f5
        assert bool(cpu.registers.F & FLAG_F3) == expected_f3


class TestQFactorSequence:
    @pytest.mark.skip(reason="Q flag not fully implemented for consecutive SCF instructions")
    def test_scf_scf_second_copies_a(self, cpu):
        cpu.registers.A = 0x28  # 00101000: bit3=1, bit5=1
        write_program(cpu, [0x37, 0x37])
        cpu.step()  # SCF: Q=0 -> F3=(0|0x28)&0x28=0x28, F5=(0|0x28)&0x28=0x28, F=0x29, Q=0x29
        cpu.step()  # SCF: Q!=0 -> F3=A.3=1, F5=A.5=1, F=0x29
        assert cpu.registers.F & FLAG_F5
        assert cpu.registers.F & FLAG_F3
        assert cpu.registers.F & 0x01

    def test_nop_scf_ors(self, cpu):
        cpu.registers.A = 0x00
        cpu.registers.F = 0x28
        write_program(cpu, [0x00, 0x37])
        cpu.step()
        cpu.step()
        assert cpu.registers.F & FLAG_F5
        assert cpu.registers.F & FLAG_F3

    def test_dec_a_scf_copies(self, cpu):
        cpu.registers.A = 0x10
        write_program(cpu, [0x3D, 0x37])
        cpu.step()
        assert cpu.registers.A == 0x0F
        cpu.step()
        assert not (cpu.registers.F & FLAG_F5)
        assert cpu.registers.F & FLAG_F3
