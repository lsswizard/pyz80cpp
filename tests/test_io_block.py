#!/usr/bin/env python3
"""Block I/O instruction tests."""

import pytest
from conftest import write_program, step_n


class TestIOBlock:
    def test_ini(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.regs.HL = 0x2000
        cpu.regs.B = 0x01
        cpu.set_on_input_callback(lambda port: 0xAB if (port & 0xFF) == 0x50 else 0x00)
        write_program(cpu, [0xED, 0xA2])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0xAB
        assert cpu.regs.B == 0x00
        assert cpu.regs.HL == 0x2001

    def test_ind(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.regs.HL = 0x2000
        cpu.regs.B = 0x01
        cpu.set_on_input_callback(lambda port: 0xAB if (port & 0xFF) == 0x50 else 0x00)
        write_program(cpu, [0xED, 0xAA])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0xAB
        assert cpu.regs.B == 0x00
        assert cpu.regs.HL == 0x1FFF

    def test_inir(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.regs.HL = 0x2000
        cpu.regs.B = 0x02
        cpu.set_on_input_callback(lambda port: 0xAB if (port & 0xFF) == 0x50 else 0x00)
        write_program(cpu, [0xED, 0xB2])
        step_n(cpu, 2)
        assert cpu.regs.B == 0x00
        assert cpu.read_byte(0x2000) == 0xAB
        assert cpu.read_byte(0x2001) == 0xAB

    def test_indr(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.regs.HL = 0x2001
        cpu.regs.B = 0x02
        cpu.set_on_input_callback(lambda port: 0xAB if (port & 0xFF) == 0x50 else 0x00)
        write_program(cpu, [0xED, 0xBA])
        step_n(cpu, 2)
        assert cpu.regs.B == 0x00
        assert cpu.read_byte(0x2000) == 0xAB
        assert cpu.read_byte(0x2001) == 0xAB

    def test_outi(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.regs.HL = 0x2000
        cpu.regs.B = 0x01
        cpu.write_byte(0x2000, 0xAB)
        output_values = {}
        cpu.set_on_output_callback(lambda port, val: output_values.update({port: val}))
        write_program(cpu, [0xED, 0xA3])
        cpu.step()
        assert output_values.get(cpu.regs.BC) == 0xAB
        assert cpu.regs.B == 0x00
        assert cpu.regs.HL == 0x2001

    def test_outd(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.regs.HL = 0x2000
        cpu.regs.B = 0x01
        cpu.write_byte(0x2000, 0xAB)
        output_values = {}
        cpu.set_on_output_callback(lambda port, val: output_values.update({port: val}))
        write_program(cpu, [0xED, 0xAB])
        cpu.step()
        assert output_values.get(cpu.regs.BC) == 0xAB
        assert cpu.regs.B == 0x00
        assert cpu.regs.HL == 0x1FFF

    def test_otir(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.regs.HL = 0x2000
        cpu.regs.B = 0x02
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        output_values = {}
        cpu.set_on_output_callback(
            lambda port, val: output_values.setdefault(port, []).append(val)
        )
        write_program(cpu, [0xED, 0xB3])
        step_n(cpu, 2)
        assert cpu.regs.B == 0x00

    def test_otdr(self, cpu):
        cpu.regs.BC = 0x0250
        cpu.regs.HL = 0x2001
        cpu.regs.B = 0x02
        cpu.write_byte(0x2000, 0xAA)
        cpu.write_byte(0x2001, 0xBB)
        output_values = {}
        cpu.set_on_output_callback(
            lambda port, val: output_values.setdefault(port, []).append(val)
        )
        write_program(cpu, [0xED, 0xBB])
        step_n(cpu, 2)
        assert cpu.regs.B == 0x00
