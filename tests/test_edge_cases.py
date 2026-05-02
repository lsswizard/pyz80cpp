#!/usr/bin/env python3
"""Edge case tests."""

from conftest import (
    write_program,
    step_n,
)


class TestEdgeCases:
    def test_r_register_increments(self, cpu):
        r_initial = cpu.registers.R
        write_program(cpu, [0x00, 0x00, 0x00])
        cpu.step()
        assert cpu.registers.R != r_initial

    def test_r_register_bit7_preserved(self, cpu):
        cpu.registers.R = 0x80
        write_program(cpu, [0x00])
        cpu.step()
        assert cpu.registers.R & 0x80

    def test_sp_wrap_on_push(self, cpu):
        cpu.registers.SP = 0x0000
        cpu.registers.BC = 0x1234
        write_program(cpu, [0xC5])
        cpu.step()
        assert cpu.registers.SP == 0xFFFE

    def test_sp_wrap_on_pop(self, cpu):
        cpu.registers.SP = 0xFFFF
        cpu.write_byte(0xFFFF, 0x34)
        cpu.write_byte(0x0000, 0x12)
        write_program(cpu, [0xC1])
        cpu.step()
        assert cpu.registers.SP == 0x0001

    def test_pc_wrap_on_jp(self, cpu):
        write_program(cpu, [0xC3, 0xFF, 0xFF])
        cpu.step()
        assert cpu.registers.PC == 0xFFFF

    def test_jr_wrap_forward(self, cpu):
        # Write program at end of memory and test PC wrapping
        # JR from 0xFFFE with displacement +2 should go to 0x0002
        cpu.registers.PC = 0xFFFE
        cpu.write_byte(0xFFFE, 0x18)  # JR
        cpu.write_byte(0xFFFF, 0x02)  # +2 displacement
        cpu.step()
        assert cpu.registers.PC == 0x0002

    def test_jr_wrap_backward(self, cpu):
        write_program(cpu, [0x18, 0xFF], 0xFFFE)
        cpu.step()
        assert cpu.registers.PC == 0xFFFF

    def test_call_wrap(self, cpu):
        cpu.registers.SP = 0x0000
        write_program(cpu, [0xCD, 0x00, 0x10])
        cpu.step()
        assert cpu.registers.SP == 0xFFFE

    def test_exx_preserves_main(self, cpu):
        cpu.registers.BC = 0x1111
        cpu.registers.DE = 0x2222
        cpu.registers.HL = 0x3333
        write_program(cpu, [0xD9, 0xD9])
        step_n(cpu, 2)
        assert cpu.registers.BC == 0x1111
        assert cpu.registers.DE == 0x2222
        assert cpu.registers.HL == 0x3333

    def test_shadow_registers_independent(self, cpu):
        cpu.registers.BC = 0x1111
        write_program(cpu, [0xD9])
        cpu.step()
        cpu.registers.BC = 0x2222
        write_program(cpu, [0xD9])
        cpu.step()
        assert cpu.registers.BC == 0x1111

    def test_undefined_ed_is_nop(self, cpu):
        # ED 00 is an undefined opcode - acts like NOP but takes 2 bytes
        # According to Z80 docs: "ED opcodes in the range 00-3F and 80-FF
        # do nothing but taking up 8 T states and incrementing the R register by 2"
        cpu.registers.A = 0x12
        write_program(cpu, [0xED, 0x00])
        cpu.step()
        assert cpu.registers.PC == 2  # ED prefix + opcode = 2 bytes
        assert cpu.registers.A == 0x12

    def test_dd_cb_fallthrough(self, cpu):
        cpu.registers.IX = 0x1000
        write_program(cpu, [0xDD, 0xCB, 0x00, 0xFF])
        cpu.step()

    def test_multiple_prefixes(self, cpu):
        # DD DD 00 - first DD prefix is applied, second DD is the opcode
        # Second DD is not in DD table, falls through to main where it's a NOP
        # Total: 2 bytes consumed (DD prefix + DD/NOP)
        write_program(cpu, [0xDD, 0xDD, 0x00])
        cpu.step()
        assert cpu.registers.PC == 2

    def test_ix_negative_displacement(self, cpu):
        cpu.registers.IX = 0x1010
        cpu.write_byte(0x1000, 0xAB)
        write_program(cpu, [0xDD, 0x7E, 0xF0])
        cpu.step()
        assert cpu.registers.A == 0xAB

    def test_iy_negative_displacement(self, cpu):
        cpu.registers.IY = 0x1010
        cpu.write_byte(0x1000, 0xAB)
        write_program(cpu, [0xFD, 0x7E, 0xF0])
        cpu.step()
        assert cpu.registers.A == 0xAB

    def test_ix_displacement_wrap(self, cpu):
        cpu.registers.IX = 0x0000
        cpu.write_byte(0xFFFF, 0xAB)
        write_program(cpu, [0xDD, 0x7E, 0xFF])
        cpu.step()
        assert cpu.registers.A == 0xAB

    def test_ix_displacement_wrap_forward(self, cpu):
        cpu.registers.IX = 0xFFFF
        cpu.write_byte(0x0000, 0xAB)  # Note: separate from program
        write_program(cpu, [0xDD, 0x46, 0x01], 0x0100)
        cpu.step()
        assert cpu.registers.B == 0xAB
