#!/usr/bin/env python3
"""All RST (restart) instructions."""
import pytest
from conftest import write_program

class TestRSTComprehensive:
    @pytest.mark.parametrize("opcode,vector", [
        (0xC7, 0x00), (0xCF, 0x08), (0xD7, 0x10), (0xDF, 0x18),
        (0xE7, 0x20), (0xEF, 0x28), (0xF7, 0x30), (0xFF, 0x38),
    ])
    def test_rst_vectors(self, cpu, opcode, vector):
        cpu.registers.SP = 0x2000
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.registers.PC == vector
        assert cpu.registers.SP == 0x1FFE

    def test_rst_pushes_return_address(self, cpu):
        cpu.registers.SP = 0xFFFF
        write_program(cpu, [0xCD, 0x00, 0x00])
        cpu.step()
        write_program(cpu, [0xFF])
        cpu.step()
        assert cpu.read_byte(0xFFFE) == 0x00
        assert cpu.read_byte(0xFFFD) == 0x03
