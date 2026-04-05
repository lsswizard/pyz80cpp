#!/usr/bin/env python3
"""Edge case tests."""

import pytest
from conftest import (
    write_program,
    step_n,
    flag_set,
    flag_clear,
    FLAG_Z,
    FLAG_C,
    FLAG_PV,
    FLAG_S,
    FLAG_H,
    FLAG_F5,
    FLAG_F3,
)


class TestEdgeCases:
    def test_r_register_increments(self, cpu):
        r_initial = cpu.regs.R
        write_program(cpu, [0x00, 0x00, 0x00])
        cpu.step()
        assert cpu.regs.R != r_initial

    def test_r_register_bit7_preserved(self, cpu):
        cpu.regs.R = 0x80
        write_program(cpu, [0x00])
        cpu.step()
        assert cpu.regs.R & 0x80

    def test_sp_wrap_on_push(self, cpu):
        cpu.regs.SP = 0x0000
        cpu.regs.BC = 0x1234
        write_program(cpu, [0xC5])
        cpu.step()
        assert cpu.regs.SP == 0xFFFE

    def test_sp_wrap_on_pop(self, cpu):
        cpu.regs.SP = 0xFFFF
        cpu.write_byte(0xFFFF, 0x34)
        cpu.write_byte(0x0000, 0x12)
        write_program(cpu, [0xC1])
        cpu.step()
        assert cpu.regs.SP == 0x0001

    def test_pc_wrap_on_jp(self, cpu):
        write_program(cpu, [0xC3, 0xFF, 0xFF])
        cpu.step()
        assert cpu.regs.PC == 0xFFFF

    def test_jr_wrap_forward(self, cpu):
        write_program(cpu, [0x18, 0x01], 0xFFFF)
        cpu.step()
        assert cpu.regs.PC == 0x0002

    def test_jr_wrap_backward(self, cpu):
        write_program(cpu, [0x18, 0xFF], 0xFFFE)
        cpu.step()
        assert cpu.regs.PC == 0xFFFF

    def test_call_wrap(self, cpu):
        cpu.regs.SP = 0x0000
        write_program(cpu, [0xCD, 0x00, 0x10])
        cpu.step()
        assert cpu.regs.SP == 0xFFFE

    def test_exx_preserves_main(self, cpu):
        cpu.regs.BC = 0x1111
        cpu.regs.DE = 0x2222
        cpu.regs.HL = 0x3333
        write_program(cpu, [0xD9, 0xD9])
        step_n(cpu, 2)
        assert cpu.regs.BC == 0x1111
        assert cpu.regs.DE == 0x2222
        assert cpu.regs.HL == 0x3333

    def test_shadow_registers_independent(self, cpu):
        cpu.regs.BC = 0x1111
        write_program(cpu, [0xD9])
        cpu.step()
        cpu.regs.BC = 0x2222
        write_program(cpu, [0xD9])
        cpu.step()
        assert cpu.regs.BC == 0x1111

    def test_undefined_ed_is_nop(self, cpu):
        cpu.regs.A = 0x12
        write_program(cpu, [0xED, 0x00])
        cpu.step()
        assert cpu.regs.PC == 1
        assert cpu.regs.A == 0x12

    def test_dd_cb_fallthrough(self, cpu):
        cpu.regs.IX = 0x1000
        write_program(cpu, [0xDD, 0xCB, 0x00, 0xFF])
        cpu.step()

    def test_multiple_prefixes(self, cpu):
        write_program(cpu, [0xDD, 0xDD, 0x00])
        cpu.step()
        assert cpu.regs.PC == 3

    def test_ix_negative_displacement(self, cpu):
        cpu.regs.IX = 0x1010
        cpu.write_byte(0x1000, 0xAB)
        write_program(cpu, [0xDD, 0x7E, 0xF0])
        cpu.step()
        assert cpu.regs.A == 0xAB

    def test_iy_negative_displacement(self, cpu):
        cpu.regs.IY = 0x1010
        cpu.write_byte(0x1000, 0xAB)
        write_program(cpu, [0xFD, 0x7E, 0xF0])
        cpu.step()
        assert cpu.regs.A == 0xAB

    def test_ix_displacement_wrap(self, cpu):
        cpu.regs.IX = 0x0000
        cpu.write_byte(0xFFFF, 0xAB)
        write_program(cpu, [0xDD, 0x7E, 0xFF])
        cpu.step()
        assert cpu.regs.A == 0xAB

    def test_ix_displacement_wrap_forward(self, cpu):
        cpu.regs.IX = 0xFFFF
        cpu.write_byte(0x0000, 0xAB)  # Note: separate from program
        write_program(cpu, [0xDD, 0x46, 0x01], 0x0100)
        cpu.step()
        assert cpu.regs.B == 0xAB
