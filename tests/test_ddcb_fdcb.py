#!/usr/bin/env python3
"""DDCB/FDCB indexed bit operation tests."""

import pytest
from conftest import (
    write_program,
    flag_set,
    flag_clear,
    FLAG_Z,
    FLAG_C,
    FLAG_S,
    FLAG_PV,
)


class TestDDCBFDCB:
    def test_ddcb_rlc_ixd(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0x80)
        write_program(cpu, [0xDD, 0xCB, 0x10, 0x06])
        cpu.step()
        assert cpu.read_byte(0x1010) == 0x01
        assert flag_set(cpu, FLAG_C)

    def test_ddcb_rl_ixd(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0x80)
        cpu.registers.F = FLAG_C
        write_program(cpu, [0xDD, 0xCB, 0x10, 0x16])
        cpu.step()
        assert cpu.read_byte(0x1010) == 0x01
        assert flag_set(cpu, FLAG_C)

    def test_ddcb_rrc_ixd(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0x01)
        write_program(cpu, [0xDD, 0xCB, 0x10, 0x0E])
        cpu.step()
        assert cpu.read_byte(0x1010) == 0x80
        assert flag_set(cpu, FLAG_C)

    def test_ddcb_rr_ixd(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0x01)
        cpu.registers.F = FLAG_C
        write_program(cpu, [0xDD, 0xCB, 0x10, 0x1E])
        cpu.step()
        assert cpu.read_byte(0x1010) == 0x80
        assert flag_set(cpu, FLAG_C)

    def test_ddcb_sla_ixd(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0x40)
        write_program(cpu, [0xDD, 0xCB, 0x10, 0x26])
        cpu.step()
        assert cpu.read_byte(0x1010) == 0x80
        assert flag_clear(cpu, FLAG_C)

    def test_ddcb_sra_ixd(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0x81)
        write_program(cpu, [0xDD, 0xCB, 0x10, 0x2E])
        cpu.step()
        assert cpu.read_byte(0x1010) == 0xC0
        assert flag_set(cpu, FLAG_C)

    def test_ddcb_srl_ixd(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0x81)
        write_program(cpu, [0xDD, 0xCB, 0x10, 0x3E])
        cpu.step()
        assert cpu.read_byte(0x1010) == 0x40
        assert flag_set(cpu, FLAG_C)

    def test_ddcb_bit_ixd(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0x01)
        write_program(cpu, [0xDD, 0xCB, 0x10, 0x46])
        cpu.step()
        assert flag_clear(cpu, FLAG_Z)

    def test_ddcb_bit_ixd_zero(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0x00)
        write_program(cpu, [0xDD, 0xCB, 0x10, 0x46])
        cpu.step()
        assert flag_set(cpu, FLAG_Z)

    def test_ddcb_res_ixd(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0xFF)
        write_program(cpu, [0xDD, 0xCB, 0x10, 0x86])
        cpu.step()
        assert cpu.read_byte(0x1010) == 0xFE

    def test_ddcb_set_ixd(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0x00)
        write_program(cpu, [0xDD, 0xCB, 0x10, 0xC6])
        cpu.step()
        assert cpu.read_byte(0x1010) == 0x01

    def test_ddcb_rot_b(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0x80)
        cpu.registers.B = 0x00
        write_program(cpu, [0xDD, 0xCB, 0x10, 0x00])
        cpu.step()
        assert cpu.registers.B == 0x01
        assert cpu.read_byte(0x1010) == 0x01

    def test_fdcb_all_operations(self, cpu):
        cpu.registers.IY = 0x1000
        cpu.write_byte(0x1010, 0x80)
        write_program(cpu, [0xFD, 0xCB, 0x10, 0x06])
        cpu.step()
        assert cpu.read_byte(0x1010) == 0x01
        assert flag_set(cpu, FLAG_C)
