#!/usr/bin/env python3
"""Comprehensive CB prefix (rotate/shift) instruction tests."""
import pytest
from conftest import (
    write_program, cpu, run_cb_instruction,
    FLAG_S, FLAG_Z, FLAG_F5, FLAG_H, FLAG_F3, FLAG_PV, FLAG_N, FLAG_C,
    assert_flags, flag_set, flag_clear, _parity
)


class TestRLC:
    """RLC r - Rotate left circular."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x00), ("C", 0x01), ("D", 0x02), ("E", 0x03),
        ("H", 0x04), ("L", 0x05), ("A", 0x07),
    ])
    def test_rlc_r(self, cpu, reg, opcode):
        """RLC r — rotate left circular."""
        setattr(cpu.registers, reg, 0x85)  # 10000101
        run_cb_instruction(cpu, opcode)
        # 10000101 << 1 = 00001011, C=1 (original bit 7)
        assert getattr(cpu.registers, reg) == 0x0B
        assert cpu.registers.F & FLAG_C  # Bit 7 was 1

    def test_rlc_hl(self, cpu):
        """RLC (HL) — rotate left circular via HL."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x85)
        run_cb_instruction(cpu, 0x06)
        assert cpu.read_byte(0x2000) == 0x0B
        assert cpu.registers.F & FLAG_C

    def test_rlc_no_carry(self, cpu):
        """RLC r — no carry when bit 7 is 0."""
        cpu.registers.B = 0x42  # 01000010
        run_cb_instruction(cpu, 0x00)
        assert cpu.registers.B == 0x84  # 10000100
        assert flag_clear(cpu, FLAG_C)

    def test_rlc_flags(self, cpu):
        """RLC r — verify all flags."""
        cpu.registers.B = 0x80
        run_cb_instruction(cpu, 0x00)
        assert flag_set(cpu, FLAG_C)
        assert flag_clear(cpu, FLAG_S)  # Result 0x01 has bit 7 clear
        assert flag_clear(cpu, FLAG_Z)
        assert flag_clear(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)
        assert flag_clear(cpu, FLAG_PV)  # 0x01 has odd parity (1 bit)

    def test_rlc_zero_result(self, cpu):
        """RLC r — Z flag set when result is 0."""
        cpu.registers.B = 0x00
        run_cb_instruction(cpu, 0x00)
        assert cpu.registers.B == 0x00
        assert flag_set(cpu, FLAG_Z)


class TestRRC:
    """RRC r - Rotate right circular."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x08), ("C", 0x09), ("D", 0x0A), ("E", 0x0B),
        ("H", 0x0C), ("L", 0x0D), ("A", 0x0F),
    ])
    def test_rrc_r(self, cpu, reg, opcode):
        """RRC r — rotate right circular."""
        setattr(cpu.registers, reg, 0x85)  # 10000101
        run_cb_instruction(cpu, opcode)
        # 10000101 rotated right = 11000010, C=1 (original bit 0)
        assert getattr(cpu.registers, reg) == 0xC2
        assert cpu.registers.F & FLAG_C

    def test_rrc_hl(self, cpu):
        """RRC (HL) — rotate right circular via HL."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x85)
        run_cb_instruction(cpu, 0x0E)
        assert cpu.read_byte(0x2000) == 0xC2
        assert cpu.registers.F & FLAG_C

    def test_rrc_no_carry(self, cpu):
        """RRC r — no carry when bit 0 is 0."""
        cpu.registers.B = 0x42  # 01000010
        run_cb_instruction(cpu, 0x08)
        assert cpu.registers.B == 0x21  # 00100001
        assert flag_clear(cpu, FLAG_C)


class TestRL:
    """RL r - Rotate left through carry."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x10), ("C", 0x11), ("D", 0x12), ("E", 0x13),
        ("H", 0x14), ("L", 0x15), ("A", 0x17),
    ])
    def test_rl_r(self, cpu, reg, opcode):
        """RL r — rotate left through carry."""
        setattr(cpu.registers, reg, 0x85)  # 10000101
        cpu.registers.F = 0  # No carry
        run_cb_instruction(cpu, opcode)
        # 10000101 << 1 = 00001010, C=1
        assert getattr(cpu.registers, reg) == 0x0A
        assert cpu.registers.F & FLAG_C

    def test_rl_r_with_carry(self, cpu):
        """RL r — with carry set."""
        cpu.registers.B = 0x85
        cpu.registers.F = FLAG_C  # Carry set
        run_cb_instruction(cpu, 0x10)  # RL B
        # 10000101 << 1 | 1 = 00001011
        assert cpu.registers.B == 0x0B

    def test_rl_hl(self, cpu):
        """RL (HL) — rotate left through carry via HL."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x85)
        cpu.registers.F = 0
        run_cb_instruction(cpu, 0x16)
        assert cpu.read_byte(0x2000) == 0x0A
        assert cpu.registers.F & FLAG_C


class TestRR:
    """RR r - Rotate right through carry."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x18), ("C", 0x19), ("D", 0x1A), ("E", 0x1B),
        ("H", 0x1C), ("L", 0x1D), ("A", 0x1F),
    ])
    def test_rr_r(self, cpu, reg, opcode):
        """RR r — rotate right through carry."""
        setattr(cpu.registers, reg, 0x85)  # 10000101
        cpu.registers.F = 0  # No carry
        run_cb_instruction(cpu, opcode)
        # 10000101 >> 1 = 01000010, C=1
        assert getattr(cpu.registers, reg) == 0x42
        assert cpu.registers.F & FLAG_C

    def test_rr_r_with_carry(self, cpu):
        """RR r — with carry set."""
        cpu.registers.B = 0x85
        cpu.registers.F = FLAG_C  # Carry set
        run_cb_instruction(cpu, 0x18)  # RR B
        # 10000101 >> 1 | 0x80 = 11000010
        assert cpu.registers.B == 0xC2

    def test_rr_hl(self, cpu):
        """RR (HL) — rotate right through carry via HL."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x85)
        cpu.registers.F = 0
        run_cb_instruction(cpu, 0x1E)
        assert cpu.read_byte(0x2000) == 0x42
        assert cpu.registers.F & FLAG_C


class TestSLA:
    """SLA r - Shift left arithmetic (same as SLL but clears bit 0)."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x20), ("C", 0x21), ("D", 0x22), ("E", 0x23),
        ("H", 0x24), ("L", 0x25), ("A", 0x27),
    ])
    def test_sla_r(self, cpu, reg, opcode):
        """SLA r — shift left arithmetic."""
        setattr(cpu.registers, reg, 0x85)  # 10000101
        run_cb_instruction(cpu, opcode)
        # 10000101 << 1 = 00001010, C=1
        assert getattr(cpu.registers, reg) == 0x0A
        assert cpu.registers.F & FLAG_C

    def test_sla_hl(self, cpu):
        """SLA (HL) — shift left arithmetic via HL."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x85)
        run_cb_instruction(cpu, 0x26)
        assert cpu.read_byte(0x2000) == 0x0A
        assert cpu.registers.F & FLAG_C

    def test_sla_no_carry(self, cpu):
        """SLA r — no carry when MSB is 0."""
        cpu.registers.B = 0x42  # 01000010
        run_cb_instruction(cpu, 0x20)
        assert cpu.registers.B == 0x84  # 10000100
        assert flag_clear(cpu, FLAG_C)


class TestSRA:
    """SRA r - Shift right arithmetic (preserve sign)."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x28), ("C", 0x29), ("D", 0x2A), ("E", 0x2B),
        ("H", 0x2C), ("L", 0x2D), ("A", 0x2F),
    ])
    def test_sra_r(self, cpu, reg, opcode):
        """SRA r — shift right arithmetic (preserve sign)."""
        setattr(cpu.registers, reg, 0x85)  # 10000101
        run_cb_instruction(cpu, opcode)
        # 10000101 >> 1 = 11000010 (sign bit preserved), C=1
        assert getattr(cpu.registers, reg) == 0xC2
        assert cpu.registers.F & FLAG_C

    def test_sra_hl(self, cpu):
        """SRA (HL) — shift right arithmetic via HL."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x85)
        run_cb_instruction(cpu, 0x2E)
        assert cpu.read_byte(0x2000) == 0xC2
        assert cpu.registers.F & FLAG_C

    def test_sra_positive(self, cpu):
        """SRA r — positive number, sign bit stays 0."""
        cpu.registers.B = 0x42  # 01000010
        run_cb_instruction(cpu, 0x28)
        assert cpu.registers.B == 0x21  # 00100001
        assert flag_clear(cpu, FLAG_C)


class TestSLL:
    """SLL r - Shift left logical (undocumented, sets bit 0)."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x30), ("C", 0x31), ("D", 0x32), ("E", 0x33),
        ("H", 0x34), ("L", 0x35), ("A", 0x37),
    ])
    def test_sll_r(self, cpu, reg, opcode):
        """SLL r — shift left logical (set bit 0)."""
        setattr(cpu.registers, reg, 0x85)  # 10000101
        run_cb_instruction(cpu, opcode)
        # 10000101 << 1 | 1 = 00001011
        assert getattr(cpu.registers, reg) == 0x0B
        assert cpu.registers.F & FLAG_C

    def test_sll_hl(self, cpu):
        """SLL (HL) — shift left logical via HL."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x85)
        run_cb_instruction(cpu, 0x36)
        assert cpu.read_byte(0x2000) == 0x0B
        assert cpu.registers.F & FLAG_C


class TestSRL:
    """SRL r - Shift right logical (reset sign)."""
    @pytest.mark.parametrize("reg,opcode", [
        ("B", 0x38), ("C", 0x39), ("D", 0x3A), ("E", 0x3B),
        ("H", 0x3C), ("L", 0x3D), ("A", 0x3F),
    ])
    def test_srl_r(self, cpu, reg, opcode):
        """SRL r — shift right logical (reset sign)."""
        setattr(cpu.registers, reg, 0x85)  # 10000101
        run_cb_instruction(cpu, opcode)
        # 10000101 >> 1 = 01000010, C=1
        assert getattr(cpu.registers, reg) == 0x42
        assert cpu.registers.F & FLAG_C

    def test_srl_hl(self, cpu):
        """SRL (HL) — shift right logical via HL."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x85)
        run_cb_instruction(cpu, 0x3E)
        assert cpu.read_byte(0x2000) == 0x42
        assert cpu.registers.F & FLAG_C

    def test_srl_no_carry(self, cpu):
        """SRL r — no carry when LSB is 0."""
        cpu.registers.B = 0x42  # 01000010
        run_cb_instruction(cpu, 0x38)
        assert cpu.registers.B == 0x21  # 00100001
        assert flag_clear(cpu, FLAG_C)


class TestCBFlags:
    """Verify flags for all CB prefix instructions."""
    def test_rlc_sets_h_n_clear(self, cpu):
        """RLC clears H and N flags."""
        cpu.registers.F = FLAG_H | FLAG_N
        cpu.registers.B = 0x01
        run_cb_instruction(cpu, 0x00)
        assert flag_clear(cpu, FLAG_H)
        assert flag_clear(cpu, FLAG_N)

    def test_rlc_sets_f5_f3(self, cpu):
        """RLC — verify PV is set for even parity result."""
        cpu.registers.B = 0x28  # Result 0x50 has 2 bits = even parity
        run_cb_instruction(cpu, 0x00)
        assert flag_set(cpu, FLAG_PV)  # even parity

    def test_parity_for_rotates(self, cpu):
        """All CB instructions set PV = parity of result."""
        cpu.registers.B = 0x03  # Two 1-bits = even parity
        run_cb_instruction(cpu, 0x00)  # RLC B
        assert cpu.registers.B == 0x06
        assert cpu.registers.F & FLAG_PV  # Even parity

        cpu.registers.B = 0x01  # One 1-bit = odd parity
        run_cb_instruction(cpu, 0x00)
        assert not (cpu.registers.F & FLAG_PV)  # Odd parity


class TestCBCycles:
    """Verify cycle counts for CB prefix instructions."""
    def test_rlc_r_cycles(self, cpu):
        """RLC r takes 8 cycles."""
        cpu.registers.B = 0x10
        assert run_cb_instruction(cpu, 0x00) == 8

    def test_rlc_hl_cycles(self, cpu):
        """RLC (HL) takes 15 cycles."""
        cpu.registers.HL = 0x2000
        assert run_cb_instruction(cpu, 0x06) == 15

    def test_rrc_r_cycles(self, cpu):
        """RRC r takes 8 cycles."""
        cpu.registers.B = 0x10
        assert run_cb_instruction(cpu, 0x08) == 8

    def test_sla_hl_cycles(self, cpu):
        """SLA (HL) takes 15 cycles."""
        cpu.registers.HL = 0x2000
        assert run_cb_instruction(cpu, 0x26) == 15
