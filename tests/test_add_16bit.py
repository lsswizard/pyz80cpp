#!/usr/bin/env python3
"""16-bit addition and subtraction tests."""

import pytest
from conftest import write_program, flag_set, flag_clear, FLAG_Z, FLAG_N, FLAG_C


class TestAdd16Bit:
    """16-bit addition and subtraction tests."""

    def test_add_hl_bc_overflow(self, cpu):
        """ADD HL,BC — overflow sets carry."""
        cpu.registers.HL = 0xFFFF
        cpu.registers.BC = 0x0001
        write_program(cpu, [0x09])
        cpu.step()
        assert cpu.registers.HL == 0x0000
        assert flag_set(cpu, FLAG_C)

    def test_add_hl_bc_no_overflow(self, cpu):
        """ADD HL,BC — no carry when no overflow."""
        cpu.registers.HL = 0x1000
        cpu.registers.BC = 0x0100
        write_program(cpu, [0x09])
        cpu.step()
        assert cpu.registers.HL == 0x1100
        assert flag_clear(cpu, FLAG_C)

    @pytest.mark.parametrize(
        "pair,opcode",
        [
            ("BC", 0x09),
            ("DE", 0x19),
            ("HL", 0x29),
            ("SP", 0x39),
        ],
    )
    def test_add_hl_rr(self, cpu, pair, opcode):
        """ADD HL,rr — all register pairs."""
        cpu.registers.HL = 0x1000
        if pair != "HL":
            setattr(cpu.registers, pair, 0x0100)
        write_program(cpu, [opcode])
        cpu.step()
        if pair == "HL":
            assert cpu.registers.HL == 0x2000
        else:
            assert cpu.registers.HL == 0x1100

    def test_add_hl_preserves_z(self, cpu):
        """ADD HL,rr — does not affect Z flag."""
        cpu.registers.F = FLAG_Z
        cpu.registers.HL = 0x0001
        cpu.registers.BC = 0x0001
        write_program(cpu, [0x09])
        cpu.step()
        assert flag_set(cpu, FLAG_Z)

    def test_add_hl_clears_n(self, cpu):
        """ADD HL,rr — always clears N."""
        cpu.registers.F = FLAG_N
        cpu.registers.HL = 0x0001
        cpu.registers.BC = 0x0001
        write_program(cpu, [0x09])
        cpu.step()
        assert flag_clear(cpu, FLAG_N)

    def test_adc_hl_bc_no_carry(self, cpu):
        """ADC HL,BC — without carry."""
        cpu.registers.HL = 0x0001
        cpu.registers.BC = 0x0002
        cpu.registers.F = 0
        write_program(cpu, [0xED, 0x4A])
        cpu.step()
        assert cpu.registers.HL == 0x0003

    def test_adc_hl_bc_with_carry(self, cpu):
        """ADC HL,BC — with carry adds one extra."""
        cpu.registers.HL = 0x0001
        cpu.registers.BC = 0x0002
        cpu.registers.F = FLAG_C
        write_program(cpu, [0xED, 0x4A])
        cpu.step()
        assert cpu.registers.HL == 0x0004

    def test_adc_hl_sets_z(self, cpu):
        """ADC HL,rr — sets Z when result is zero."""
        cpu.registers.HL = 0xFFFF
        cpu.registers.BC = 0x0001
        cpu.registers.F = 0
        write_program(cpu, [0xED, 0x4A])
        cpu.step()
        assert cpu.registers.HL == 0x0000
        assert flag_set(cpu, FLAG_Z)

    def test_sbc_hl_bc(self, cpu):
        """SBC HL,BC — subtraction sets N flag."""
        cpu.registers.HL = 0x0003
        cpu.registers.BC = 0x0001
        cpu.registers.F = 0
        write_program(cpu, [0xED, 0x42])
        cpu.step()
        assert cpu.registers.HL == 0x0002
        assert flag_set(cpu, FLAG_N)

    def test_sbc_hl_bc_with_carry(self, cpu):
        """SBC HL,BC — with carry subtracts one extra."""
        cpu.registers.HL = 0x0003
        cpu.registers.BC = 0x0001
        cpu.registers.F = FLAG_C
        write_program(cpu, [0xED, 0x42])
        cpu.step()
        assert cpu.registers.HL == 0x0001

    def test_sbc_hl_zero_result(self, cpu):
        """SBC HL,rr — Z set when result is zero."""
        cpu.registers.HL = 0x1000
        cpu.registers.BC = 0x1000
        cpu.registers.F = 0
        write_program(cpu, [0xED, 0x42])
        cpu.step()
        assert cpu.registers.HL == 0x0000
        assert flag_set(cpu, FLAG_Z)

    def test_sbc_hl_borrow(self, cpu):
        """SBC HL,rr — carry set on underflow."""
        cpu.registers.HL = 0x0000
        cpu.registers.BC = 0x0001
        cpu.registers.F = 0
        write_program(cpu, [0xED, 0x42])
        cpu.step()
        assert cpu.registers.HL == 0xFFFF
        assert flag_set(cpu, FLAG_C)
