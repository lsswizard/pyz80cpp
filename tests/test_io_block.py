#!/usr/bin/env python3
"""Block I/O instruction tests."""

import pytest
from conftest import write_program, step_n


class TestIOBlock:
    def test_ini(self, cpu):
        cpu.registers.BC = 0x0250
        cpu.registers.HL = 0x2000
        cpu.registers.B = 0x01
        # Set up I/O port using bus.out()
        cpu.bus.out_(0x50, 0xAB)
        write_program(cpu, [0xED, 0xA2])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0xAB
        assert cpu.registers.B == 0x00
        assert cpu.registers.HL == 0x2001

    def test_ind(self, cpu):
        cpu.registers.BC = 0x0250
        cpu.registers.HL = 0x2000
        cpu.registers.B = 0x01
        cpu.bus.out_(0x50, 0xAB)
        write_program(cpu, [0xED, 0xAA])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0xAB
        assert cpu.registers.B == 0x00
        assert cpu.registers.HL == 0x1FFF

    def test_inir(self, cpu):
        cpu.registers.BC = 0x0250
        cpu.registers.HL = 0x2000
        cpu.registers.B = 0x02
        cpu.bus.out_(0x50, 0xAB)
        write_program(cpu, [0xED, 0xB2])
        step_n(cpu, 2)
        assert cpu.registers.B == 0x00
        assert cpu.read_byte(0x2000) == 0xAB
        assert cpu.read_byte(0x2001) == 0xAB

    def test_indr(self, cpu):
        cpu.registers.BC = 0x0250
        cpu.registers.HL = 0x2001
        cpu.registers.B = 0x02
        cpu.bus.out_(0x50, 0xAB)
        write_program(cpu, [0xED, 0xBA])
        step_n(cpu, 2)
        assert cpu.registers.B == 0x00
        assert cpu.read_byte(0x2000) == 0xAB
        assert cpu.read_byte(0x2001) == 0xAB

    def test_outi(self, cpu):
        cpu.registers.BC = 0x0250
        cpu.registers.HL = 0x2000
        cpu.registers.B = 0x01
        cpu.write_byte(0x2000, 0xAB)
        write_program(cpu, [0xED, 0xA3])
        cpu.step()
        # Verify output via bus.in_()
        assert cpu.bus.in_(0x50) == 0xAB
        assert cpu.registers.B == 0x00
        assert cpu.registers.HL == 0x2001

    def test_outd(self, cpu):
        cpu.registers.BC = 0x0250
        cpu.registers.HL = 0x2000
        cpu.registers.B = 0x01
        cpu.write_byte(0x2000, 0xAB)
        write_program(cpu, [0xED, 0xAB])
        cpu.step()
        assert cpu.bus.in_(0x50) == 0xAB
        assert cpu.registers.B == 0x00
        assert cpu.registers.HL == 0x1FFF

    def test_otir(self, cpu):
        # OTIR: Output (HL), increment HL, decrement B, repeat until B=0
        # Each iteration: 16 T-states, last iteration: 21 T-states
        cpu.registers.BC = 0x0250
        cpu.registers.HL = 0x2000
        cpu.registers.B = 0x02
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        write_program(cpu, [0xED, 0xB3])
        # OTIR takes 2 steps in emulator: first iteration (21 cycles), second (16 cycles)
        step_n(cpu, 2)
        assert cpu.registers.B == 0x00
        # Verify outputs - last output was at address 0x2001 (0xBB)
        assert cpu.bus.in_(0x50) == 0xBB

    def test_otdr(self, cpu):
        # OTDR: Output (HL), decrement HL, decrement B, repeat until B=0
        # Note: HL is DECREMENTED first, then output
        cpu.registers.BC = 0x0250
        cpu.registers.HL = 0x2001
        cpu.registers.B = 0x02
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        write_program(cpu, [0xED, 0xBB])
        # OTDR takes 2 steps:
        # - Step 1: output (0x2001)=0xBB, HL->0x2000, B->1 (21 cycles)
        # - Step 2: output (0x2000)=0xAA, HL->0x1FFF, B->0 (16 cycles)
        step_n(cpu, 2)
        assert cpu.registers.B == 0x00
        # Last output was at 0x2000 (0xAA) because HL is decremented first
        assert cpu.bus.in_(0x50) == 0xAA
