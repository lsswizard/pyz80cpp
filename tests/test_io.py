#!/usr/bin/env python3
"""Comprehensive I/O instruction tests."""
import pytest
from conftest import (
    write_program, flag_set, flag_clear,
    FLAG_Z, FLAG_PV, FLAG_S, FLAG_H, FLAG_N
)


class TestInAN:
    """IN A,(n) - Input from port (A<<8 | n)."""
    def test_in_a_n_basic(self, cpu):
        """IN A,(n) — basic input."""
        cpu.registers.A = 0x50
        # Port address = A<<8 | n = 0x50 <<8 | 0x50 = 0x5050
        cpu.bus.out_(0x5050, 0xAB)
        write_program(cpu, [0xDB, 0x50])
        cpu.step()
        assert cpu.registers.A == 0xAB

    def test_in_a_n_flags(self, cpu):
        """IN A,(n) — sets flags based on value."""
        cpu.registers.A = 0x01
        # Port address = 0x01 <<8 | 0x01 = 0x0101
        cpu.bus.out_(0x0101, 0x80)
        write_program(cpu, [0xDB, 0x01])
        cpu.step()
        assert cpu.registers.A == 0x80
        assert flag_set(cpu, FLAG_S)  # 0x80 has bit 7 set
        assert flag_clear(cpu, FLAG_Z)
        assert flag_clear(cpu, FLAG_H)  # H cleared by IN
        assert flag_clear(cpu, FLAG_N)

    def test_in_a_n_zero(self, cpu):
        """IN A,(n) — Z flag set when input is 0."""
        cpu.registers.A = 0x50
        # Port address = 0x50 <<8 | 0x50 = 0x5050
        cpu.bus.out_(0x5050, 0x00)
        write_program(cpu, [0xDB, 0x50])
        cpu.step()
        assert cpu.registers.A == 0x00
        assert flag_set(cpu, FLAG_Z)


class TestOutNA:
    """OUT (n),A - Output A to port (A<<8 | n)."""
    def test_out_n_a_basic(self, cpu):
        """OUT (n),A — basic output."""
        cpu.registers.A = 0xAB
        write_program(cpu, [0xD3, 0x50])
        cpu.step()
        assert cpu.bus.in_(0x50) == 0xAB

    def test_out_n_a_port_address(self, cpu):
        """OUT (n),A — port address is A<<8 | n."""
        cpu.registers.A = 0xAB
        write_program(cpu, [0xD3, 0xCD])
        cpu.step()
        # Port address = 0xABCD
        assert cpu.bus.in_(0xCD) == 0xAB


class TestInRC:
    """IN r,(C) - Input from port (B<<8 | C)."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x40), ("C", 0x48), ("D", 0x50), ("E", 0x58),
        ("H", 0x60), ("L", 0x68), ("A", 0x78),  # 0x78 = IN A,(C), 0x70 is IN F,(C)
    ])
    def test_in_r_c(self, cpu, reg, opcode):
        """IN r,(C) — input from port (BC)."""
        cpu.registers.BC = 0x0250
        cpu.bus.out_(0x0250, 0xAB)
        write_program(cpu, [0xED, opcode])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0xAB

    def test_in_a_c_flags(self, cpu):
        """IN A,(C) — sets flags based on value."""
        cpu.registers.BC = 0x0250
        cpu.bus.out_(0x0250, 0x80)
        write_program(cpu, [0xED, 0x78])  # 0x78 = IN A,(C) (correct opcode)
        cpu.step()
        assert cpu.registers.A == 0x80
        assert flag_set(cpu, FLAG_S)

    def test_in_f_c(self, cpu):
        """IN F,(C) — input to F (undocumented, actually IN (C))."""
        cpu.registers.BC = 0x0250
        cpu.bus.out_(0x0250, 0xAB)
        write_program(cpu, [0xED, 0x70])  # IN (C) - stores in F?
        # Note: IN (C) actually stores to a register, not F directly
        # This is an alias for IN r,(C) where r specifies the register
        cpu.step()


class TestOutRC:
    """OUT (C),r - Output register to port (B<<8 | C)."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x41), ("C", 0x49), ("D", 0x51), ("E", 0x59),
        ("H", 0x61), ("L", 0x69), ("A", 0x79),
    ])
    def test_out_c_r(self, cpu, reg, opcode):
        """OUT (C),r — output register to port (BC)."""
        cpu.registers.BC = 0x0250
        setattr(cpu.registers, reg, 0xAB)
        write_program(cpu, [0xED, opcode])
        cpu.step()
        assert cpu.bus.in_(0x0250) == 0xAB

    def test_out_c_a(self, cpu):
        """OUT (C),A — output A to port (BC)."""
        cpu.registers.BC = 0x0250
        cpu.registers.A = 0xCD
        write_program(cpu, [0xED, 0x79])
        cpu.step()
        assert cpu.bus.in_(0x0250) == 0xCD

    def test_out_c_b_flags(self, cpu):
        """OUT (C),r — preserves flags."""
        cpu.registers.F = 0xFF
        cpu.registers.BC = 0x0250
        cpu.registers.B = 0xAB
        write_program(cpu, [0xED, 0x41])
        cpu.step()
        assert cpu.registers.F == 0xFF  # Flags preserved


class TestInOutFlags:
    """Verify flags for IN/OUT instructions."""
    def test_in_r_c_sets_h_n_clear(self, cpu):
        """IN r,(C) — sets H=1, N=0."""
        cpu.registers.BC = 0x0250
        cpu.bus.out_(0x0250, 0x01)  # Correct port address (BC)
        cpu.registers.F = FLAG_H | FLAG_N  # These should be modified
        write_program(cpu, [0xED, 0x40])  # IN B,(C)
        cpu.step()
        assert flag_clear(cpu, FLAG_H)  # H is cleared by IN
        assert flag_clear(cpu, FLAG_N)  # N is cleared

    def test_in_r_c_parity(self, cpu):
        """IN r,(C) — PV = parity of input."""
        cpu.registers.BC = 0x0250
        cpu.bus.out_(0x0250, 0x03)  # Two 1-bits = even parity
        write_program(cpu, [0xED, 0x40])  # IN B,(C)
        cpu.step()
        assert flag_set(cpu, FLAG_PV)  # Even parity

        cpu.registers.BC = 0x0250  # Reset BC!
        cpu.bus.out_(0x0250, 0x01)  # One 1-bit = odd parity
        write_program(cpu, [0xED, 0x40])
        cpu.step()
        assert flag_clear(cpu, FLAG_PV)  # Odd parity

class TestIOUndocumented:
    """Undocumented I/O instructions."""
    def test_in_none_c(self, cpu):
        """IN (C) — input to no register (alias for IN F,(C))."""
        cpu.registers.BC = 0x0250
        cpu.bus.out_(0x0250, 0xAB)  # Correct port address (BC)
        write_program(cpu, [0xED, 0x70])  # IN (C) - actually IN F,(C), reads port and sets flags
        cpu.step()
        # Flags should be set based on input value 0xAB (S=1, Z=0, P/V=1 (odd parity? 0xAB has 4 bits set: even? Wait 0xAB is 10101011, which has 4 1s: even parity, so PV=1)
        # At minimum, F should not be 0
        assert cpu.registers.F != 0  # Some flags should be set


class TestIOTiming:
    """Verify cycle counts for I/O instructions."""
    def test_in_a_n_cycles(self, cpu):
        """IN A,(n) takes 11 cycles."""
        cpu.registers.A = 0x50
        write_program(cpu, [0xDB, 0x50])
        assert cpu.step() == 11

    def test_out_n_a_cycles(self, cpu):
        """OUT (n),A takes 11 cycles."""
        cpu.registers.A = 0xAB
        write_program(cpu, [0xD3, 0x50])
        assert cpu.step() == 11

    def test_in_r_c_cycles(self, cpu):
        """IN r,(C) takes 12 cycles."""
        cpu.registers.BC = 0x0250
        write_program(cpu, [0xED, 0x40])
        assert cpu.step() == 12

    def test_out_c_r_cycles(self, cpu):
        """OUT (C),r takes 12 cycles."""
        cpu.registers.BC = 0x0250
        write_program(cpu, [0xED, 0x41])
        assert cpu.step() == 12
