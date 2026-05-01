"""Shared fixtures and helpers for all Z80 tests."""
import pytest
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Import the nanobind module (was z80_py, now z80_core)
try:
    from z80_core import Z80 as Z80CPU, SimpleBus
except ImportError:
    # Fallback for development
    from z80_py import Z80 as Z80CPU

# Z80 Flag constants
FLAG_S  = 0x80  # Sign
FLAG_Z  = 0x40  # Zero
FLAG_F5 = 0x20  # Undocumented F5
FLAG_H  = 0x10  # Half carry
FLAG_F3 = 0x08  # Undocumented F3
FLAG_PV = 0x04  # Parity/Overflow
FLAG_N  = 0x02  # Add/Subtract
FLAG_C  = 0x01  # Carry

_F53 = FLAG_F5 | FLAG_F3

_shared_bus = None

def _get_bus():
    """Get or create shared bus."""
    global _shared_bus
    if _shared_bus is None:
        _shared_bus = SimpleBus()
    return _shared_bus

@pytest.fixture(scope="function")
def cpu():
    """Fresh Z80CPU instance for each test."""
    c = Z80CPU()
    c.bus = _get_bus()
    c.registers.F = 0
    c.registers.A = 0
    return c

def write_program(cpu, program_bytes, addr=0):
    """Write a sequence of bytes into CPU memory and set PC."""
    for i, b in enumerate(program_bytes):
        cpu.write_byte(addr + i, b)
    cpu.registers.PC = addr

def run_cb_instruction(cpu, cb_op):
    """Execute a CB-prefixed instruction."""
    cpu.write_byte(0, 0xCB)
    cpu.write_byte(1, cb_op)
    cpu.registers.PC = 0
    return cpu.step()

def run_ed_instruction(cpu, ed_op):
    """Execute an ED-prefixed instruction."""
    cpu.write_byte(0, 0xED)
    cpu.write_byte(1, ed_op)
    cpu.registers.PC = 0
    return cpu.step()

def run_dd_instruction(cpu, dd_op, displacement=0):
    """Execute a DD-prefixed instruction."""
    cpu.write_byte(0, 0xDD)
    cpu.write_byte(1, dd_op)
    if displacement != 0:
        cpu.write_byte(2, displacement & 0xFF)
    cpu.registers.PC = 0
    return cpu.step()

def run_fd_instruction(cpu, fd_op, displacement=0):
    """Execute a FD-prefixed instruction."""
    cpu.write_byte(0, 0xFD)
    cpu.write_byte(1, fd_op)
    if displacement != 0:
        cpu.write_byte(2, displacement & 0xFF)
    cpu.registers.PC = 0
    return cpu.step()

def run_ddcb_instruction(cpu, displacement, cb_op):
    """Execute a DDCB instruction."""
    cpu.write_byte(0, 0xDD)
    cpu.write_byte(1, 0xCB)
    cpu.write_byte(2, displacement & 0xFF)
    cpu.write_byte(3, cb_op)
    cpu.registers.PC = 0
    return cpu.step()

def run_fdcb_instruction(cpu, displacement, cb_op):
    """Execute a FDCB instruction."""
    cpu.write_byte(0, 0xFD)
    cpu.write_byte(1, 0xCB)
    cpu.write_byte(2, displacement & 0xFF)
    cpu.write_byte(3, cb_op)
    cpu.registers.PC = 0
    return cpu.step()

def step_n(cpu, n):
    """Step the CPU n times, returning total cycles."""
    total = 0
    for _ in range(n):
        total += cpu.step()
    return total

def _add_flags(a, b, carry=0):
    """Compute expected flags for ADD/SBC A,n."""
    result = a + b + carry
    r = result & 0xFF
    f = r & _F53
    if result & 0x100:
        f |= FLAG_C
    if ((a & 0x0F) + (b & 0x0F) + carry) & 0x10:
        f |= FLAG_H
    if r == 0:
        f |= FLAG_Z
    if r & 0x80:
        f |= FLAG_S
    # PV = signed overflow
    if carry:
        if (~(a ^ b) & (a ^ r) & 0x80):
            f |= FLAG_PV
    else:
        if (r ^ a) & (r ^ b) & 0x80:
            f |= FLAG_PV
    return f

def _sub_flags(a, b, carry=0):
    """Compute expected flags for SUB/SBC A,n."""
    result = a - b - carry
    r = result & 0xFF
    f = FLAG_N | (r & _F53)
    if result < 0:
        f |= FLAG_C
    if (a & 0x0F) < ((b & 0x0F) + carry):
        f |= FLAG_H
    if r == 0:
        f |= FLAG_Z
    if r & 0x80:
        f |= FLAG_S
    # PV = signed overflow
    if (a ^ b) & (a ^ r) & 0x80:
        f |= FLAG_PV
    return f

def _parity(val):
    """Return True if val has even parity."""
    return bin(val).count("1") % 2 == 0

def flag_set(cpu, flag):
    """Check if a flag is set."""
    return bool(cpu.registers.F & flag)

def flag_clear(cpu, flag):
    """Check if a flag is clear."""
    return not (cpu.registers.F & flag)

def assert_flags(cpu, expected_flags, mask=None):
    """Assert CPU flags match expected."""
    if mask is None:
        mask = FLAG_S | FLAG_Z | FLAG_F5 | FLAG_H | FLAG_F3 | FLAG_PV | FLAG_N | FLAG_C
    actual = cpu.registers.F & mask
    expected = expected_flags & mask
    assert actual == expected, f"Flags mismatch: expected 0x{expected:02X}, got 0x{actual:02X}"

# Comprehensive instruction opcode table for testing
INSTRUCTIONS = {
    # 8-bit loads
    "LD B,n": 0x06, "LD C,n": 0x0E, "LD D,n": 0x16, "LD E,n": 0x1E,
    "LD H,n": 0x26, "LD L,n": 0x2E, "LD A,n": 0x3E,
    # 8-bit register loads
    "LD A,B": 0x78, "LD A,C": 0x79, "LD A,D": 0x7A, "LD A,E": 0x7B,
    "LD A,H": 0x7C, "LD A,L": 0x7D, "LD A,A": 0x7F,
    "LD B,B": 0x40, "LD B,C": 0x41, "LD B,D": 0x42, "LD B,E": 0x43,
    "LD B,H": 0x44, "LD B,L": 0x45, "LD B,A": 0x47,
    # 8-bit arithmetic
    "ADD A,B": 0x80, "ADD A,C": 0x81, "ADD A,D": 0x82, "ADD A,E": 0x83,
    "ADD A,H": 0x84, "ADD A,L": 0x85, "ADD A,(HL)": 0x86, "ADD A,A": 0x87,
    "ADC A,B": 0x88, "ADC A,C": 0x89, "ADC A,D": 0x8A, "ADC A,E": 0x8B,
    "ADC A,H": 0x8C, "ADC A,L": 0x8D, "ADC A,(HL)": 0x8E, "ADC A,A": 0x8F,
    "SUB B": 0x90, "SUB C": 0x91, "SUB D": 0x92, "SUB E": 0x93,
    "SUB H": 0x94, "SUB L": 0x95, "SUB (HL)": 0x96, "SUB A": 0x97,
    "SBC A,B": 0x98, "SBC A,C": 0x99, "SBC A,D": 0x9A, "SBC A,E": 0x9B,
    "SBC A,H": 0x9C, "SBC A,L": 0x9D, "SBC A,(HL)": 0x9E, "SBC A,A": 0x9F,
    "AND B": 0xA0, "AND C": 0xA1, "AND D": 0xA2, "AND E": 0xA3,
    "AND H": 0xA4, "AND L": 0xA5, "AND (HL)": 0xA6, "AND A": 0xA7,
    "XOR B": 0xA8, "XOR C": 0xA9, "XOR D": 0xAA, "XOR E": 0xAB,
    "XOR H": 0xAC, "XOR L": 0xAD, "XOR (HL)": 0xAE, "XOR A": 0xAF,
    "OR B": 0xB0, "OR C": 0xB1, "OR D": 0xB2, "OR E": 0xB3,
    "OR H": 0xB4, "OR L": 0xB5, "OR (HL)": 0xB6, "OR A": 0xB7,
    "CP B": 0xB8, "CP C": 0xB9, "CP D": 0xBA, "CP E": 0xBB,
    "CP H": 0xBC, "CP L": 0xBD, "CP (HL)": 0xBE, "CP A": 0xBF,
    # INC/DEC 8-bit
    "INC A": 0x3C, "INC B": 0x04, "INC C": 0x0C, "INC D": 0x14,
    "INC E": 0x1C, "INC H": 0x24, "INC L": 0x2C, "INC (HL)": 0x34,
    "DEC A": 0x3D, "DEC B": 0x05, "DEC C": 0x0D, "DEC D": 0x15,
    "DEC E": 0x1D, "DEC H": 0x25, "DEC L": 0x2D, "DEC (HL)": 0x35,
    # 16-bit loads
    "LD BC,nn": 0x01, "LD DE,nn": 0x11, "LD HL,nn": 0x21, "LD SP,nn": 0x31,
    "LD (BC),A": 0x02, "LD (DE),A": 0x12, "LD (HL),n": 0x36,
    "LD A,(BC)": 0x0A, "LD A,(DE)": 0x1A,
    # 16-bit arithmetic
    "ADD HL,BC": 0x09, "ADD HL,DE": 0x19, "ADD HL,HL": 0x29, "ADD HL,SP": 0x39,
    "INC BC": 0x03, "INC DE": 0x13, "INC HL": 0x23, "INC SP": 0x33,
    "DEC BC": 0x0B, "DEC DE": 0x1B, "DEC HL": 0x2B, "DEC SP": 0x3B,
    # Jumps and calls
    "JP nn": 0xC3, "JP (HL)": 0xE9, "CALL nn": 0xCD, "RET": 0xC9,
    "JR e": 0x18, "DJNZ e": 0x10,
    # Rotates on accumulator
    "RLCA": 0x07, "RRCA": 0x0F, "RLA": 0x17, "RRA": 0x1F,
    # Misc
    "DAA": 0x27, "CPL": 0x2F, "CCF": 0x3F, "SCF": 0x37,
    "NOP": 0x00, "HALT": 0x76, "DI": 0xF3, "EI": 0xFB,
    "EX AF,AF'": 0x08, "EX DE,HL": 0xEB, "EXX": 0xD9,
    "POP BC": 0xC1, "POP DE": 0xD1, "POP HL": 0xE1, "POP AF": 0xF1,
    "PUSH BC": 0xC5, "PUSH DE": 0xD5, "PUSH HL": 0xE5, "PUSH AF": 0xF5,
}
