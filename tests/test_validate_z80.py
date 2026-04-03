#!/usr/bin/env python3
"""
Z80 CPU Core Validation — Machine-Agnostic API
Tests the C++ Z80 implementation against expected Z80 behavior.
Uses the raw CPython API: cpu.read_byte/write_byte, cpu.X register access, I/O callbacks.
"""

import sys
import os
import pytest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

from core import (
    Z80CPU,
    FLAG_S,
    FLAG_Z,
    FLAG_H,
    FLAG_PV,
    FLAG_N,
    FLAG_C,
    FLAG_F5,
    FLAG_F3,
)


# ============================================================
# I/O Callback Bus (for testing port I/O instructions)
# ============================================================


class IOBus:
    """Simple in-memory I/O state for testing IN/OUT instructions."""

    def __init__(self):
        self.ports = {}

    def input_callback(self, port):
        return self.ports.get(port & 0xFF, 0xFF)

    def output_callback(self, port, value):
        self.ports[port & 0xFF] = value


# ============================================================
# Fixtures
# ============================================================


class _TestCPU:
    """Thin wrapper holding CPU + IO bus reference."""

    def __init__(self):
        self._cpu = Z80CPU()
        self._io = IOBus()
        self._cpu.set_on_input_callback(self._io.input_callback)
        self._cpu.set_on_output_callback(self._io.output_callback)

    # Delegate all Z80CPU attributes/methods
    def __getattr__(self, name):
        return getattr(self._cpu, name)

    def __setattr__(self, name, value):
        if name.startswith("_"):
            object.__setattr__(self, name, value)
        else:
            setattr(self._cpu, name, value)


@pytest.fixture
def cpu():
    return _TestCPU()


# ============================================================
# Helpers
# ============================================================


def write_program(c, program_bytes, addr=0):
    """Write a sequence of bytes into CPU memory and set PC."""
    for i, b in enumerate(program_bytes):
        c.write_byte(addr + i, b)
    c.PC = addr


def flag_set(c, flag):
    """Check if a flag is set."""
    return bool(c.F & flag)


def flag_clear(c, flag):
    """Check if a flag is clear."""
    return not (c.F & flag)


def run_cb_instruction(c, cb_op):
    """Execute a CB-prefixed instruction."""
    c.write_byte(0, 0xCB)
    c.write_byte(1, cb_op)
    c.PC = 0
    return c.step()


def step_n(c, n):
    """Step the CPU n times, returning total cycles."""
    total = 0
    for _ in range(n):
        total += c.step()
    return total


def parity(val):
    """Compute even parity of a byte."""
    return bin(val).count("1") % 2 == 0


# ============================================================
# 1. 8-Bit Load Instructions
# ============================================================


class TestLoad8Bit:
    @pytest.mark.parametrize(
        "reg,opcode",
        [
            ("A", 0x3E),
            ("B", 0x06),
            ("C", 0x0E),
            ("D", 0x16),
            ("E", 0x1E),
            ("H", 0x26),
            ("L", 0x2E),
        ],
    )
    def test_ld_r_n(self, cpu, reg, opcode):
        write_program(cpu, [opcode, 0xAB])
        cpu.step()
        assert getattr(cpu, reg) == 0xAB

    def test_ld_a_bc_indirect(self, cpu):
        cpu.BC = 0x1234
        cpu.write_byte(0x1234, 0x77)
        write_program(cpu, [0x0A])
        cpu.step()
        assert cpu.A == 0x77

    def test_ld_a_de_indirect(self, cpu):
        cpu.DE = 0x1234
        cpu.write_byte(0x1234, 0x55)
        write_program(cpu, [0x1A])
        cpu.step()
        assert cpu.A == 0x55

    def test_ld_hl_indirect_n(self, cpu):
        cpu.HL = 0x2000
        write_program(cpu, [0x36, 0xCC])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0xCC

    def test_ld_a_nn_indirect(self, cpu):
        cpu.write_byte(0x3000, 0x99)
        write_program(cpu, [0x3A, 0x00, 0x30])
        cpu.step()
        assert cpu.A == 0x99

    def test_ld_nn_indirect_a(self, cpu):
        cpu.A = 0xBB
        write_program(cpu, [0x32, 0x00, 0x30])
        cpu.step()
        assert cpu.read_byte(0x3000) == 0xBB

    @pytest.mark.parametrize(
        "opcode,src,dst",
        [
            (0x78, "B", "A"),
            (0x79, "C", "A"),
            (0x7A, "D", "A"),
            (0x7B, "E", "A"),
            (0x7C, "H", "A"),
            (0x7D, "L", "A"),
            (0x47, "A", "B"),
            (0x48, "B", "C"),
            (0x50, "B", "D"),
            (0x58, "B", "E"),
            (0x60, "B", "H"),
            (0x68, "B", "L"),
        ],
    )
    def test_ld_r_r(self, cpu, opcode, src, dst):
        setattr(cpu, src, 0x42)
        write_program(cpu, [opcode])
        cpu.step()
        assert getattr(cpu, dst) == 0x42

    def test_ld_bc_indirect_a(self, cpu):
        cpu.A = 0xEE
        cpu.BC = 0x3000
        write_program(cpu, [0x02])
        cpu.step()
        assert cpu.read_byte(0x3000) == 0xEE

    def test_ld_de_indirect_a(self, cpu):
        cpu.A = 0xDD
        cpu.DE = 0x3000
        write_program(cpu, [0x12])
        cpu.step()
        assert cpu.read_byte(0x3000) == 0xDD

    def test_ld_hl_indirect_r(self, cpu):
        cpu.HL = 0x2000
        cpu.B = 0x42
        write_program(cpu, [0x70])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x42

    def test_ld_r_hl_indirect(self, cpu):
        cpu.HL = 0x2000
        cpu.write_byte(0x2000, 0x99)
        write_program(cpu, [0x46])
        cpu.step()
        assert cpu.B == 0x99


# ============================================================
# 2. 16-Bit Load Instructions
# ============================================================


class TestLoad16Bit:
    @pytest.mark.parametrize(
        "pair,opcode", [("BC", 0x01), ("DE", 0x11), ("HL", 0x21), ("SP", 0x31)]
    )
    def test_ld_rr_nn(self, cpu, pair, opcode):
        write_program(cpu, [opcode, 0xCD, 0xAB])
        cpu.step()
        assert getattr(cpu, pair) == 0xABCD

    def test_ld_hl_nn_indirect(self, cpu):
        cpu.write_byte(0x4000, 0x78)
        cpu.write_byte(0x4001, 0x56)
        write_program(cpu, [0x2A, 0x00, 0x40])
        cpu.step()
        assert cpu.HL == 0x5678

    def test_ld_nn_indirect_hl(self, cpu):
        cpu.HL = 0x1234
        write_program(cpu, [0x22, 0x00, 0x40])
        cpu.step()
        assert cpu.read_byte(0x4000) == 0x34
        assert cpu.read_byte(0x4001) == 0x12

    def test_ld_sp_hl(self, cpu):
        cpu.HL = 0x1234
        write_program(cpu, [0xF9])
        cpu.step()
        assert cpu.SP == 0x1234


# ============================================================
# 3. PUSH / POP
# ============================================================


class TestPushPop:
    @pytest.mark.parametrize(
        "pair,push_op,pop_op",
        [("BC", 0xC5, 0xC1), ("DE", 0xD5, 0xD1), ("HL", 0xE5, 0xE1)],
    )
    def test_push_pop_round_trip(self, cpu, pair, push_op, pop_op):
        cpu.SP = 0x2000
        setattr(cpu, pair, 0xDEAD)
        write_program(cpu, [push_op, pop_op])
        cpu.step()
        setattr(cpu, pair, 0x0000)
        cpu.step()
        assert getattr(cpu, pair) == 0xDEAD
        assert cpu.SP == 0x2000

    def test_push_decrements_sp(self, cpu):
        cpu.SP = 0xFFFF
        cpu.BC = 0xDEAD
        write_program(cpu, [0xC5])
        cpu.step()
        assert cpu.SP == 0xFFFD

    def test_push_stores_value(self, cpu):
        cpu.SP = 0xFFFF
        cpu.BC = 0xDEAD
        write_program(cpu, [0xC5])
        cpu.step()
        assert cpu.read_byte(0xFFFE) == 0xDE
        assert cpu.read_byte(0xFFFD) == 0xAD

    def test_push_pop_af(self, cpu):
        cpu.A = 0xFF
        cpu.F = 0xD7
        cpu.SP = 0xFFFF
        write_program(cpu, [0xF5, 0xF1])
        cpu.step()
        cpu.A = 0x00
        cpu.F = 0x00
        cpu.step()
        assert cpu.A == 0xFF

    def test_nested_push_pop(self, cpu):
        cpu.SP = 0x2000
        cpu.BC = 0xAAAA
        cpu.DE = 0xBBBB
        write_program(cpu, [0xC5, 0xD5, 0xD1, 0xC1])
        step_n(cpu, 4)
        assert cpu.BC == 0xAAAA
        assert cpu.DE == 0xBBBB

    def test_push_pop_cross(self, cpu):
        cpu.SP = 0x2000
        cpu.BC = 0x1234
        cpu.DE = 0x0000
        write_program(cpu, [0xC5, 0xD1])
        step_n(cpu, 2)
        assert cpu.DE == 0x1234


# ============================================================
# 4. ADD / ADC Flags
# ============================================================


class TestAddFlags:
    @pytest.mark.parametrize(
        "a,b",
        [
            (0x00, 0x00),
            (0x01, 0x02),
            (0x7F, 0x01),
            (0xFF, 0x01),
            (0x80, 0x80),
            (0x0F, 0x01),
            (0xF0, 0x10),
            (0x55, 0xAA),
            (0x01, 0xFF),
            (0x40, 0x40),
            (0xFE, 0x01),
        ],
    )
    def test_add_a_n_flags(self, cpu, a, b):
        write_program(cpu, [0x3E, a, 0xC6, b])
        cpu.step()
        cpu.step()
        assert cpu.A == (a + b) & 0xFF
        # Verify flags are computed correctly by checking specific properties
        result = (a + b) & 0xFF
        assert flag_set(cpu, FLAG_H) == (((a & 0x0F) + (b & 0x0F)) > 0x0F)
        assert flag_set(cpu, FLAG_C) == ((a + b) > 0xFF)
        assert flag_set(cpu, FLAG_Z) == (result == 0)
        assert flag_clear(cpu, FLAG_N)

    def test_adc_with_carry(self, cpu):
        cpu.A = 0x0F
        cpu.F = FLAG_C
        write_program(cpu, [0xCE, 0x01])
        cpu.step()
        assert cpu.A == 0x11
        assert flag_set(cpu, FLAG_H)

    def test_adc_no_carry(self, cpu):
        cpu.A = 0x10
        cpu.F = 0
        write_program(cpu, [0xCE, 0x05])
        cpu.step()
        assert cpu.A == 0x15

    def test_adc_carry_causes_overflow(self, cpu):
        cpu.A = 0x7F
        cpu.F = FLAG_C
        write_program(cpu, [0xCE, 0x00])
        cpu.step()
        assert cpu.A == 0x80
        assert flag_set(cpu, FLAG_PV)
        assert flag_set(cpu, FLAG_S)

    @pytest.mark.parametrize(
        "reg,opcode",
        [("B", 0x80), ("C", 0x81), ("D", 0x82), ("E", 0x83), ("H", 0x84), ("L", 0x85)],
    )
    def test_add_a_r(self, cpu, reg, opcode):
        cpu.A = 0x10
        setattr(cpu, reg, 0x05)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.A == 0x15

    def test_add_a_hl_indirect(self, cpu):
        cpu.A = 0x10
        cpu.HL = 0x2000
        cpu.write_byte(0x2000, 0x20)
        write_program(cpu, [0x86])
        cpu.step()
        assert cpu.A == 0x30

    def test_add_a_a(self, cpu):
        cpu.A = 0x40
        write_program(cpu, [0x87])
        cpu.step()
        assert cpu.A == 0x80
        assert flag_set(cpu, FLAG_PV)


# ============================================================
# 5. ADD HL,rr / ADC HL,rr / SBC HL,rr
# ============================================================


class TestAdd16Bit:
    def test_add_hl_bc_overflow(self, cpu):
        cpu.HL = 0xFFFF
        cpu.BC = 0x0001
        write_program(cpu, [0x09])
        cpu.step()
        assert cpu.HL == 0x0000
        assert flag_set(cpu, FLAG_C)

    def test_add_hl_bc_no_overflow(self, cpu):
        cpu.HL = 0x1000
        cpu.BC = 0x0100
        write_program(cpu, [0x09])
        cpu.step()
        assert cpu.HL == 0x1100
        assert flag_clear(cpu, FLAG_C)

    @pytest.mark.parametrize(
        "pair,opcode", [("BC", 0x09), ("DE", 0x19), ("HL", 0x29), ("SP", 0x39)]
    )
    def test_add_hl_rr(self, cpu, pair, opcode):
        cpu.HL = 0x1000
        if pair != "HL":
            setattr(cpu, pair, 0x0100)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.HL == (0x2000 if pair == "HL" else 0x1100)

    def test_add_hl_preserves_z(self, cpu):
        cpu.F = FLAG_Z
        cpu.HL = 0x0001
        cpu.BC = 0x0001
        write_program(cpu, [0x09])
        cpu.step()
        assert flag_set(cpu, FLAG_Z)

    def test_add_hl_clears_n(self, cpu):
        cpu.F = FLAG_N
        cpu.HL = 0x0001
        cpu.BC = 0x0001
        write_program(cpu, [0x09])
        cpu.step()
        assert flag_clear(cpu, FLAG_N)

    def test_adc_hl_bc_no_carry(self, cpu):
        cpu.HL = 0x0001
        cpu.BC = 0x0002
        cpu.F = 0
        write_program(cpu, [0xED, 0x4A])
        cpu.step()
        assert cpu.HL == 0x0003

    def test_adc_hl_bc_with_carry(self, cpu):
        cpu.HL = 0x0001
        cpu.BC = 0x0002
        cpu.F = FLAG_C
        write_program(cpu, [0xED, 0x4A])
        cpu.step()
        assert cpu.HL == 0x0004

    def test_adc_hl_sets_z(self, cpu):
        cpu.HL = 0xFFFF
        cpu.BC = 0x0001
        cpu.F = 0
        write_program(cpu, [0xED, 0x4A])
        cpu.step()
        assert cpu.HL == 0x0000
        assert flag_set(cpu, FLAG_Z)

    def test_sbc_hl_bc(self, cpu):
        cpu.HL = 0x0003
        cpu.BC = 0x0001
        cpu.F = 0
        write_program(cpu, [0xED, 0x42])
        cpu.step()
        assert cpu.HL == 0x0002
        assert flag_set(cpu, FLAG_N)

    def test_sbc_hl_bc_with_carry(self, cpu):
        cpu.HL = 0x0003
        cpu.BC = 0x0001
        cpu.F = FLAG_C
        write_program(cpu, [0xED, 0x42])
        cpu.step()
        assert cpu.HL == 0x0001

    def test_sbc_hl_zero_result(self, cpu):
        cpu.HL = 0x1000
        cpu.BC = 0x1000
        cpu.F = 0
        write_program(cpu, [0xED, 0x42])
        cpu.step()
        assert cpu.HL == 0x0000
        assert flag_set(cpu, FLAG_Z)

    def test_sbc_hl_borrow(self, cpu):
        cpu.HL = 0x0000
        cpu.BC = 0x0001
        cpu.F = 0
        write_program(cpu, [0xED, 0x42])
        cpu.step()
        assert cpu.HL == 0xFFFF
        assert flag_set(cpu, FLAG_C)


# ============================================================
# 6. SUB / SBC / CP Flags
# ============================================================


class TestSubFlags:
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
        write_program(cpu, [0x3E, a, 0xD6, b])
        cpu.step()
        cpu.step()
        assert cpu.A == (a - b) & 0xFF
        assert flag_set(cpu, FLAG_H) == ((a & 0x0F) < (b & 0x0F))
        assert flag_set(cpu, FLAG_C) == (a < b)
        assert flag_clear(cpu, FLAG_N) == False  # SUB always sets N
        assert flag_set(cpu, FLAG_N)

    def test_sbc_with_carry(self, cpu):
        cpu.A = 0x10
        cpu.F = FLAG_C
        write_program(cpu, [0xDE, 0x01])
        cpu.step()
        assert cpu.A == 0x0E

    def test_sbc_no_carry(self, cpu):
        cpu.A = 0x10
        cpu.F = 0
        write_program(cpu, [0xDE, 0x01])
        cpu.step()
        assert cpu.A == 0x0F

    def test_cp_does_not_modify_a(self, cpu):
        cpu.A = 0x10
        write_program(cpu, [0xFE, 0x10])
        cpu.step()
        assert cpu.A == 0x10

    def test_cp_equal_sets_z(self, cpu):
        cpu.A = 0x10
        write_program(cpu, [0xFE, 0x10])
        cpu.step()
        assert flag_set(cpu, FLAG_Z)

    def test_cp_less_sets_carry(self, cpu):
        cpu.A = 0x05
        write_program(cpu, [0xFE, 0x10])
        cpu.step()
        assert flag_set(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_N)

    def test_cp_greater_no_carry(self, cpu):
        cpu.A = 0x20
        write_program(cpu, [0xFE, 0x10])
        cpu.step()
        assert flag_clear(cpu, FLAG_C)
        assert flag_clear(cpu, FLAG_Z)

    @pytest.mark.parametrize(
        "reg,opcode",
        [("B", 0x90), ("C", 0x91), ("D", 0x92), ("E", 0x93), ("H", 0x94), ("L", 0x95)],
    )
    def test_sub_a_r(self, cpu, reg, opcode):
        cpu.A = 0x20
        setattr(cpu, reg, 0x10)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.A == 0x10

    def test_sub_a_hl_indirect(self, cpu):
        cpu.A = 0x30
        cpu.HL = 0x2000
        cpu.write_byte(0x2000, 0x10)
        write_program(cpu, [0x96])
        cpu.step()
        assert cpu.A == 0x20


# ============================================================
# 7. INC / DEC (8-bit)
# ============================================================


class TestIncDec8Bit:
    @pytest.mark.parametrize(
        "val,exp_result,exp_s,exp_z,exp_h,exp_pv",
        [
            (0x00, 0x01, False, False, False, False),
            (0x7F, 0x80, True, False, True, True),
            (0xFF, 0x00, False, True, True, False),
            (0x0F, 0x10, False, False, True, False),
            (0xFE, 0xFF, True, False, False, False),
        ],
    )
    def test_inc_a(self, cpu, val, exp_result, exp_s, exp_z, exp_h, exp_pv):
        cpu.A = val
        cpu.F = FLAG_C
        write_program(cpu, [0x3C])
        cpu.step()
        assert cpu.A == exp_result
        assert flag_set(cpu, FLAG_S) == exp_s
        assert flag_set(cpu, FLAG_Z) == exp_z
        assert flag_set(cpu, FLAG_H) == exp_h
        assert flag_set(cpu, FLAG_PV) == exp_pv
        assert flag_set(cpu, FLAG_C)
        assert flag_clear(cpu, FLAG_N)

    @pytest.mark.parametrize(
        "val,exp_result,exp_s,exp_z,exp_h,exp_pv",
        [
            (0x01, 0x00, False, True, False, False),
            (0x80, 0x7F, False, False, True, True),
            (0x00, 0xFF, True, False, True, False),
            (0x10, 0x0F, False, False, True, False),
            (0x02, 0x01, False, False, False, False),
        ],
    )
    def test_dec_a(self, cpu, val, exp_result, exp_s, exp_z, exp_h, exp_pv):
        cpu.A = val
        cpu.F = FLAG_C
        write_program(cpu, [0x3D])
        cpu.step()
        assert cpu.A == exp_result
        assert flag_set(cpu, FLAG_S) == exp_s
        assert flag_set(cpu, FLAG_Z) == exp_z
        assert flag_set(cpu, FLAG_H) == exp_h
        assert flag_set(cpu, FLAG_PV) == exp_pv
        assert flag_set(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_N)

    @pytest.mark.parametrize(
        "reg,inc_op,dec_op",
        [
            ("B", 0x04, 0x05),
            ("C", 0x0C, 0x0D),
            ("D", 0x14, 0x15),
            ("E", 0x1C, 0x1D),
            ("H", 0x24, 0x25),
            ("L", 0x2C, 0x2D),
        ],
    )
    def test_inc_dec_registers(self, cpu, reg, inc_op, dec_op):
        setattr(cpu, reg, 0x10)
        write_program(cpu, [inc_op])
        cpu.step()
        assert getattr(cpu, reg) == 0x11

        cpu.reset()
        setattr(cpu, reg, 0x10)
        write_program(cpu, [dec_op])
        cpu.step()
        assert getattr(cpu, reg) == 0x0F

    def test_inc_hl_indirect(self, cpu):
        cpu.HL = 0x2000
        cpu.write_byte(0x2000, 0x0F)
        write_program(cpu, [0x34])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x10
        assert flag_set(cpu, FLAG_H)

    def test_dec_hl_indirect(self, cpu):
        cpu.HL = 0x2000
        cpu.write_byte(0x2000, 0x10)
        write_program(cpu, [0x35])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x0F
        assert flag_set(cpu, FLAG_H)


# ============================================================
# 8. INC / DEC (16-bit)
# ============================================================


class TestIncDec16Bit:
    def test_inc_bc_wrap(self, cpu):
        cpu.BC = 0xFFFF
        write_program(cpu, [0x03])
        cpu.step()
        assert cpu.BC == 0x0000

    def test_dec_de_wrap(self, cpu):
        cpu.DE = 0x0000
        write_program(cpu, [0x1B])
        cpu.step()
        assert cpu.DE == 0xFFFF

    @pytest.mark.parametrize(
        "pair,inc_op,dec_op",
        [
            ("BC", 0x03, 0x0B),
            ("DE", 0x13, 0x1B),
            ("HL", 0x23, 0x2B),
            ("SP", 0x33, 0x3B),
        ],
    )
    def test_inc_dec_16_all_pairs(self, cpu, pair, inc_op, dec_op):
        setattr(cpu, pair, 0x1000)
        write_program(cpu, [inc_op])
        cpu.step()
        assert getattr(cpu, pair) == 0x1001

        cpu.reset()
        setattr(cpu, pair, 0x1000)
        write_program(cpu, [dec_op])
        cpu.step()
        assert getattr(cpu, pair) == 0x0FFF

    def test_inc_16_preserves_flags(self, cpu):
        cpu.F = FLAG_Z | FLAG_C
        cpu.BC = 0x0001
        write_program(cpu, [0x03])
        cpu.step()
        assert cpu.F == (FLAG_Z | FLAG_C)


# ============================================================
# 9. Logical Operations: AND / OR / XOR
# ============================================================


class TestLogical:
    @pytest.mark.parametrize(
        "a,n,expected",
        [
            (0xFF, 0x0F, 0x0F),
            (0x00, 0xFF, 0x00),
            (0xAA, 0x55, 0x00),
            (0xFF, 0xFF, 0xFF),
            (0x12, 0x34, 0x10),
        ],
    )
    def test_and_n(self, cpu, a, n, expected):
        cpu.A = a
        write_program(cpu, [0xE6, n])
        cpu.step()
        assert cpu.A == expected
        assert flag_set(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)
        assert flag_clear(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_Z) == (expected == 0)
        assert flag_set(cpu, FLAG_S) == bool(expected & 0x80)
        assert flag_set(cpu, FLAG_PV) == parity(expected)

    @pytest.mark.parametrize(
        "a,n,expected",
        [
            (0x0F, 0xF0, 0xFF),
            (0x00, 0x00, 0x00),
            (0xAA, 0x55, 0xFF),
            (0x80, 0x00, 0x80),
        ],
    )
    def test_or_n(self, cpu, a, n, expected):
        cpu.A = a
        write_program(cpu, [0xF6, n])
        cpu.step()
        assert cpu.A == expected
        assert flag_clear(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)
        assert flag_clear(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_Z) == (expected == 0)
        assert flag_set(cpu, FLAG_S) == bool(expected & 0x80)

    @pytest.mark.parametrize(
        "a,n,expected",
        [
            (0xFF, 0xFF, 0x00),
            (0xAA, 0x55, 0xFF),
            (0x00, 0x80, 0x80),
            (0x12, 0x12, 0x00),
            (0x00, 0x00, 0x00),
        ],
    )
    def test_xor_n(self, cpu, a, n, expected):
        cpu.A = a
        write_program(cpu, [0xEE, n])
        cpu.step()
        assert cpu.A == expected
        assert flag_clear(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)
        assert flag_clear(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_Z) == (expected == 0)

    @pytest.mark.parametrize(
        "reg,opcode",
        [
            ("B", 0xA0),
            ("C", 0xA1),
            ("D", 0xA2),
            ("E", 0xA3),
            ("H", 0xA4),
            ("L", 0xA5),
        ],
    )
    def test_and_r(self, cpu, reg, opcode):
        cpu.A = 0xFF
        setattr(cpu, reg, 0x0F)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.A == 0x0F

    @pytest.mark.parametrize(
        "reg,opcode",
        [
            ("B", 0xB0),
            ("C", 0xB1),
            ("D", 0xB2),
            ("E", 0xB3),
            ("H", 0xB4),
            ("L", 0xB5),
        ],
    )
    def test_or_r(self, cpu, reg, opcode):
        cpu.A = 0x0F
        setattr(cpu, reg, 0xF0)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.A == 0xFF

    @pytest.mark.parametrize(
        "reg,opcode",
        [
            ("B", 0xA8),
            ("C", 0xA9),
            ("D", 0xAA),
            ("E", 0xAB),
            ("H", 0xAC),
            ("L", 0xAD),
        ],
    )
    def test_xor_r(self, cpu, reg, opcode):
        cpu.A = 0xFF
        setattr(cpu, reg, 0xFF)
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.A == 0x00
        assert flag_set(cpu, FLAG_Z)

    def test_and_hl_indirect(self, cpu):
        cpu.A = 0xFF
        cpu.HL = 0x2000
        cpu.write_byte(0x2000, 0x0F)
        write_program(cpu, [0xA6])
        cpu.step()
        assert cpu.A == 0x0F

    def test_or_hl_indirect(self, cpu):
        cpu.A = 0x0F
        cpu.HL = 0x2000
        cpu.write_byte(0x2000, 0xF0)
        write_program(cpu, [0xB6])
        cpu.step()
        assert cpu.A == 0xFF

    def test_xor_hl_indirect(self, cpu):
        cpu.A = 0xAA
        cpu.HL = 0x2000
        cpu.write_byte(0x2000, 0xAA)
        write_program(cpu, [0xAE])
        cpu.step()
        assert cpu.A == 0x00


# ============================================================
# 10. CPL / NEG / DAA / SCF / CCF
# ============================================================


class TestAccFlagOps:
    def test_cpl(self, cpu):
        cpu.A = 0x3C
        write_program(cpu, [0x2F])
        cpu.step()
        assert cpu.A == 0xC3
        assert flag_set(cpu, FLAG_H)
        assert flag_set(cpu, FLAG_N)

    def test_neg_positive(self, cpu):
        cpu.A = 0x01
        write_program(cpu, [0xED, 0x44])
        cpu.step()
        assert cpu.A == 0xFF
        assert flag_set(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_N)

    def test_neg_zero(self, cpu):
        cpu.A = 0x00
        write_program(cpu, [0xED, 0x44])
        cpu.step()
        assert cpu.A == 0x00
        assert flag_set(cpu, FLAG_Z)
        assert flag_clear(cpu, FLAG_C)

    def test_neg_0x80_overflow(self, cpu):
        cpu.A = 0x80
        write_program(cpu, [0xED, 0x44])
        cpu.step()
        assert cpu.A == 0x80
        assert flag_set(cpu, FLAG_PV)

    def test_scf(self, cpu):
        cpu.F = 0x00
        write_program(cpu, [0x37])
        cpu.step()
        assert flag_set(cpu, FLAG_C)
        assert flag_clear(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)

    def test_ccf_invert_set(self, cpu):
        cpu.F = FLAG_C
        write_program(cpu, [0x3F])
        cpu.step()
        assert flag_clear(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_H)

    def test_ccf_invert_clear(self, cpu):
        cpu.F = 0x00
        write_program(cpu, [0x3F])
        cpu.step()
        assert flag_set(cpu, FLAG_C)

    def test_daa_add_09_01(self, cpu):
        cpu.A = 0x09
        write_program(cpu, [0xC6, 0x01, 0x27])
        step_n(cpu, 2)
        assert cpu.A == 0x10

    def test_daa_add_09_09(self, cpu):
        cpu.A = 0x09
        write_program(cpu, [0xC6, 0x09, 0x27])
        step_n(cpu, 2)
        assert cpu.A == 0x18

    def test_daa_add_99_01(self, cpu):
        cpu.A = 0x99
        write_program(cpu, [0xC6, 0x01, 0x27])
        step_n(cpu, 2)
        assert cpu.A == 0x00
        assert flag_set(cpu, FLAG_C)

    def test_daa_sub(self, cpu):
        cpu.A = 0x10
        write_program(cpu, [0xD6, 0x01, 0x27])
        step_n(cpu, 2)
        assert cpu.A == 0x09


# ============================================================
# 11. Exchange Instructions
# ============================================================


class TestExchange:
    def test_ex_de_hl(self, cpu):
        cpu.DE = 0xAAAA
        cpu.HL = 0xBBBB
        write_program(cpu, [0xEB])
        cpu.step()
        assert cpu.DE == 0xBBBB
        assert cpu.HL == 0xAAAA

    def test_exx_round_trip(self, cpu):
        cpu.BC = 0x1111
        cpu.DE = 0x2222
        cpu.HL = 0x3333
        write_program(cpu, [0xD9])
        cpu.step()
        write_program(cpu, [0xD9])
        cpu.step()
        assert cpu.BC == 0x1111
        assert cpu.DE == 0x2222
        assert cpu.HL == 0x3333

    def test_ex_sp_hl(self, cpu):
        cpu.SP = 0x1000
        cpu.HL = 0x1234
        cpu.write_byte(0x1000, 0x78)
        cpu.write_byte(0x1001, 0x56)
        write_program(cpu, [0xE3])
        cpu.step()
        assert cpu.HL == 0x5678
        assert cpu.read_byte(0x1000) == 0x34
        assert cpu.read_byte(0x1001) == 0x12

    def test_ex_sp_ix(self, cpu):
        cpu.SP = 0x1000
        cpu.IX = 0xABCD
        cpu.write_byte(0x1000, 0x34)
        cpu.write_byte(0x1001, 0x12)
        write_program(cpu, [0xDD, 0xE3])
        cpu.step()
        assert cpu.IX == 0x1234
        assert cpu.read_byte(0x1000) == 0xCD
        assert cpu.read_byte(0x1001) == 0xAB


# ============================================================
# 12. Jumps
# ============================================================


class TestJumps:
    def test_jp_nn(self, cpu):
        write_program(cpu, [0xC3, 0x00, 0x20])
        cpu.step()
        assert cpu.PC == 0x2000

    def test_jr_forward(self, cpu):
        write_program(cpu, [0x18, 0x04])
        cpu.step()
        assert cpu.PC == 6

    def test_jr_backward_self_loop(self, cpu):
        write_program(cpu, [0x18, 0xFE], 0x0010)
        cpu.step()
        assert cpu.PC == 0x0010

    @pytest.mark.parametrize(
        "opcode,flag,flag_val,taken",
        [
            (0xC2, FLAG_Z, 0, True),
            (0xC2, FLAG_Z, FLAG_Z, False),
            (0xCA, FLAG_Z, FLAG_Z, True),
            (0xCA, FLAG_Z, 0, False),
            (0xD2, FLAG_C, 0, True),
            (0xD2, FLAG_C, FLAG_C, False),
            (0xDA, FLAG_C, FLAG_C, True),
            (0xDA, FLAG_C, 0, False),
        ],
    )
    def test_jp_cc_nn(self, cpu, opcode, flag, flag_val, taken):
        cpu.F = flag_val
        write_program(cpu, [opcode, 0x00, 0x10])
        cpu.step()
        assert cpu.PC == (0x1000 if taken else 3)

    @pytest.mark.parametrize(
        "opcode,flag_val,taken",
        [
            (0x20, 0, True),
            (0x20, FLAG_Z, False),
            (0x28, FLAG_Z, True),
            (0x28, 0, False),
            (0x30, 0, True),
            (0x30, FLAG_C, False),
            (0x38, FLAG_C, True),
            (0x38, 0, False),
        ],
    )
    def test_jr_cc_e(self, cpu, opcode, flag_val, taken):
        cpu.F = flag_val
        write_program(cpu, [opcode, 0x04])
        cpu.step()
        assert cpu.PC == (6 if taken else 2)


# ============================================================
# 13. DJNZ
# ============================================================


class TestDJNZ:
    def test_djnz_branch(self, cpu):
        cpu.B = 2
        write_program(cpu, [0x10, 0x00])
        cpu.step()
        assert cpu.B == 1
        assert cpu.PC == 2

    def test_djnz_no_branch(self, cpu):
        cpu.B = 1
        write_program(cpu, [0x10, 0x04])
        cpu.step()
        assert cpu.B == 0
        assert cpu.PC == 2

    def test_djnz_count_to_zero(self, cpu):
        cpu.B = 3
        write_program(cpu, [0x10, 0xFE])
        cpu.step()
        assert cpu.B == 2
        cpu.step()
        assert cpu.B == 1
        cpu.step()
        assert cpu.B == 0
        assert cpu.PC == 2


# ============================================================
# 14. CALL / RET
# ============================================================


class TestCallRet:
    def test_call_nn(self, cpu):
        cpu.SP = 0xFFFF
        write_program(cpu, [0xCD, 0x00, 0x10])
        cpu.step()
        assert cpu.PC == 0x1000
        assert cpu.SP == 0xFFFD
        assert cpu.read_byte(0xFFFE) == 0x00
        assert cpu.read_byte(0xFFFD) == 0x03

    def test_ret(self, cpu):
        cpu.SP = 0xFFFF
        write_program(cpu, [0xCD, 0x00, 0x10])
        cpu.step()
        cpu.write_byte(0x1000, 0xC9)
        cpu.step()
        assert cpu.PC == 0x0003
        assert cpu.SP == 0xFFFF

    @pytest.mark.parametrize(
        "opcode,flag_val,taken",
        [
            (0xC4, 0, True),
            (0xC4, FLAG_Z, False),
            (0xCC, FLAG_Z, True),
            (0xCC, 0, False),
            (0xD4, 0, True),
            (0xD4, FLAG_C, False),
            (0xDC, FLAG_C, True),
            (0xDC, 0, False),
        ],
    )
    def test_call_cc_nn(self, cpu, opcode, flag_val, taken):
        cpu.SP = 0xFFFF
        cpu.F = flag_val
        write_program(cpu, [opcode, 0x00, 0x20])
        cpu.step()
        assert cpu.PC == (0x2000 if taken else 3)

    @pytest.mark.parametrize(
        "opcode,flag_val,taken",
        [
            (0xC0, 0, True),
            (0xC0, FLAG_Z, False),
            (0xC8, FLAG_Z, True),
            (0xC8, 0, False),
            (0xD0, 0, True),
            (0xD0, FLAG_C, False),
            (0xD8, FLAG_C, True),
            (0xD8, 0, False),
        ],
    )
    def test_ret_cc(self, cpu, opcode, flag_val, taken):
        cpu.SP = 0xFFFD
        cpu.write_byte(0xFFFD, 0x00)
        cpu.write_byte(0xFFFE, 0x30)
        cpu.F = flag_val
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.PC == (0x3000 if taken else 1)

    @pytest.mark.parametrize(
        "opcode,target",
        [
            (0xC7, 0x00),
            (0xCF, 0x08),
            (0xD7, 0x10),
            (0xDF, 0x18),
            (0xE7, 0x20),
            (0xEF, 0x28),
            (0xF7, 0x30),
            (0xFF, 0x38),
        ],
    )
    def test_rst(self, cpu, opcode, target):
        cpu.SP = 0xFFFF
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.PC == target
        assert cpu.SP == 0xFFFD


# ============================================================
# 15. Rotate Instructions (Accumulator)
# ============================================================


class TestRotateAcc:
    def test_rlca_bit7_set(self, cpu):
        cpu.A = 0x88
        write_program(cpu, [0x07])
        cpu.step()
        assert cpu.A == 0x11
        assert flag_set(cpu, FLAG_C)

    def test_rrca_bit0_set(self, cpu):
        cpu.A = 0x11
        write_program(cpu, [0x0F])
        cpu.step()
        assert cpu.A == 0x88
        assert flag_set(cpu, FLAG_C)

    def test_rla_with_carry(self, cpu):
        cpu.A = 0x41
        cpu.F = FLAG_C
        write_program(cpu, [0x17])
        cpu.step()
        assert cpu.A == 0x83
        assert flag_clear(cpu, FLAG_C)

    def test_rra_with_carry(self, cpu):
        cpu.A = 0x00
        cpu.F = FLAG_C
        write_program(cpu, [0x1F])
        cpu.step()
        assert cpu.A == 0x80
        assert flag_clear(cpu, FLAG_C)

    def test_rlca_clears_h_n(self, cpu):
        cpu.A = 0x01
        cpu.F = FLAG_H | FLAG_N
        write_program(cpu, [0x07])
        cpu.step()
        assert flag_clear(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)


# ============================================================
# 16. CB-Prefix Rotate/Shift Instructions
# ============================================================


class TestCBRotateShift:
    def test_rlc_a(self, cpu):
        cpu.A = 0x80
        run_cb_instruction(cpu, 0x07)
        assert cpu.A == 0x01
        assert flag_set(cpu, FLAG_C)

    def test_sla_a(self, cpu):
        cpu.A = 0x80
        run_cb_instruction(cpu, 0x27)
        assert cpu.A == 0x00
        assert flag_set(cpu, FLAG_C)
        assert flag_set(cpu, FLAG_Z)

    def test_sra_a(self, cpu):
        cpu.A = 0x81
        run_cb_instruction(cpu, 0x2F)
        assert cpu.A == 0xC0
        assert flag_set(cpu, FLAG_C)

    def test_srl_a(self, cpu):
        cpu.A = 0x81
        run_cb_instruction(cpu, 0x3F)
        assert cpu.A == 0x40
        assert flag_set(cpu, FLAG_C)

    @pytest.mark.parametrize(
        "reg_idx,reg_name",
        [
            (0, "B"),
            (1, "C"),
            (2, "D"),
            (3, "E"),
            (4, "H"),
            (5, "L"),
            (7, "A"),
        ],
    )
    def test_rlc_all_registers(self, cpu, reg_idx, reg_name):
        setattr(cpu, reg_name, 0x80)
        run_cb_instruction(cpu, 0x00 + reg_idx)
        assert getattr(cpu, reg_name) == 0x01

    @pytest.mark.parametrize(
        "reg_idx,reg_name",
        [
            (0, "B"),
            (1, "C"),
            (2, "D"),
            (3, "E"),
            (4, "H"),
            (5, "L"),
            (7, "A"),
        ],
    )
    def test_srl_all_registers(self, cpu, reg_idx, reg_name):
        setattr(cpu, reg_name, 0x02)
        run_cb_instruction(cpu, 0x38 + reg_idx)
        assert getattr(cpu, reg_name) == 0x01


# ============================================================
# 17. BIT / SET / RES
# ============================================================


class TestBitSetRes:
    @pytest.mark.parametrize("bit", range(8))
    def test_bit_set_in_a(self, cpu, bit):
        cpu.A = 1 << bit
        run_cb_instruction(cpu, 0x40 + (bit * 8) + 7)
        assert flag_clear(cpu, FLAG_Z)
        assert flag_set(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)

    @pytest.mark.parametrize("bit", range(8))
    def test_bit_clear_in_a(self, cpu, bit):
        cpu.A = 0xFF ^ (1 << bit)
        run_cb_instruction(cpu, 0x40 + (bit * 8) + 7)
        assert flag_set(cpu, FLAG_Z)

    @pytest.mark.parametrize("bit", range(8))
    def test_set_bit_a(self, cpu, bit):
        cpu.A = 0x00
        run_cb_instruction(cpu, 0xC0 + (bit * 8) + 7)
        assert cpu.A == (1 << bit)

    @pytest.mark.parametrize("bit", range(8))
    def test_res_bit_a(self, cpu, bit):
        cpu.A = 0xFF
        run_cb_instruction(cpu, 0x80 + (bit * 8) + 7)
        assert cpu.A == (0xFF ^ (1 << bit))

    def test_set_0_hl_indirect(self, cpu):
        cpu.HL = 0x2000
        cpu.write_byte(0x2000, 0xF0)
        run_cb_instruction(cpu, 0xC6)
        assert cpu.read_byte(0x2000) == 0xF1

    def test_res_4_hl_indirect(self, cpu):
        cpu.HL = 0x2000
        cpu.write_byte(0x2000, 0xFF)
        run_cb_instruction(cpu, 0xA6)
        assert cpu.read_byte(0x2000) == 0xEF


# ============================================================
# 18. Block Instructions
# ============================================================


class TestBlockInstructions:
    def test_ldi(self, cpu):
        cpu.HL = 0x1000
        cpu.DE = 0x2000
        cpu.BC = 0x0003
        cpu.write_byte(0x1000, 0x42)
        write_program(cpu, [0xED, 0xA0])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x42
        assert cpu.HL == 0x1001
        assert cpu.DE == 0x2001
        assert cpu.BC == 0x0002
        assert flag_set(cpu, FLAG_PV)

    def test_ldi_bc_zero(self, cpu):
        cpu.HL = 0x1000
        cpu.DE = 0x2000
        cpu.BC = 0x0001
        cpu.write_byte(0x1000, 0x42)
        write_program(cpu, [0xED, 0xA0])
        cpu.step()
        assert cpu.BC == 0x0000
        assert flag_clear(cpu, FLAG_PV)

    def test_ldir(self, cpu):
        cpu.write_byte(0x1000, 0xAA)
        cpu.write_byte(0x1001, 0xBB)
        cpu.write_byte(0x1002, 0xCC)
        cpu.HL = 0x1000
        cpu.DE = 0x2000
        cpu.BC = 0x0003
        write_program(cpu, [0xED, 0xB0])
        step_n(cpu, 3)
        assert cpu.read_byte(0x2000) == 0xAA
        assert cpu.read_byte(0x2001) == 0xBB
        assert cpu.read_byte(0x2002) == 0xCC
        assert cpu.BC == 0x0000

    def test_cpi(self, cpu):
        cpu.A = 0x42
        cpu.HL = 0x1000
        cpu.BC = 0x0002
        cpu.write_byte(0x1000, 0x42)
        write_program(cpu, [0xED, 0xA1])
        cpu.step()
        assert flag_set(cpu, FLAG_Z)
        assert cpu.HL == 0x1001
        assert cpu.BC == 0x0001


# ============================================================
# 19. I/O Block Instructions
# ============================================================


class TestIOBlock:
    def test_ini(self, cpu):
        cpu.B = 0x02
        cpu.C = 0x10
        cpu.HL = 0x2000
        cpu._io.ports[0x10] = 0xAB
        write_program(cpu, [0xED, 0xA2])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0xAB
        assert cpu.HL == 0x2001
        assert cpu.B == 0x01

    def test_ind(self, cpu):
        cpu.B = 0x02
        cpu.C = 0x10
        cpu.HL = 0x2005
        cpu._io.ports[0x10] = 0xCD
        write_program(cpu, [0xED, 0xAA])
        cpu.step()
        assert cpu.read_byte(0x2005) == 0xCD
        assert cpu.HL == 0x2004
        assert cpu.B == 0x01

    def test_outi(self, cpu):
        cpu.B = 0x02
        cpu.C = 0x10
        cpu.HL = 0x2000
        cpu.write_byte(0x2000, 0xEF)
        write_program(cpu, [0xED, 0xA3])
        cpu.step()
        assert cpu._io.ports.get(0x10, None) == 0xEF
        assert cpu.HL == 0x2001
        assert cpu.B == 0x01

    def test_outd(self, cpu):
        cpu.B = 0x02
        cpu.C = 0x10
        cpu.HL = 0x2005
        cpu.write_byte(0x2005, 0x77)
        write_program(cpu, [0xED, 0xAB])
        cpu.step()
        assert cpu._io.ports.get(0x10, None) == 0x77
        assert cpu.HL == 0x2004
        assert cpu.B == 0x01


# ============================================================
# 20. IN / OUT Instructions
# ============================================================


class TestIO:
    def test_in_a_n(self, cpu):
        cpu._io.ports[0x42] = 0xAB
        write_program(cpu, [0xDB, 0x42])
        cpu.step()
        assert cpu.A == 0xAB

    def test_out_n_a(self, cpu):
        cpu.A = 0xCD
        write_program(cpu, [0xD3, 0x42])
        cpu.step()
        assert cpu._io.ports.get(0x42, None) == 0xCD

    def test_in_r_c(self, cpu):
        cpu.C = 0x10
        cpu._io.ports[0x10] = 0x80
        write_program(cpu, [0xED, 0x78])
        cpu.step()
        assert cpu.A == 0x80
        assert flag_set(cpu, FLAG_S)
        assert flag_clear(cpu, FLAG_Z)

    def test_out_c_r(self, cpu):
        cpu.C = 0x10
        cpu.B = 0x42
        write_program(cpu, [0xED, 0x41])
        cpu.step()
        assert cpu._io.ports.get(0x10, None) == 0x42


# ============================================================
# 21. Instruction Timing
# ============================================================


class TestTiming:
    @pytest.mark.parametrize(
        "program,mnemonic,expected_cycles",
        [
            (b"\x00", "NOP", 4),
            (b"\x3e\x00", "LD A,n", 7),
            (b"\x01\x00\x00", "LD BC,nn", 10),
            (b"\x80", "ADD A,B", 4),
            (b"\x86", "ADD A,(HL)", 7),
            (b"\xc6\x00", "ADD A,n", 7),
            (b"\x09", "ADD HL,BC", 11),
            (b"\xc3\x00\x00", "JP nn", 10),
            (b"\xe9", "JP (HL)", 4),
            (b"\x18\x00", "JR e", 12),
            (b"\xcd\x00\x00", "CALL nn", 17),
            (b"\xc9", "RET", 10),
            (b"\x76", "HALT", 4),
            (b"\x3c", "INC A", 4),
            (b"\x34", "INC (HL)", 11),
            (b"\x03", "INC BC", 6),
            (b"\xcb\x07", "RLC A", 8),
            (b"\xcb\x06", "RLC (HL)", 15),
            (b"\xed\xa0", "LDI", 16),
            (b"\xff", "RST 38H", 11),
            (b"\xc5", "PUSH BC", 11),
            (b"\xc1", "POP BC", 10),
            (b"\xd6\x00", "SUB n", 7),
            (b"\x2f", "CPL", 4),
            (b"\x37", "SCF", 4),
            (b"\x3f", "CCF", 4),
            (b"\x07", "RLCA", 4),
            (b"\x0f", "RRCA", 4),
            (b"\x17", "RLA", 4),
            (b"\x1f", "RRA", 4),
            (b"\xeb", "EX DE,HL", 4),
            (b"\x08", "EX AF,AF'", 4),
            (b"\xd9", "EXX", 4),
            (b"\xe3", "EX (SP),HL", 19),
            (b"\xf9", "LD SP,HL", 6),
            (b"\xf3", "DI", 4),
            (b"\xfb", "EI", 4),
            (b"\x27", "DAA", 4),
            (b"\xfe\x00", "CP n", 7),
            (b"\xed\x44", "NEG", 8),
        ],
    )
    def test_instruction_timing(self, cpu, program, mnemonic, expected_cycles):
        for i, b in enumerate(program):
            cpu.write_byte(i, b)
        cpu.PC = 0
        cycles = cpu.step()
        assert cycles == expected_cycles, (
            f"{mnemonic}: expected {expected_cycles}, got {cycles}"
        )

    def test_djnz_no_branch_timing(self, cpu):
        cpu.B = 1
        write_program(cpu, [0x10, 0x00])
        assert cpu.step() == 8

    def test_djnz_branch_timing(self, cpu):
        cpu.B = 2
        write_program(cpu, [0x10, 0x00])
        assert cpu.step() == 13

    def test_jp_cc_always_10_cycles(self, cpu):
        cpu.F = 0
        write_program(cpu, [0xC2, 0x00, 0x00])
        assert cpu.step() == 10
        cpu.reset()
        cpu.F = FLAG_Z
        write_program(cpu, [0xC2, 0x00, 0x00])
        assert cpu.step() == 10

    def test_call_cc_taken_timing(self, cpu):
        cpu.SP = 0xFFFF
        cpu.F = 0
        write_program(cpu, [0xC4, 0x00, 0x00])
        assert cpu.step() == 17

    def test_call_cc_not_taken_timing(self, cpu):
        cpu.SP = 0xFFFF
        cpu.F = FLAG_Z
        write_program(cpu, [0xC4, 0x00, 0x00])
        assert cpu.step() == 10

    def test_ret_cc_taken_timing(self, cpu):
        cpu.SP = 0xFFFD
        cpu.write_byte(0xFFFD, 0x00)
        cpu.write_byte(0xFFFE, 0x10)
        cpu.F = FLAG_Z
        write_program(cpu, [0xC8])
        assert cpu.step() == 11

    def test_ret_cc_not_taken_timing(self, cpu):
        cpu.SP = 0xFFFD
        cpu.F = 0
        write_program(cpu, [0xC8])
        assert cpu.step() == 5

    def test_ldir_repeating_timing(self, cpu):
        cpu.HL = 0x1000
        cpu.DE = 0x2000
        cpu.BC = 0x0002
        cpu.write_byte(0x1000, 0x01)
        cpu.write_byte(0x1001, 0x02)
        write_program(cpu, [0xED, 0xB0])
        assert cpu.step() == 21

    def test_ldir_final_timing(self, cpu):
        cpu.HL = 0x1000
        cpu.DE = 0x2000
        cpu.BC = 0x0001
        cpu.write_byte(0x1000, 0x01)
        write_program(cpu, [0xED, 0xB0])
        assert cpu.step() == 16

    @pytest.mark.parametrize(
        "program,mnemonic,expected_cycles",
        [
            (b"\xed\x42", "SBC HL,BC", 15),
            (b"\xed\x4a", "ADC HL,BC", 15),
            (b"\xed\x43\x00\x00", "LD (nn),BC", 20),
            (b"\xed\x4b\x00\x00", "LD BC,(nn)", 20),
            (b"\xed\x47", "LD I,A", 9),
            (b"\xed\x4f", "LD R,A", 9),
            (b"\xed\x57", "LD A,I", 9),
            (b"\xed\x5f", "LD A,R", 9),
            (b"\xed\x46", "IM 0", 8),
            (b"\xed\x56", "IM 1", 8),
            (b"\xed\x5e", "IM 2", 8),
            (b"\xed\x6f", "RLD", 18),
            (b"\xed\x67", "RRD", 18),
        ],
    )
    def test_ed_timing(self, cpu, program, mnemonic, expected_cycles):
        for i, b in enumerate(program):
            cpu.write_byte(i, b)
        cpu.PC = 0
        cycles = cpu.step()
        assert cycles == expected_cycles, (
            f"{mnemonic}: expected {expected_cycles}, got {cycles}"
        )

    @pytest.mark.parametrize(
        "program,mnemonic,expected_cycles",
        [
            (b"\xdd\x21\x00\x00", "LD IX,nn", 14),
            (b"\xdd\x7e\x00", "LD A,(IX+d)", 19),
            (b"\xdd\x36\x00\x00", "LD (IX+d),n", 19),
            (b"\xdd\x34\x00", "INC (IX+d)", 23),
            (b"\xdd\x86\x00", "ADD A,(IX+d)", 19),
            (b"\xdd\xe9", "JP (IX)", 8),
            (b"\xdd\xe5", "PUSH IX", 15),
            (b"\xdd\xe1", "POP IX", 14),
        ],
    )
    def test_ix_timing(self, cpu, program, mnemonic, expected_cycles):
        cpu.IX = 0x1000
        for i, b in enumerate(program):
            cpu.write_byte(i, b)
        cpu.PC = 0
        cycles = cpu.step()
        assert cycles == expected_cycles, (
            f"{mnemonic}: expected {expected_cycles}, got {cycles}"
        )


# ============================================================
# 22. Interrupts
# ============================================================


class TestInterrupts:
    def test_nmi_jumps_to_0066(self, cpu):
        cpu.PC = 0x1000
        cpu.SP = 0xFFFF
        cpu.IFF1 = True
        cpu.trigger_nmi()
        cpu.step()
        assert cpu.PC == 0x0066

    def test_nmi_clears_iff1(self, cpu):
        cpu.PC = 0x1000
        cpu.SP = 0xFFFF
        cpu.IFF1 = True
        cpu.trigger_nmi()
        cpu.step()
        assert not cpu.IFF1

    def test_im1(self, cpu):
        cpu.PC = 0x1000
        cpu.SP = 0xFFFF
        cpu.IM = 1
        cpu.IFF1 = True
        cpu.trigger_interrupt(0x00)
        cpu.step()
        assert cpu.PC == 0x0038

    def test_im2_vectored(self, cpu):
        cpu.PC = 0x1000
        cpu.SP = 0xFFFF
        cpu.IM = 2
        cpu.I = 0x10
        cpu.IFF1 = True
        cpu.write_byte(0x1020, 0x00)
        cpu.write_byte(0x1021, 0x40)
        cpu.trigger_interrupt(0x20)
        cpu.step()
        assert cpu.PC == 0x4000

    def test_di(self, cpu):
        cpu.IFF1 = True
        cpu.IFF2 = True
        write_program(cpu, [0xF3])
        cpu.step()
        assert not cpu.IFF1
        assert not cpu.IFF2

    def test_ei(self, cpu):
        cpu.IFF1 = False
        cpu.IFF2 = False
        write_program(cpu, [0xFB, 0x00])
        cpu.step()
        cpu.step()
        assert cpu.IFF1
        assert cpu.IFF2

    def test_interrupt_not_accepted_when_disabled(self, cpu):
        cpu.PC = 0x0000
        cpu.SP = 0xFFFF
        cpu.IM = 1
        cpu.IFF1 = False
        cpu.write_byte(0x0000, 0x00)
        cpu.trigger_interrupt(0x00)
        cpu.step()
        assert cpu.PC == 0x0001

    def test_retn(self, cpu):
        cpu.SP = 0xFFFD
        cpu.write_byte(0xFFFD, 0x00)
        cpu.write_byte(0xFFFE, 0x10)
        cpu.IFF1 = False
        cpu.IFF2 = True
        write_program(cpu, [0xED, 0x45])
        cpu.step()
        assert cpu.PC == 0x1000
        assert cpu.IFF1

    def test_nmi_exits_halt(self, cpu):
        write_program(cpu, [0x76])
        cpu.step()
        assert cpu.halted
        cpu.trigger_nmi()
        cpu.step()
        assert not cpu.halted
        assert cpu.PC == 0x0066

    def test_ld_a_i_interrupt_bug(self, cpu):
        cpu.I = 0x55
        cpu.IFF2 = True
        write_program(cpu, [0xED, 0x57, 0x00])
        cpu.step()
        cpu.trigger_interrupt(0x00)
        cpu.IM = 1
        cpu.IFF1 = True
        cpu.step()
        assert not flag_set(cpu, FLAG_PV)


# ============================================================
# 23. IX / IY Indexed Instructions
# ============================================================


class TestIndexed:
    def test_ld_ix_nn(self, cpu):
        write_program(cpu, [0xDD, 0x21, 0x34, 0x12])
        cpu.step()
        assert cpu.IX == 0x1234

    def test_ld_a_ix_d(self, cpu):
        cpu.IX = 0x1000
        cpu.write_byte(0x1005, 0xAB)
        write_program(cpu, [0xDD, 0x7E, 0x05])
        cpu.step()
        assert cpu.A == 0xAB

    def test_ld_ix_d_n(self, cpu):
        cpu.IX = 0x1000
        write_program(cpu, [0xDD, 0x36, 0x02, 0x99])
        cpu.step()
        assert cpu.read_byte(0x1002) == 0x99

    def test_add_a_ix_d(self, cpu):
        cpu.A = 0x10
        cpu.IX = 0x1000
        cpu.write_byte(0x1003, 0x05)
        write_program(cpu, [0xDD, 0x86, 0x03])
        cpu.step()
        assert cpu.A == 0x15

    def test_inc_ix_d(self, cpu):
        cpu.IX = 0x1000
        cpu.write_byte(0x1001, 0x09)
        write_program(cpu, [0xDD, 0x34, 0x01])
        cpu.step()
        assert cpu.read_byte(0x1001) == 0x0A

    def test_bit_ix_d(self, cpu):
        cpu.IX = 0x1000
        cpu.write_byte(0x1002, 0x08)
        write_program(cpu, [0xDD, 0xCB, 0x02, 0x5E])
        cpu.step()
        assert flag_clear(cpu, FLAG_Z)

    def test_set_ix_d(self, cpu):
        cpu.IX = 0x1000
        cpu.write_byte(0x1002, 0x00)
        write_program(cpu, [0xDD, 0xCB, 0x02, 0xC6])
        cpu.step()
        assert cpu.read_byte(0x1002) == 0x01

    def test_add_ix_bc(self, cpu):
        cpu.IX = 0x1000
        cpu.BC = 0x0100
        write_program(cpu, [0xDD, 0x09])
        cpu.step()
        assert cpu.IX == 0x1100

    @pytest.mark.parametrize(
        "reg,opcode_suffix",
        [
            ("B", 0x70),
            ("C", 0x71),
            ("D", 0x72),
            ("E", 0x73),
            ("A", 0x77),
        ],
    )
    def test_ld_ix_d_r(self, cpu, reg, opcode_suffix):
        cpu.IX = 0x1000
        setattr(cpu, reg, 0x42)
        write_program(cpu, [0xDD, opcode_suffix, 0x05])
        cpu.step()
        assert cpu.read_byte(0x1005) == 0x42

    @pytest.mark.parametrize(
        "reg,opcode_suffix",
        [
            ("B", 0x46),
            ("C", 0x4E),
            ("D", 0x56),
            ("E", 0x5E),
            ("H", 0x66),
            ("L", 0x6E),
            ("A", 0x7E),
        ],
    )
    def test_ld_r_ix_d(self, cpu, reg, opcode_suffix):
        cpu.IX = 0x1000
        cpu.write_byte(0x1005, 0x99)
        write_program(cpu, [0xDD, opcode_suffix, 0x05])
        cpu.step()
        assert getattr(cpu, reg) == 0x99


# ============================================================
# 24. ED-Prefix Instructions
# ============================================================


class TestEDInstructions:
    def test_ld_i_a(self, cpu):
        cpu.A = 0x42
        write_program(cpu, [0xED, 0x47])
        cpu.step()
        assert cpu.I == 0x42

    def test_ld_r_a(self, cpu):
        cpu.A = 0x11
        write_program(cpu, [0xED, 0x4F])
        cpu.step()
        assert cpu.R == 0x11

    def test_ld_a_i(self, cpu):
        cpu.I = 0x55
        cpu.IFF2 = True
        write_program(cpu, [0xED, 0x57])
        cpu.step()
        assert cpu.A == 0x55
        assert flag_set(cpu, FLAG_PV)

    def test_ld_a_i_iff2_clear(self, cpu):
        cpu.I = 0x55
        cpu.IFF2 = False
        write_program(cpu, [0xED, 0x57])
        cpu.step()
        assert cpu.A == 0x55
        assert flag_clear(cpu, FLAG_PV)

    def test_rld(self, cpu):
        cpu.A = 0x9A
        cpu.HL = 0x1000
        cpu.write_byte(0x1000, 0x31)
        write_program(cpu, [0xED, 0x6F])
        cpu.step()
        assert cpu.A == 0x93
        assert cpu.read_byte(0x1000) == 0x1A

    def test_rrd(self, cpu):
        cpu.A = 0x84
        cpu.HL = 0x1000
        cpu.write_byte(0x1000, 0x20)
        write_program(cpu, [0xED, 0x67])
        cpu.step()
        assert cpu.A == 0x80
        assert cpu.read_byte(0x1000) == 0x42

    @pytest.mark.parametrize("im_val,opcode", [(0, 0x46), (1, 0x56), (2, 0x5E)])
    def test_im(self, cpu, im_val, opcode):
        write_program(cpu, [0xED, opcode])
        cpu.step()
        assert cpu.IM == im_val


# ============================================================
# 25. HALT
# ============================================================


class TestHalt:
    def test_halt_stops_execution(self, cpu):
        write_program(cpu, [0x76])
        cpu.step()
        assert cpu.halted

    def test_halt_pc_does_not_advance(self, cpu):
        write_program(cpu, [0x76])
        cpu.step()
        pc_after = cpu.PC
        cpu.step()
        assert cpu.PC == pc_after


# ============================================================
# 26. NOP
# ============================================================


class TestNop:
    def test_nop_advances_pc(self, cpu):
        write_program(cpu, [0x00])
        cpu.step()
        assert cpu.PC == 1

    def test_nop_preserves_registers(self, cpu):
        cpu.A = 0x42
        cpu.BC = 0x1234
        cpu.DE = 0x5678
        cpu.HL = 0x9ABC
        cpu.F = 0xFF
        write_program(cpu, [0x00])
        cpu.step()
        assert cpu.A == 0x42
        assert cpu.BC == 0x1234
        assert cpu.DE == 0x5678
        assert cpu.HL == 0x9ABC
        assert cpu.F == 0xFF


# ============================================================
# 27. Undocumented: SLL (CB 30-37)
# ============================================================


class TestSLL:
    def test_sll_a(self, cpu):
        cpu.A = 0x00
        run_cb_instruction(cpu, 0x37)
        assert cpu.A == 0x01

    def test_sll_a_with_carry(self, cpu):
        cpu.A = 0x80
        run_cb_instruction(cpu, 0x37)
        assert cpu.A == 0x01
        assert flag_set(cpu, FLAG_C)


# ============================================================
# 28. Edge Cases / Regression Tests
# ============================================================


class TestEdgeCases:
    def test_sp_wrap_around(self, cpu):
        cpu.SP = 0x0001
        cpu.BC = 0x1234
        write_program(cpu, [0xC5])
        cpu.step()
        assert cpu.SP == 0xFFFF

    def test_pc_wrap_around(self, cpu):
        write_program(cpu, [0xC3, 0xFF, 0xFF])
        cpu.step()
        assert cpu.PC == 0xFFFF
        cpu.write_byte(0xFFFF, 0x00)
        cpu.step()
        assert cpu.PC == 0x0000

    def test_add_overflow_signed(self, cpu):
        cpu.A = 0x7F
        write_program(cpu, [0xC6, 0x01])
        cpu.step()
        assert cpu.A == 0x80
        assert flag_set(cpu, FLAG_PV)
        assert flag_set(cpu, FLAG_S)

    def test_r_register_increments(self, cpu):
        cpu.R = 0x00
        write_program(cpu, [0x00, 0x00, 0x00])
        cpu.step()
        assert cpu.R == 1
        cpu.step()
        assert cpu.R == 2
        cpu.step()
        assert cpu.R == 3

    def test_r_register_bit7_preserved(self, cpu):
        cpu.R = 0x80
        write_program(cpu, [0x00])
        cpu.step()
        assert cpu.R & 0x80

    def test_add_hl_hl(self, cpu):
        cpu.HL = 0x4000
        write_program(cpu, [0x29])
        cpu.step()
        assert cpu.HL == 0x8000


# ============================================================
# 29. Full Program Integration Tests
# ============================================================


class TestIntegration:
    def test_memcpy_loop(self, cpu):
        cpu.write_byte(0x1000, 0x11)
        cpu.write_byte(0x1001, 0x22)
        cpu.write_byte(0x1002, 0x33)

        program = [
            0x21,
            0x00,
            0x10,  # LD HL,0x1000
            0x11,
            0x00,
            0x20,  # LD DE,0x2000
            0x06,
            0x03,  # LD B,3
            0x7E,  # LD A,(HL)
            0x12,  # LD (DE),A
            0x23,  # INC HL
            0x13,  # INC DE
            0x05,  # DEC B
            0x20,
            0xF9,  # JR NZ,-7
        ]
        write_program(cpu, program)

        for _ in range(50):
            if cpu.PC >= len(program):
                break
            cpu.step()

        assert cpu.read_byte(0x2000) == 0x11
        assert cpu.read_byte(0x2001) == 0x22
        assert cpu.read_byte(0x2002) == 0x33

    def test_sum_array(self, cpu):
        cpu.write_byte(0x1000, 10)
        cpu.write_byte(0x1001, 20)
        cpu.write_byte(0x1002, 30)
        cpu.write_byte(0x1003, 40)

        program = [
            0x21,
            0x00,
            0x10,  # LD HL,0x1000
            0x06,
            0x04,  # LD B,4
            0x3E,
            0x00,  # LD A,0
            0x86,  # ADD A,(HL)
            0x23,  # INC HL
            0x05,  # DEC B
            0x20,
            0xFB,  # JR NZ,-5
        ]
        write_program(cpu, program)

        for _ in range(50):
            if cpu.PC >= len(program):
                break
            cpu.step()

        assert cpu.A == 100

    def test_call_ret_sequence(self, cpu):
        main = [
            0x3E,
            0x10,  # LD A,0x10
            0x06,
            0x20,  # LD B,0x20
            0xCD,
            0x00,
            0x01,  # CALL 0x0100
            0x76,  # HALT
        ]
        write_program(cpu, main)
        cpu.SP = 0xFFFF

        cpu.write_byte(0x0100, 0x80)  # ADD A,B
        cpu.write_byte(0x0101, 0xC9)  # RET

        for _ in range(10):
            if cpu.halted:
                break
            cpu.step()

        assert cpu.A == 0x30
        assert cpu.halted


# ============================================================
# 30. Undocumented Flags (F3, F5)
# ============================================================


class TestUndocumentedFlags:
    def test_add_f3_f5_from_result(self, cpu):
        cpu.A = 0x08
        write_program(cpu, [0xC6, 0x00])
        cpu.step()
        assert cpu.F & FLAG_F3

    def test_sub_f3_f5_from_result(self, cpu):
        cpu.A = 0x20
        write_program(cpu, [0xD6, 0x00])
        cpu.step()
        assert cpu.F & FLAG_F5

    def test_cp_f3_f5_from_operand(self, cpu):
        cpu.A = 0x00
        write_program(cpu, [0xFE, 0x28])
        cpu.step()
        assert cpu.F & FLAG_F3
        assert cpu.F & FLAG_F5


# ============================================================
# 31. DD/FD Prefix Undocumented Behavior
# ============================================================


class TestDDFDFallthrough:
    def test_dd_prefix_nop_fallthrough(self, cpu):
        write_program(cpu, [0xDD, 0x00])
        cpu.step()
        assert cpu.PC == 2

    def test_fd_prefix_nop_fallthrough(self, cpu):
        write_program(cpu, [0xFD, 0x00])
        cpu.step()
        assert cpu.PC == 2

    def test_dd_prefix_ld_a_n_fallthrough(self, cpu):
        write_program(cpu, [0xDD, 0x3E, 0x42])
        cpu.step()
        assert cpu.A == 0x42


# ============================================================
# 32. DDCB/FDCB Indexed Bit Operations
# ============================================================


class TestDDCBFDCB:
    def test_ddcb_rlc_ix_d(self, cpu):
        cpu.IX = 0x1000
        cpu.write_byte(0x1002, 0x80)
        write_program(cpu, [0xDD, 0xCB, 0x02, 0x06])
        cpu.step()
        assert cpu.read_byte(0x1002) == 0x01

    def test_ddcb_bit_ix_d(self, cpu):
        cpu.IX = 0x1000
        cpu.write_byte(0x1003, 0x08)
        write_program(cpu, [0xDD, 0xCB, 0x03, 0x5E])
        cpu.step()
        assert flag_clear(cpu, FLAG_Z)

    def test_ddcb_set_ix_d(self, cpu):
        cpu.IX = 0x1000
        cpu.write_byte(0x1001, 0x00)
        write_program(cpu, [0xDD, 0xCB, 0x01, 0xC6])
        cpu.step()
        assert cpu.read_byte(0x1001) == 0x01

    def test_ddcb_res_ix_d(self, cpu):
        cpu.IX = 0x1000
        cpu.write_byte(0x1001, 0xFF)
        write_program(cpu, [0xDD, 0xCB, 0x01, 0x86])
        cpu.step()
        assert cpu.read_byte(0x1001) == 0xFE

    @pytest.mark.parametrize(
        "program,mnemonic,expected_cycles",
        [
            ([0xDD, 0xCB, 0x00, 0x06], "RLC (IX+0)", 23),
            ([0xDD, 0xCB, 0x00, 0x46], "BIT 0,(IX+0)", 20),
            ([0xDD, 0xCB, 0x00, 0xC6], "SET 0,(IX+0)", 23),
            ([0xDD, 0xCB, 0x00, 0x86], "RES 0,(IX+0)", 23),
        ],
    )
    def test_ddcb_timing(self, cpu, program, mnemonic, expected_cycles):
        cpu.IX = 0x1000
        cpu.write_byte(0x1000, 0x00)
        for i, b in enumerate(program):
            cpu.write_byte(i, b)
        cpu.PC = 0
        cycles = cpu.step()
        assert cycles == expected_cycles, (
            f"{mnemonic}: expected {expected_cycles}, got {cycles}"
        )


# ============================================================
# 33. Repeat I/O Block Instructions
# ============================================================


class TestRepeatIOBlock:
    def test_inir_terminates_on_b_zero(self, cpu):
        cpu.B = 1
        cpu.C = 0x10
        cpu.HL = 0x2000
        cpu._io.ports[0x10] = 0x99
        write_program(cpu, [0xED, 0xB2])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0x99
        assert cpu.B == 0

    def test_otir_decrements_b(self, cpu):
        cpu.B = 2
        cpu.C = 0x10
        cpu.HL = 0x1000
        cpu.write_byte(0x1000, 0xAA)
        cpu.write_byte(0x1001, 0xBB)
        write_program(cpu, [0xED, 0xB3])
        step_n(cpu, 2)
        assert cpu.B == 0


# ============================================================
# 34. IX/IY Arithmetic Operations
# ============================================================


class TestIXIYArithmetic:
    def test_adc_ix_bc(self, cpu):
        cpu.IX = 0x0001
        cpu.BC = 0x0002
        cpu.F = 0
        write_program(cpu, [0xDD, 0xED, 0x4A])
        cpu.step()
        assert cpu.IX == 0x0003

    def test_adc_ix_bc_with_carry(self, cpu):
        cpu.IX = 0xFFFF
        cpu.BC = 0x0001
        cpu.F = FLAG_C
        write_program(cpu, [0xDD, 0xED, 0x4A])
        cpu.step()
        assert cpu.IX == 0x0001
        assert flag_set(cpu, FLAG_C)

    def test_sbc_ix_de(self, cpu):
        cpu.IX = 0x0010
        cpu.DE = 0x0002
        cpu.F = 0
        write_program(cpu, [0xDD, 0xED, 0x52])
        cpu.step()
        assert cpu.IX == 0x000E


# ============================================================
# 35. DAA Comprehensive Tests
# ============================================================


class TestDAAComprehensive:
    def test_daa_add_0f_01(self, cpu):
        cpu.A = 0x0F
        write_program(cpu, [0xC6, 0x01, 0x27])
        step_n(cpu, 2)
        assert cpu.A == 0x16
        assert flag_clear(cpu, FLAG_C)

    def test_daa_add_f9_01(self, cpu):
        cpu.A = 0xF9
        write_program(cpu, [0xC6, 0x01, 0x27])
        step_n(cpu, 2)
        assert cpu.A == 0x60
        assert flag_set(cpu, FLAG_C)

    def test_daa_sub_00_01(self, cpu):
        cpu.A = 0x00
        write_program(cpu, [0xD6, 0x01, 0x27])
        step_n(cpu, 2)
        assert cpu.A == 0x99
        assert flag_set(cpu, FLAG_C)

    def test_daa_preserves_z_flag(self, cpu):
        cpu.A = 0x90
        write_program(cpu, [0xC6, 0x10, 0x27])
        step_n(cpu, 2)
        assert cpu.A == 0x00
        assert flag_set(cpu, FLAG_Z)


# ============================================================
# 36. Q Factor Tracking (Patrik Rak discovery)
# ============================================================


class TestQFactor:
    def test_scf_after_flag_modifying_copies_a(self, cpu):
        cpu.A = 0x10
        write_program(cpu, [0x3D, 0x37])  # DEC A; SCF
        cpu.step()  # DEC A -> A=0x0F, Q=F
        cpu.step()  # SCF: Q=F, F3/F5 = A.3/A.5
        assert not (cpu.F & FLAG_F5)  # A.5=0
        assert cpu.F & FLAG_F3  # A.3=1
        assert cpu.F & FLAG_C

    def test_scf_after_non_flag_modifying_ors(self, cpu):
        cpu.A = 0x00
        cpu.F = 0x28  # F5=1, F3=1
        write_program(cpu, [0x00, 0x37])  # NOP; SCF
        cpu.step()  # NOP -> Q=0
        cpu.step()  # SCF: Q=0, F3/F5 = F|A
        assert cpu.F & FLAG_F5
        assert cpu.F & FLAG_F3

    def test_ccf_after_flag_modifying_copies_a(self, cpu):
        cpu.A = 0x30
        cpu.F = 0x01  # C=1
        write_program(cpu, [0x3D, 0x3F])  # DEC A; CCF
        cpu.step()  # DEC A -> A=0x2F, Q=F
        cpu.step()  # CCF: Q=F, F3/F5 = A.3/A.5
        assert cpu.F & FLAG_F5  # A.5=1
        assert cpu.F & FLAG_F3  # A.3=1
        assert not (cpu.F & FLAG_C)  # CCF toggles
