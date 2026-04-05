#!/usr/bin/env python3
"""I/O instruction tests."""

import pytest
from conftest import write_program


class TestIO:
    def test_in_a_n(self, cpu):
        cpu.regs.A = 0x50
        cpu.set_on_input_callback(lambda port: 0xAB if (port & 0xFF) == 0x50 else 0x00)
        write_program(cpu, [0xDB, 0x50])
        cpu.step()
        assert cpu.regs.A == 0xAB

    def test_out_n_a(self, cpu):
        cpu.regs.A = 0xAB
        output_values = {}
        cpu.set_on_output_callback(lambda port, val: output_values.update({port: val}))
        write_program(cpu, [0xD3, 0x50])
        cpu.step()
        assert output_values.get(0xAB50) == 0xAB

    def test_in_b_c(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.set_on_input_callback(lambda port: 0xAB if (port & 0xFF) == 0x50 else 0x00)
        write_program(cpu, [0xED, 0x40])
        cpu.step()
        assert cpu.regs.B == 0xAB

    def test_out_c_b(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.regs.B = 0xAB
        output_values = {}
        cpu.set_on_output_callback(lambda port, val: output_values.update({port: val}))
        write_program(cpu, [0xED, 0x41])
        cpu.step()
        assert output_values.get(cpu.regs.BC) == 0xAB

    def test_in_f_c(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.set_on_input_callback(lambda port: 0xAB if (port & 0xFF) == 0x50 else 0x00)
        write_program(cpu, [0xED, 0x70])
        cpu.step()
        assert cpu.regs.F & 0x80
