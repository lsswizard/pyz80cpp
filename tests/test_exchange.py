#!/usr/bin/env python3
"""Register exchange instruction tests."""
import pytest
from conftest import write_program

class TestExchange:
    def test_ex_af_af_prime_round_trip(self, cpu):
        cpu.regs.A = 0x11
        cpu.regs.F = 0x22
        write_program(cpu, [0x08])
        cpu.step()
        cpu.regs.A = 0x00
        cpu.regs.F = 0x00
        write_program(cpu, [0x08])
        cpu.step()
        assert cpu.regs.A == 0x11

    def test_exx_round_trip(self, cpu):
        cpu.regs.BC = 0x1111
        cpu.regs.DE = 0x2222
        cpu.regs.HL = 0x3333
        write_program(cpu, [0xD9])
        cpu.step()
        write_program(cpu, [0xD9])
        cpu.step()
        assert cpu.regs.BC == 0x1111
        assert cpu.regs.DE == 0x2222
        assert cpu.regs.HL == 0x3333

    def test_ex_de_hl(self, cpu):
        cpu.regs.DE = 0xAAAA
        cpu.regs.HL = 0xBBBB
        write_program(cpu, [0xEB])
        cpu.step()
        assert cpu.regs.DE == 0xBBBB
        assert cpu.regs.HL == 0xAAAA

    def test_ex_sp_hl(self, cpu):
        cpu.regs.SP = 0x1000
        cpu.regs.HL = 0x1234
        cpu.write_byte(0x1000, 0x78)
        cpu.write_byte(0x1001, 0x56)
        write_program(cpu, [0xE3])
        cpu.step()
        assert cpu.regs.HL == 0x5678
        assert cpu.read_byte(0x1000) == 0x34
        assert cpu.read_byte(0x1001) == 0x12

    def test_ex_sp_ix(self, cpu):
        cpu.regs.SP = 0x1000
        cpu.regs.IX = 0xABCD
        cpu.write_byte(0x1000, 0x34)
        cpu.write_byte(0x1001, 0x12)
        write_program(cpu, [0xDD, 0xE3])
        cpu.step()
        assert cpu.regs.IX == 0x1234
        assert cpu.read_byte(0x1000) == 0xCD
        assert cpu.read_byte(0x1001) == 0xAB

    def test_ex_sp_iy(self, cpu):
        cpu.regs.SP = 0x1000
        cpu.regs.IY = 0xABCD
        cpu.write_byte(0x1000, 0x34)
        cpu.write_byte(0x1001, 0x12)
        write_program(cpu, [0xFD, 0xE3])
        cpu.step()
        assert cpu.regs.IY == 0x1234
        assert cpu.read_byte(0x1000) == 0xCD
        assert cpu.read_byte(0x1001) == 0xAB
