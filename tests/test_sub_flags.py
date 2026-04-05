#!/usr/bin/env python3
"""SUB A,n, SBC A,n, and CP n flag verification."""

import pytest
from conftest import write_program, _sub_flags, flag_set, flag_clear, FLAG_S, FLAG_Z, FLAG_H, FLAG_PV, FLAG_C, FLAG_N


class TestSubFlags:
    """SUB A,n, SBC A,n, and CP n flag verification."""

    @pytest.mark.parametrize(
        "a,b",
        [
            (0x00, 0x00),
            (0x00, 0x01),
            (0x7F, 0x01),
            (0x80, 0x01),
            (0x00, 0x80),
            (0xFF, 0xFF),
            (0x10, 0x01),
            (0x01, 0x01),
            (0x80, 0x80),
            (0x3E, 0x3E),
        ],
    )
    def test_sub_a_n_flags(self, cpu, a, b):
        """SUB A,n — verify result and all affected flags."""
        write_program(cpu, [0x3E, a, 0xD6, b])
        cpu.step()
        cpu.step()
        expected_flags = _sub_flags(a, b)
        mask = FLAG_S | FLAG_Z | FLAG_H | FLAG_PV | FLAG_C | FLAG_N
        assert cpu.regs.A == (a - b) & 0xFF
        assert (cpu.regs.F & mask) == (expected_flags & mask)

    def test_sbc_with_carry(self, cpu):
        """SBC A,n — carry is subtracted too."""
        cpu.regs.A = 0x10
        cpu.regs.F = FLAG_C
        write_program(cpu, [0xDE, 0x01])
        cpu.step()
        assert cpu.regs.A == 0x0E

    def test_sbc_no_carry(self, cpu):
        """SBC A,n — without carry behaves like SUB."""
        cpu.regs.A = 0x10
        cpu.regs.F = 0
        write_program(cpu, [0xDE, 0x01])
        cpu.step()
        assert cpu.regs.A == 0x0F

    def test_cp_does_not_modify_a(self, cpu):
        """CP n — A is unchanged after comparison."""
        cpu.regs.A = 0x10
        write_program(cpu, [0xFE, 0x10])
        cpu.step()
        assert cpu.regs.A == 0x10

    def test_cp_equal_sets_z(self, cpu):
        """CP n — Z flag set when A == n."""
        cpu.regs.A = 0x10
        write_program(cpu, [0xFE, 0x10])
        cpu.step()
        assert flag_set(cpu, FLAG_Z)

    def test_cp_less_sets_carry(self, cpu):
        """CP n — carry set when A < n."""
        cpu.regs.A = 0x05
        write_program(cpu, [0xFE, 0x10])
        cpu.step()
        assert flag_set(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_N)

    def test_cp_greater_no_carry(self, cpu):
        """CP n — no carry when A > n."""
        cpu.regs.A = 0x20
        write_program(cpu, [0xFE, 0x10])
        cpu.step()
        assert flag_clear(cpu, FLAG_C)
        assert flag_clear(cpu, FLAG_Z)

    @pytest.mark.parametrize(
        "reg,opcode",
        [
            ("B", 0x90),
            ("C", 0x91),
            ("D", 0x92),
            ("E", 0x93),
            ("H", 0x94),
            ("L", 0x95),
        ],
    )
    def test_sub_a_r(self, cpu, reg, opcode):
        """SUB A,r — subtract register from A."""
        cpu.regs.A = 0x20
        setattr(cpu.regs, reg, 0x10)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.regs.A == 0x10

    def test_sub_a_hl_indirect(self, cpu):
        """SUB A,(HL) — subtract memory byte from A."""
        cpu.regs.A = 0x30
        cpu.regs.HL = 0x2000
        cpu.write_byte(0x2000, 0x10)
        write_program(cpu, [0x96])
        cpu.step()
        assert cpu.regs.A == 0x20
