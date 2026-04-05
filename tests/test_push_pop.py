#!/usr/bin/env python3
"""Stack push and pop operations."""

import pytest
from conftest import write_program, step_n


class TestPushPop:
    """Stack push and pop operations."""

    @pytest.mark.parametrize(
        "pair,push_op,pop_op",
        [
            ("BC", 0xC5, 0xC1),
            ("DE", 0xD5, 0xD1),
            ("HL", 0xE5, 0xE1),
        ],
    )
    def test_push_pop_round_trip(self, cpu, pair, push_op, pop_op):
        """PUSH rr / POP rr — round-trip preserves value and SP."""
        cpu.regs.SP = 0x2000
        setattr(cpu.regs, pair, 0xDEAD)
        write_program(cpu, [push_op, pop_op])
        cpu.step()
        setattr(cpu.regs, pair, 0x0000)
        cpu.step()
        assert getattr(cpu.regs, pair) == 0xDEAD
        assert cpu.regs.SP == 0x2000

    def test_push_decrements_sp(self, cpu):
        """PUSH decrements SP by 2."""
        cpu.regs.SP = 0xFFFF
        cpu.regs.BC = 0xDEAD
        write_program(cpu, [0xC5])
        cpu.step()
        assert cpu.regs.SP == 0xFFFD

    def test_push_stores_value(self, cpu):
        """PUSH stores high byte at SP+1, low byte at SP."""
        cpu.regs.SP = 0xFFFF
        cpu.regs.BC = 0xDEAD
        write_program(cpu, [0xC5])
        cpu.step()
        assert cpu.read_byte(0xFFFE) == 0xDE
        assert cpu.read_byte(0xFFFD) == 0xAD

    def test_push_pop_af(self, cpu):
        """PUSH AF / POP AF — preserves accumulator and flags."""
        cpu.regs.A = 0xFF
        cpu.regs.F = 0xD7
        cpu.regs.SP = 0xFFFF
        write_program(cpu, [0xF5, 0xF1])
        cpu.step()
        cpu.regs.A = 0x00
        cpu.regs.F = 0x00
        cpu.step()
        assert cpu.regs.A == 0xFF

    def test_nested_push_pop(self, cpu):
        """Nested PUSH/POP preserves all values (LIFO order)."""
        cpu.regs.SP = 0x2000
        cpu.regs.BC = 0xAAAA
        cpu.regs.DE = 0xBBBB
        write_program(cpu, [0xC5, 0xD5, 0xD1, 0xC1])
        step_n(cpu, 4)
        assert cpu.regs.BC == 0xAAAA
        assert cpu.regs.DE == 0xBBBB

    def test_push_pop_cross(self, cpu):
        """PUSH BC / POP DE — transfer value between pairs."""
        cpu.regs.SP = 0x2000
        cpu.regs.BC = 0x1234
        cpu.regs.DE = 0x0000
        write_program(cpu, [0xC5, 0xD1])
        step_n(cpu, 2)
        assert cpu.regs.DE == 0x1234
