#!/usr/bin/env python3
"""Comprehensive CB-prefixed BIT, SET, RES instruction tests."""
import pytest
from conftest import write_program, run_cb_instruction, flag_set, flag_clear, FLAG_Z, FLAG_H, FLAG_C, FLAG_PV, FLAG_N, FLAG_S


class TestBIT:
    """BIT b,r - Test bit in register."""
    @pytest.mark.parametrize("bit,reg,opcode", [
        (0, "B", 0x40), (1, "B", 0x48), (2, "B", 0x50), (3, "B", 0x58),
        (4, "B", 0x60), (5, "B", 0x68), (6, "B", 0x70), (7, "B", 0x78),
        (0, "C", 0x41), (7, "C", 0x79),
        (0, "D", 0x42), (7, "D", 0x7A),
        (0, "E", 0x43), (7, "E", 0x7B),
        (0, "H", 0x44), (7, "H", 0x7C),
        (0, "L", 0x45), (7, "L", 0x7D),
        (0, "A", 0x47), (7, "A", 0x7F),
    ])
    def test_bit_r_set(self, cpu, bit, reg, opcode):
        """BIT b,r — bit is set."""
        setattr(cpu.registers, reg, 1 << bit)
        run_cb_instruction(cpu, opcode)
        assert flag_clear(cpu, FLAG_Z)  # Bit is set, so Z = 0
        assert cpu.registers.F & FLAG_H  # H always set for BIT
        assert flag_clear(cpu, FLAG_PV)  # PV = Z (bit is set, so PV = 0)

    @pytest.mark.parametrize("bit,reg,opcode", [
        (0, "B", 0x40), (7, "B", 0x78),
    ])
    def test_bit_r_clear(self, cpu, bit, reg, opcode):
        """BIT b,r — bit is clear."""
        setattr(cpu.registers, reg, 0x00)
        run_cb_instruction(cpu, opcode)
        assert flag_set(cpu, FLAG_Z)  # Bit is clear, so Z = 1
        assert cpu.registers.F & FLAG_H  # H always set for BIT
        assert flag_set(cpu, FLAG_PV)  # PV = Z (bit is clear, so PV = 1)

    def test_bit_7_sets_sign(self, cpu):
        """BIT 7,r — sets S flag when bit 7 is set."""
        cpu.registers.B = 0x80
        run_cb_instruction(cpu, 0x78)  # BIT 7,B
        assert cpu.registers.F & FLAG_S

    def test_bit_7_clears_sign(self, cpu):
        """BIT 7,r — clears S flag when bit 7 is clear."""
        cpu.registers.B = 0x7F
        run_cb_instruction(cpu, 0x78)  # BIT 7,B
        assert flag_clear(cpu, FLAG_S)

    def test_bit_hl_indirect_set(self, cpu):
        """BIT b,(HL) — bit is set."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x01)  # Bit 0 set
        run_cb_instruction(cpu, 0x46)  # BIT 0,(HL)
        assert flag_clear(cpu, FLAG_Z)

    def test_bit_hl_indirect_clear(self, cpu):
        """BIT b,(HL) — bit is clear."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x00)
        run_cb_instruction(cpu, 0x46)  # BIT 0,(HL)
        assert flag_set(cpu, FLAG_Z)

    @pytest.mark.parametrize("bit", [0, 1, 2, 3, 4, 5, 6, 7])
    def test_bit_hl_all_bits(self, cpu, bit):
        """BIT b,(HL) — test all bits."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 1 << bit)
        opcode = 0x40 | (bit << 3) | 0x06  # BIT b,(HL) = 0x46 + (bit * 8)
        run_cb_instruction(cpu, opcode)
        assert flag_clear(cpu, FLAG_Z)  # Bit is set
        assert cpu.registers.F & FLAG_H


class TestSET:
    """SET b,r - Set bit in register."""
    @pytest.mark.parametrize("bit,reg,opcode", [
        (0, "B", 0xC0), (1, "B", 0xC8), (2, "B", 0xD0), (3, "B", 0xD8),
        (4, "B", 0xE0), (5, "B", 0xE8), (6, "B", 0xF0), (7, "B", 0xF8),
        (0, "C", 0xC1), (7, "C", 0xF9),
        (0, "D", 0xC2), (7, "D", 0xFA),
        (0, "E", 0xC3), (7, "E", 0xFB),
        (0, "H", 0xC4), (7, "H", 0xFC),
        (0, "L", 0xC5), (7, "L", 0xFD),
        (0, "A", 0xC7), (7, "A", 0xFF),
    ])
    def test_set_r(self, cpu, bit, reg, opcode):
        """SET b,r — set bit in register."""
        setattr(cpu.registers, reg, 0x00)
        run_cb_instruction(cpu, opcode)
        assert getattr(cpu.registers, reg) == (1 << bit)

    def test_set_hl_indirect(self, cpu):
        """SET b,(HL) — set bit in memory."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x00)
        run_cb_instruction(cpu, 0xC6)  # SET 0,(HL)
        assert cpu.read_byte(0x2000) == 0x01

    @pytest.mark.parametrize("bit", [0, 1, 2, 3, 4, 5, 6, 7])
    def test_set_hl_all_bits(self, cpu, bit):
        """SET b,(HL) — set all bits in memory."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0x00)
        opcode = 0xC0 | (bit << 3) | 0x06
        run_cb_instruction(cpu, opcode)
        assert cpu.read_byte(0x2000) == (1 << bit)


class TestRES:
    """RES b,r - Reset bit in register."""
    @pytest.mark.parametrize("bit,reg,opcode", [
        (0, "B", 0x80), (1, "B", 0x88), (2, "B", 0x90), (3, "B", 0x98),
        (4, "B", 0xA0), (5, "B", 0xA8), (6, "B", 0xB0), (7, "B", 0xB8),
        (0, "C", 0x81), (7, "C", 0xB9),
        (0, "D", 0x82), (7, "D", 0xBA),
        (0, "E", 0x83), (7, "E", 0xBB),
        (0, "H", 0x84), (7, "H", 0xBC),
        (0, "L", 0x85), (7, "L", 0xBD),
        (0, "A", 0x87), (7, "A", 0xBF),
    ])
    def test_res_r(self, cpu, bit, reg, opcode):
        """RES b,r — reset bit in register."""
        setattr(cpu.registers, reg, 0xFF)
        run_cb_instruction(cpu, opcode)
        assert getattr(cpu.registers, reg) == (0xFF & ~(1 << bit))

    def test_res_hl_indirect(self, cpu):
        """RES b,(HL) — reset bit in memory."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0xFF)
        run_cb_instruction(cpu, 0x86)  # RES 0,(HL)
        assert cpu.read_byte(0x2000) == 0xFE

    @pytest.mark.parametrize("bit", [0, 1, 2, 3, 4, 5, 6, 7])
    def test_res_hl_all_bits(self, cpu, bit):
        """RES b,(HL) — reset all bits in memory."""
        cpu.registers.HL = 0x2000
        cpu.write_byte(0x2000, 0xFF)
        opcode = 0x80 | (bit << 3) | 0x06
        run_cb_instruction(cpu, opcode)
        assert cpu.read_byte(0x2000) == (0xFF & ~(1 << bit))


class TestBITFlags:
    """Verify flags for BIT instructions."""
    def test_bit_sets_h(self, cpu):
        """BIT always sets H flag."""
        cpu.registers.B = 0x01
        run_cb_instruction(cpu, 0x40)  # BIT 0,B
        assert cpu.registers.F & FLAG_H

    def test_bit_clears_n(self, cpu):
        """BIT always clears N flag."""
        cpu.registers.F = FLAG_N
        cpu.registers.B = 0x01
        run_cb_instruction(cpu, 0x40)
        assert flag_clear(cpu, FLAG_N)

    def test_bit_preserves_c(self, cpu):
        """BIT preserves C flag."""
        cpu.registers.F = FLAG_C
        cpu.registers.B = 0x01
        run_cb_instruction(cpu, 0x40)
        assert cpu.registers.F & FLAG_C

    def test_bit_sign_flag(self, cpu):
        """BIT 7 sets S flag based on bit 7 value."""
        cpu.registers.B = 0x80
        run_cb_instruction(cpu, 0x78)  # BIT 7,B
        assert cpu.registers.F & FLAG_S

        cpu.registers.B = 0x7F
        run_cb_instruction(cpu, 0x78)
        assert flag_clear(cpu, FLAG_S)

    def test_bit_pv_equals_z(self, cpu):
        """BIT sets PV = Z (PV mirrors the Z flag)."""
        cpu.registers.B = 0x01  # Bit 0 set
        run_cb_instruction(cpu, 0x40)  # BIT 0,B
        assert flag_clear(cpu, FLAG_PV)  # PV = 0 (Z is 0, bit is set)

        cpu.registers.B = 0x00  # Bit 0 clear
        run_cb_instruction(cpu, 0x40)
        assert flag_set(cpu, FLAG_PV)  # PV = 1 (Z is 1, bit is clear)


class TestSETRESFlags:
    """Verify flags for SET/RES instructions."""
    def test_set_does_not_affect_flags(self, cpu):
        """SET does not affect flags."""
        cpu.registers.F = 0x00
        cpu.registers.B = 0x00
        run_cb_instruction(cpu, 0xC0)  # SET 0,B
        assert cpu.registers.F == 0x00

    def test_res_does_not_affect_flags(self, cpu):
        """RES does not affect flags."""
        cpu.registers.F = 0x00
        cpu.registers.B = 0xFF
        run_cb_instruction(cpu, 0x80)  # RES 0,B
        assert cpu.registers.F == 0x00


class TestBITSETRESCycles:
    """Verify cycle counts for BIT/SET/RES instructions."""
    def test_bit_r_cycles(self, cpu):
        """BIT b,r takes 8 cycles."""
        cpu.registers.B = 0x01
        assert run_cb_instruction(cpu, 0x40) == 8

    def test_bit_hl_cycles(self, cpu):
        """BIT b,(HL) takes 12 cycles."""
        cpu.registers.HL = 0x2000
        assert run_cb_instruction(cpu, 0x46) == 12

    def test_set_r_cycles(self, cpu):
        """SET b,r takes 8 cycles."""
        cpu.registers.B = 0x00
        assert run_cb_instruction(cpu, 0xC0) == 8

    def test_set_hl_cycles(self, cpu):
        """SET b,(HL) takes 15 cycles."""
        cpu.registers.HL = 0x2000
        assert run_cb_instruction(cpu, 0xC6) == 15

    def test_res_r_cycles(self, cpu):
        """RES b,r takes 8 cycles."""
        cpu.registers.B = 0xFF
        assert run_cb_instruction(cpu, 0x80) == 8

    def test_res_hl_cycles(self, cpu):
        """RES b,(HL) takes 15 cycles."""
        cpu.registers.HL = 0x2000
        assert run_cb_instruction(cpu, 0x86) == 15
