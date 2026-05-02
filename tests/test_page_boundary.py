#!/usr/bin/env python3
"""Memory operations that cross 64KB boundaries."""
from conftest import write_program

class TestPageBoundary:
    def test_jp_at_page_boundary(self, cpu):
        cpu.write_byte(0x0100, 0xC3)
        cpu.write_byte(0x0101, 0x00)
        cpu.write_byte(0x0102, 0x20)
        write_program(cpu, [0xC3, 0x00, 0x01])
        cpu.step()
        assert cpu.registers.PC == 0x0100

    def test_call_at_page_boundary(self, cpu):
        cpu.registers.SP = 0xFFFC
        cpu.write_byte(0x0100, 0xC9)
        write_program(cpu, [0xCD, 0x00, 0x01])
        cpu.step()
        assert cpu.read_byte(0xFFFA) == 0x03
        assert cpu.read_byte(0xFFFB) == 0x00

    def test_inc_hl_wraps(self, cpu):
        cpu.registers.HL = 0xFFFF
        write_program(cpu, [0x23])
        cpu.step()
        assert cpu.registers.HL == 0x0000
