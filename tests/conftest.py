"""Shared fixtures and helpers for all Z80 tests."""

import pytest
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

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

_F53 = FLAG_F5 | FLAG_F3


@pytest.fixture
def cpu():
    """Fresh Z80CPU instance for each test."""
    c = Z80CPU()
    c.regs.F = 0
    c.regs.A = 0
    return c


def write_program(cpu, program_bytes, addr=0):
    """Write a sequence of bytes into CPU memory and set PC."""
    for i, b in enumerate(program_bytes):
        cpu.write_byte(addr + i, b)
    cpu.regs.PC = addr


def flag_set(cpu, flag):
    """Check if a flag is set."""
    return bool(cpu.regs.F & flag)


def flag_clear(cpu, flag):
    """Check if a flag is clear."""
    return not (cpu.regs.F & flag)


def run_cb_instruction(cpu, cb_op):
    """Execute a CB-prefixed instruction."""
    cpu.write_byte(0, 0xCB)
    cpu.write_byte(1, cb_op)
    cpu.regs.PC = 0
    return cpu.step()


def step_n(cpu, n):
    """Step the CPU n times, returning total cycles."""
    total = 0
    for _ in range(n):
        total += cpu.step()
    return total


def _add_flags(a, b):
    """Compute expected flags for ADD A,n."""
    result = a + b
    r = result & 0xFF
    f = r & _F53
    if result & 0x100:
        f |= FLAG_C
    if ((a & 0x0F) + (b & 0x0F)) & 0x10:
        f |= FLAG_H
    if r == 0:
        f |= FLAG_Z
    if r & 0x80:
        f |= FLAG_S
    if bin(r).count("1") % 2 == 0:
        f |= FLAG_PV
    return f


def _sub_flags(a, b):
    """Compute expected flags for SUB A,n."""
    result = a - b
    r = result & 0xFF
    f = FLAG_N | (r & _F53)
    if result < 0:
        f |= FLAG_C
    if (a & 0x0F) < (b & 0x0F):
        f |= FLAG_H
    if r == 0:
        f |= FLAG_Z
    if r & 0x80:
        f |= FLAG_S
    if bin(r).count("1") % 2 == 0:
        f |= FLAG_PV
    return f


def _parity(val):
    """Return True if val has even parity."""
    return bin(val).count("1") % 2 == 0
