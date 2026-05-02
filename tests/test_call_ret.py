#!/usr/bin/env python3
"""CALL and RET instruction tests."""
import pytest
from conftest import write_program, FLAG_Z, FLAG_C, FLAG_PV, FLAG_S

class TestCallRet:
    def test_call_nn(self, cpu):
        cpu.registers.SP = 0xFFFF
        write_program(cpu, [0xCD, 0x00, 0x10])
        cpu.step()
        assert cpu.registers.PC == 0x1000
        assert cpu.registers.SP == 0xFFFD
        assert cpu.read_byte(0xFFFE) == 0x00
        assert cpu.read_byte(0xFFFD) == 0x03

    def test_ret(self, cpu):
        cpu.registers.SP = 0xFFFF
        write_program(cpu, [0xCD, 0x00, 0x10])
        cpu.step()
        cpu.write_byte(0x1000, 0xC9)
        cpu.step()
        assert cpu.registers.PC == 0x0003
        assert cpu.registers.SP == 0xFFFF

    @pytest.mark.parametrize("opcode,flag_val,taken", [
        (0xC4, 0, True), (0xC4, FLAG_Z, False),
        (0xCC, FLAG_Z, True), (0xCC, 0, False),
        (0xD4, 0, True), (0xD4, FLAG_C, False),
        (0xDC, FLAG_C, True), (0xDC, 0, False),
        (0xE4, 0, True), (0xE4, FLAG_PV, False),
        (0xEC, FLAG_PV, True), (0xEC, 0, False),
        (0xF4, 0, True), (0xF4, FLAG_S, False),
        (0xFC, FLAG_S, True), (0xFC, 0, False),
    ])
    def test_call_cc_nn(self, cpu, opcode, flag_val, taken):
        cpu.registers.SP = 0xFFFF
        cpu.registers.F = flag_val
        write_program(cpu, [opcode, 0x00, 0x20])
        cpu.step()
        if taken:
            assert cpu.registers.PC == 0x2000
        else:
            assert cpu.registers.PC == 3

    @pytest.mark.parametrize("opcode,flag_val,taken", [
        (0xC0, 0, True), (0xC0, FLAG_Z, False),
        (0xC8, FLAG_Z, True), (0xC8, 0, False),
        (0xD0, 0, True), (0xD0, FLAG_C, False),
        (0xD8, FLAG_C, True), (0xD8, 0, False),
        (0xE0, 0, True), (0xE0, FLAG_PV, False),
        (0xE8, FLAG_PV, True), (0xE8, 0, False),
        (0xF0, 0, True), (0xF0, FLAG_S, False),
        (0xF8, FLAG_S, True), (0xF8, 0, False),
    ])
    def test_ret_cc(self, cpu, opcode, flag_val, taken):
        cpu.registers.SP = 0xFFFD
        cpu.write_byte(0xFFFD, 0x00)
        cpu.write_byte(0xFFFE, 0x30)
        cpu.registers.F = flag_val
        write_program(cpu, [opcode])
        cpu.step()
        if taken:
            assert cpu.registers.PC == 0x3000
        else:
            assert cpu.registers.PC == 1

    @pytest.mark.parametrize("opcode,target", [
        (0xC7, 0x00), (0xCF, 0x08), (0xD7, 0x10), (0xDF, 0x18),
        (0xE7, 0x20), (0xEF, 0x28), (0xF7, 0x30), (0xFF, 0x38),
    ])
    def test_rst(self, cpu, opcode, target):
        cpu.registers.SP = 0xFFFF
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.registers.PC == target
        assert cpu.registers.SP == 0xFFFD
