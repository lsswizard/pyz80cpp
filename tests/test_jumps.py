#!/usr/bin/env python3
"""Jump instruction tests."""
import pytest
from conftest import write_program, flag_set, flag_clear, FLAG_Z, FLAG_C, FLAG_PV, FLAG_S

class TestJumps:
    def test_jp_nn(self, cpu):
        write_program(cpu, [0xC3, 0x00, 0x20])
        cpu.step()
        assert cpu.regs.PC == 0x2000

    def test_jp_hl(self, cpu):
        cpu.regs.HL = 0x3000
        write_program(cpu, [0xE9])
        cpu.step()
        assert cpu.regs.PC == 0x3000

    def test_jr_forward(self, cpu):
        write_program(cpu, [0x18, 0x04])
        cpu.step()
        assert cpu.regs.PC == 6

    def test_jr_backward_self_loop(self, cpu):
        write_program(cpu, [0x18, 0xFE], 0x0010)
        cpu.step()
        assert cpu.regs.PC == 0x0010

    def test_jr_backward(self, cpu):
        write_program(cpu, [0x18, 0xFC], 0x0010)
        cpu.step()
        assert cpu.regs.PC == 0x000E

    @pytest.mark.parametrize("opcode,flag,flag_val,taken", [
        (0xC2, FLAG_Z, 0, True), (0xC2, FLAG_Z, FLAG_Z, False),
        (0xCA, FLAG_Z, FLAG_Z, True), (0xCA, FLAG_Z, 0, False),
        (0xD2, FLAG_C, 0, True), (0xD2, FLAG_C, FLAG_C, False),
        (0xDA, FLAG_C, FLAG_C, True), (0xDA, FLAG_C, 0, False),
        (0xE2, FLAG_PV, 0, True), (0xE2, FLAG_PV, FLAG_PV, False),
        (0xEA, FLAG_PV, FLAG_PV, True), (0xEA, FLAG_PV, 0, False),
        (0xF2, FLAG_S, 0, True), (0xF2, FLAG_S, FLAG_S, False),
        (0xFA, FLAG_S, FLAG_S, True), (0xFA, FLAG_S, 0, False),
    ])
    def test_jp_cc_nn(self, cpu, opcode, flag, flag_val, taken):
        cpu.regs.F = flag_val
        write_program(cpu, [opcode, 0x00, 0x10])
        cpu.step()
        if taken:
            assert cpu.regs.PC == 0x1000
        else:
            assert cpu.regs.PC == 3

    @pytest.mark.parametrize("opcode,flag_val,taken", [
        (0x20, 0, True), (0x20, FLAG_Z, False),
        (0x28, FLAG_Z, True), (0x28, 0, False),
        (0x30, 0, True), (0x30, FLAG_C, False),
        (0x38, FLAG_C, True), (0x38, 0, False),
    ])
    def test_jr_cc_e(self, cpu, opcode, flag_val, taken):
        cpu.regs.F = flag_val
        write_program(cpu, [opcode, 0x04])
        cpu.step()
        if taken:
            assert cpu.regs.PC == 6
        else:
            assert cpu.regs.PC == 2

    def test_jp_ix(self, cpu):
        cpu.regs.IX = 0x4000
        write_program(cpu, [0xDD, 0xE9])
        cpu.step()
        assert cpu.regs.PC == 0x4000

    def test_jp_iy(self, cpu):
        cpu.regs.IY = 0x5000
        write_program(cpu, [0xFD, 0xE9])
        cpu.step()
        assert cpu.regs.PC == 0x5000
