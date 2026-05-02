#!/usr/bin/env python3
"""Comprehensive 16-bit load and arithmetic instruction tests."""
import pytest
from conftest import (
    write_program, FLAG_S, FLAG_Z, FLAG_H, FLAG_PV, FLAG_N, FLAG_C
)


class TestLoad16BitImmediate:
    """LD rr,nn - Load 16-bit immediate."""
    @pytest.mark.parametrize("reg,opcode,setter", [
        ("BC", 0x01, lambda c,v: setattr(c.regs, 'BC', v)),
        ("DE", 0x11, lambda c,v: setattr(c.regs, 'DE', v)),
        ("HL", 0x21, lambda c,v: setattr(c.regs, 'HL', v)),
        ("SP", 0x31, lambda c,v: setattr(c.regs, 'SP', v)),
    ])
    def test_ld_rr_nn(self, cpu, reg, opcode, setter):
        """LD rr,nn — load 16-bit immediate."""
        write_program(cpu, [opcode, 0x34, 0x12])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0x1234

    @pytest.mark.parametrize("reg,opcode", [
        ("BC", 0x01), ("DE", 0x11), ("HL", 0x21), ("SP", 0x31),
    ])
    def test_ld_rr_nn_zero(self, cpu, reg, opcode):
        """LD rr,nn — load 0x0000."""
        write_program(cpu, [opcode, 0x00, 0x00])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0x0000

    @pytest.mark.parametrize("reg,opcode", [
        ("BC", 0x01), ("DE", 0x11), ("HL", 0x21), ("SP", 0x31),
    ])
    def test_ld_rr_nn_ffff(self, cpu, reg, opcode):
        """LD rr,nn — load 0xFFFF."""
        write_program(cpu, [opcode, 0xFF, 0xFF])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0xFFFF


class TestLoadSPHL:
    """LD SP,HL - Copy HL to SP."""
    def test_ld_sp_hl(self, cpu):
        """LD SP,HL — copy HL to SP."""
        cpu.registers.HL = 0xDEAD
        write_program(cpu, [0xF9])
        cpu.step()
        assert cpu.registers.SP == 0xDEAD

    def test_ld_sp_ix(self, cpu):
        """LD SP,IX — copy IX to SP (DD prefix)."""
        cpu.registers.IX = 0x1234
        write_program(cpu, [0xDD, 0xF9])
        cpu.step()
        assert cpu.registers.SP == 0x1234

    def test_ld_sp_iy(self, cpu):
        """LD SP,IY — copy IY to SP (FD prefix)."""
        cpu.registers.IY = 0x5678
        write_program(cpu, [0xFD, 0xF9])
        cpu.step()
        assert cpu.registers.SP == 0x5678


class TestLoad16BitIndirect:
    """LD rr,(nn) and LD (nn),rr - Load/store via absolute address."""
    def test_ld_hl_nn_indirect(self, cpu):
        """LD HL,(nn) — load HL from absolute address."""
        cpu.write_byte(0x1000, 0x78)
        cpu.write_byte(0x1001, 0x56)
        write_program(cpu, [0x2A, 0x00, 0x10])
        cpu.step()
        assert cpu.registers.HL == 0x5678

    def test_ld_nn_hl(self, cpu):
        """LD (nn),HL — store HL to absolute address."""
        cpu.registers.HL = 0xBEEF
        write_program(cpu, [0x22, 0x00, 0x20])
        cpu.step()
        assert cpu.read_byte(0x2000) == 0xEF
        assert cpu.read_byte(0x2001) == 0xBE

    def test_ld_bc_nn_indirect_ed(self, cpu):
        """LD BC,(nn) — ED prefix."""
        cpu.write_byte(0x3000, 0x21)
        cpu.write_byte(0x3001, 0x43)
        write_program(cpu, [0xED, 0x4B, 0x00, 0x30])
        cpu.step()
        assert cpu.registers.BC == 0x4321

    def test_ld_de_nn_indirect_ed(self, cpu):
        """LD DE,(nn) — ED prefix."""
        cpu.write_byte(0x4000, 0x65)
        cpu.write_byte(0x4001, 0x87)
        write_program(cpu, [0xED, 0x5B, 0x00, 0x40])
        cpu.step()
        assert cpu.registers.DE == 0x8765

    def test_ld_hl_nn_indirect_ed(self, cpu):
        """LD HL,(nn) — ED prefix."""
        cpu.write_byte(0x5000, 0xAA)
        cpu.write_byte(0x5001, 0xBB)
        write_program(cpu, [0xED, 0x6B, 0x00, 0x50])
        cpu.step()
        assert cpu.registers.HL == 0xBBAA

    def test_ld_sp_nn_indirect_ed(self, cpu):
        """LD SP,(nn) — ED prefix."""
        cpu.write_byte(0x6000, 0x11)
        cpu.write_byte(0x6001, 0x22)
        write_program(cpu, [0xED, 0x7B, 0x00, 0x60])
        cpu.step()
        assert cpu.registers.SP == 0x2211

    def test_ld_nn_bc_ed(self, cpu):
        """LD (nn),BC — ED prefix."""
        cpu.registers.BC = 0x1234
        write_program(cpu, [0xED, 0x43, 0x00, 0x70])
        cpu.step()
        assert cpu.read_byte(0x7000) == 0x34
        assert cpu.read_byte(0x7001) == 0x12

    def test_ld_nn_de_ed(self, cpu):
        """LD (nn),DE — ED prefix."""
        cpu.registers.DE = 0x5678
        write_program(cpu, [0xED, 0x53, 0x00, 0x80])
        cpu.step()
        assert cpu.read_byte(0x8000) == 0x78
        assert cpu.read_byte(0x8001) == 0x56

    def test_ld_nn_hl_ed(self, cpu):
        """LD (nn),HL — ED prefix."""
        cpu.registers.HL = 0x9ABC
        write_program(cpu, [0xED, 0x63, 0x00, 0x90])
        cpu.step()
        assert cpu.read_byte(0x9000) == 0xBC
        assert cpu.read_byte(0x9001) == 0x9A

    def test_ld_nn_sp_ed(self, cpu):
        """LD (nn),SP — ED prefix."""
        cpu.registers.SP = 0xDEAD
        write_program(cpu, [0xED, 0x73, 0x00, 0xA0])
        cpu.step()
        assert cpu.read_byte(0xA000) == 0xAD
        assert cpu.read_byte(0xA001) == 0xDE


class TestLoad16BitFlags:
    """Verify 16-bit loads don't affect flags."""
    def test_ld_rr_nn_preserves_flags(self, cpu):
        """LD rr,nn does not affect flags."""
        cpu.registers.F = 0xFF
        write_program(cpu, [0x21, 0x00, 0x10])
        cpu.step()
        assert cpu.registers.F == 0xFF

    def test_ld_hl_indirect_preserves_flags(self, cpu):
        """LD HL,(nn) does not affect flags."""
        cpu.registers.F = 0xFF
        cpu.write_byte(0x1000, 0x34)
        cpu.write_byte(0x1001, 0x12)
        write_program(cpu, [0x2A, 0x00, 0x10])
        cpu.step()
        assert cpu.registers.F == 0xFF


class TestAdd16Bit:
    """ADD HL,rr and ADD IX/IY,rr - 16-bit addition."""
    @pytest.mark.parametrize("opcode,reg_name", [
        (0x09, "BC"), (0x19, "DE"), (0x29, "HL"), (0x39, "SP"),
    ])
    def test_add_hl_rr(self, cpu, opcode, reg_name):
        """ADD HL,rr — 16-bit addition to HL."""
        cpu.registers.HL = 0x1000
        if reg_name != "HL":
            setattr(cpu.registers, reg_name, 0x2000)
            expected = 0x3000
        else:
            expected = 0x2000  # HL = 0x1000, ADD HL,HL = 0x2000
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.registers.HL == expected
        assert not (cpu.registers.F & FLAG_N)

    def test_add_hl_rr_carry(self, cpu):
        """ADD HL,BC — with carry set."""
        cpu.registers.HL = 0x8000
        cpu.registers.BC = 0x8000
        write_program(cpu, [0x09])
        cpu.step()
        assert cpu.registers.HL == 0x0000
        assert cpu.registers.F & FLAG_C

    def test_add_hl_rr_half_carry(self, cpu):
        """ADD HL,DE — half carry from bit 11."""
        cpu.registers.HL = 0x0800
        cpu.registers.DE = 0x0800
        write_program(cpu, [0x19])
        cpu.step()
        assert cpu.registers.F & FLAG_H

    def test_add_hl_rr_flags(self, cpu):
        """ADD HL,rr preserves S,Z,PV flags."""
        cpu.registers.F = FLAG_S | FLAG_Z | FLAG_PV
        cpu.registers.HL = 0x1000
        cpu.registers.BC = 0x2000
        write_program(cpu, [0x09])
        cpu.step()
        # S, Z, PV should be preserved
        assert cpu.registers.F & (FLAG_S | FLAG_Z | FLAG_PV)
        # 0x1000 + 0x2000 = 0x3000 (no carry from bit 11)
        assert not (cpu.registers.F & FLAG_H)
        assert not (cpu.registers.F & FLAG_N)  # N always cleared

    # ADD IX/IY,rr
    @pytest.mark.parametrize("prefix,opcode,rr,ix_reg,expected", [
        (0xDD, 0x09, "BC", "IX", 0x3000), (0xDD, 0x19, "DE", "IX", 0x3000),
        (0xDD, 0x29, "IX", "IX", 0x2000), (0xDD, 0x39, "SP", "IX", 0x3000),
        (0xFD, 0x09, "BC", "IY", 0x3000), (0xFD, 0x19, "DE", "IY", 0x3000),
        (0xFD, 0x29, "IY", "IY", 0x2000), (0xFD, 0x39, "SP", "IY", 0x3000),
    ])
    def test_add_ix_iy_rr(self, cpu, prefix, opcode, rr, ix_reg, expected):
        """ADD IX/IY,rr — 16-bit index addition."""
        # Set source register first, then IX/IY
        if rr in ["IX", "IY"]:
             setattr(cpu.registers, ix_reg, 0x1000)
        else:
             setattr(cpu.registers, rr, 0x2000)
             setattr(cpu.registers, ix_reg, 0x1000)
             
        write_program(cpu, [prefix, opcode])
        cpu.step()
        assert getattr(cpu.registers, ix_reg) == expected


class TestAdcSbc16Bit:
    """ADC HL,rr and SBC HL,rr (ED prefix) - 16-bit add/sub with carry."""
    @pytest.mark.parametrize("opcode,reg,expected", [
        (0x4A, "BC", 0x3001), (0x5A, "DE", 0x3001), 
        (0x6A, "HL", 0x2001),  # ADC HL,HL = 0x1000 + 0x1000 + 1
        (0x7A, "SP", 0x3001),
    ])
    def test_adc_hl_rr(self, cpu, opcode, reg, expected):
        """ADC HL,rr — 16-bit add with carry."""
        cpu.registers.HL = 0x1000
        if reg != "HL":
            setattr(cpu.registers, reg, 0x2000)
        cpu.registers.F = FLAG_C  # Set carry
        write_program(cpu, [0xED, opcode])
        cpu.step()
        assert cpu.registers.HL == expected

    @pytest.mark.parametrize("opcode,reg,expected", [
        (0x42, "BC", 0x1FFF), (0x52, "DE", 0x1FFF),
        (0x62, "HL", 0xFFFF),  # SBC HL,HL = 0x3000 - 0x3000 - 1 = 0xFFFF
        (0x72, "SP", 0x1FFF),
    ])
    def test_sbc_hl_rr(self, cpu, opcode, reg, expected):
        """SBC HL,rr — 16-bit sub with carry."""
        cpu.registers.HL = 0x3000
        if reg != "HL":
            setattr(cpu.registers, reg, 0x1000)
        cpu.registers.F = FLAG_C  # Set carry
        write_program(cpu, [0xED, opcode])
        cpu.step()
        assert cpu.registers.HL == expected

    def test_adc_hl_rr_no_carry(self, cpu):
        """ADC HL,BC — without carry."""
        cpu.registers.HL = 0x1000
        cpu.registers.BC = 0x2000
        cpu.registers.F = 0  # No carry
        write_program(cpu, [0xED, 0x4A])
        cpu.step()
        assert cpu.registers.HL == 0x3000

    def test_sbc_hl_rr_no_carry(self, cpu):
        """SBC HL,BC — without carry."""
        cpu.registers.HL = 0x3000
        cpu.registers.BC = 0x1000
        cpu.registers.F = 0  # No carry
        write_program(cpu, [0xED, 0x42])
        cpu.step()
        assert cpu.registers.HL == 0x2000


class TestIncDec16Bit:
    """INC ss and DEC ss - 16-bit increment/decrement."""
    @pytest.mark.parametrize("opcode,reg", [
        (0x03, "BC"), (0x13, "DE"), (0x23, "HL"), (0x33, "SP"),
    ])
    def test_inc_rr(self, cpu, opcode, reg):
        """INC ss — 16-bit increment."""
        setattr(cpu.registers, reg, 0x1234)
        write_program(cpu, [opcode])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0x1235

    @pytest.mark.parametrize("opcode,reg", [
        (0x03, "BC"), (0x13, "DE"), (0x23, "HL"), (0x33, "SP"),
    ])
    def test_inc_rr_wrap(self, cpu, opcode, reg):
        """INC ss — wraps from 0xFFFF to 0x0000."""
        setattr(cpu.registers, reg, 0xFFFF)
        write_program(cpu, [opcode])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0x0000

    @pytest.mark.parametrize("opcode,reg", [
        (0x0B, "BC"), (0x1B, "DE"), (0x2B, "HL"), (0x3B, "SP"),
    ])
    def test_dec_rr(self, cpu, opcode, reg):
        """DEC ss — 16-bit decrement."""
        setattr(cpu.registers, reg, 0x1234)
        write_program(cpu, [opcode])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0x1233

    @pytest.mark.parametrize("opcode,reg", [
        (0x0B, "BC"), (0x1B, "DE"), (0x2B, "HL"), (0x3B, "SP"),
    ])
    def test_dec_rr_wrap(self, cpu, opcode, reg):
        """DEC ss — wraps from 0x0000 to 0xFFFF."""
        setattr(cpu.registers, reg, 0x0000)
        write_program(cpu, [opcode])
        cpu.step()
        assert getattr(cpu.registers, reg) == 0xFFFF

    def test_inc_rr_preserves_flags(self, cpu):
        """INC ss does not modify any flags."""
        cpu.registers.F = FLAG_Z | FLAG_C
        cpu.registers.BC = 0x0001
        write_program(cpu, [0x03])
        cpu.step()
        assert cpu.registers.F == (FLAG_Z | FLAG_C)

    def test_dec_rr_preserves_flags(self, cpu):
        """DEC ss does not modify any flags."""
        cpu.registers.F = FLAG_Z | FLAG_C
        cpu.registers.BC = 0x0001
        write_program(cpu, [0x0B])
        cpu.step()
        assert cpu.registers.F == (FLAG_Z | FLAG_C)

    # INC/DEC IX/IY
    @pytest.mark.parametrize("prefix,opcode,ix_reg", [
        (0xDD, 0x23, "IX"), (0xDD, 0x2B, "IX"),
        (0xFD, 0x23, "IY"), (0xFD, 0x2B, "IY"),
    ])
    def test_inc_dec_ix_iy(self, cpu, prefix, opcode, ix_reg):
        """INC/DEC IX/IY."""
        initial = 0x1000
        setattr(cpu.registers, ix_reg, initial)
        write_program(cpu, [prefix, opcode])
        cpu.step()
        if opcode == 0x23:  # INC
            assert getattr(cpu.registers, ix_reg) == initial + 1
        else:  # DEC
            assert getattr(cpu.registers, ix_reg) == initial - 1


class Test16BitCycles:
    """Verify cycle counts for 16-bit instructions."""
    def test_ld_rr_nn_cycles(self, cpu):
        """LD rr,nn takes 10 cycles."""
        write_program(cpu, [0x21, 0x00, 0x10])
        assert cpu.step() == 10

    def test_ld_hl_nn_indirect_cycles(self, cpu):
        """LD HL,(nn) takes 16 cycles."""
        write_program(cpu, [0x2A, 0x00, 0x10])
        assert cpu.step() == 16

    def test_ld_nn_hl_cycles(self, cpu):
        """LD (nn),HL takes 16 cycles."""
        write_program(cpu, [0x22, 0x00, 0x20])
        assert cpu.step() == 16

    def test_add_hl_rr_cycles(self, cpu):
        """ADD HL,rr takes 11 cycles."""
        write_program(cpu, [0x09])
        assert cpu.step() == 11

    def test_inc_rr_cycles(self, cpu):
        """INC ss takes 6 cycles."""
        write_program(cpu, [0x03])
        assert cpu.step() == 6

    def test_dec_rr_cycles(self, cpu):
        """DEC ss takes 6 cycles."""
        write_program(cpu, [0x0B])
        assert cpu.step() == 6

    def test_ld_rr_nn_indirect_ed_cycles(self, cpu):
        """LD rr,(nn) ED takes 20 cycles."""
        write_program(cpu, [0xED, 0x4B, 0x00, 0x10])
        assert cpu.step() == 20

    def test_ld_nn_rr_ed_cycles(self, cpu):
        """LD (nn),rr ED takes 20 cycles."""
        write_program(cpu, [0xED, 0x43, 0x00, 0x10])
        assert cpu.step() == 20
