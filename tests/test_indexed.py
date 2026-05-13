#!/usr/bin/env python3
"""Indexed (IX/IY) instruction tests."""

from conftest import (
    write_program,
)


class TestIndexed:
    def test_ld_ix_nn(self, cpu):
        write_program(cpu, [0xDD, 0x21, 0x34, 0x12])
        cpu.step()
        assert cpu.registers.IX == 0x1234

    def test_ld_iy_nn(self, cpu):
        write_program(cpu, [0xFD, 0x21, 0x34, 0x12])
        cpu.step()
        assert cpu.registers.IY == 0x1234

    def test_ld_nn_ix(self, cpu):
        cpu.registers.IX = 0x1234
        write_program(cpu, [0xDD, 0x22, 0x00, 0x10])
        cpu.step()
        assert cpu.read_byte(0x1000) == 0x34
        assert cpu.read_byte(0x1001) == 0x12

    def test_ld_ix_nn_ind(self, cpu):
        cpu.write_byte(0x1000, 0x34)
        cpu.write_byte(0x1001, 0x12)
        write_program(cpu, [0xDD, 0x2A, 0x00, 0x10])
        cpu.step()
        assert cpu.registers.IX == 0x1234

    def test_ld_a_ixd(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0xAB)
        write_program(cpu, [0xDD, 0x7E, 0x10])
        cpu.step()
        assert cpu.registers.A == 0xAB

    def test_ld_ixd_a(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.registers.A = 0xAB
        write_program(cpu, [0xDD, 0x77, 0x10])
        cpu.step()
        assert cpu.read_byte(0x1010) == 0xAB

    def test_ld_ixd_n(self, cpu):
        cpu.registers.IX = 0x1000
        write_program(cpu, [0xDD, 0x36, 0x10, 0xAB])
        cpu.step()
        assert cpu.read_byte(0x1010) == 0xAB

    def test_ld_b_ixd(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0xAB)
        write_program(cpu, [0xDD, 0x46, 0x10])
        cpu.step()
        assert cpu.registers.B == 0xAB

    def test_ld_ixd_b(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.registers.B = 0xAB
        write_program(cpu, [0xDD, 0x70, 0x10])
        cpu.step()
        assert cpu.read_byte(0x1010) == 0xAB

    def test_inc_ix(self, cpu):
        cpu.registers.IX = 0x1234
        write_program(cpu, [0xDD, 0x23])
        cpu.step()
        assert cpu.registers.IX == 0x1235

    def test_dec_ix(self, cpu):
        cpu.registers.IX = 0x1234
        write_program(cpu, [0xDD, 0x2B])
        cpu.step()
        assert cpu.registers.IX == 0x1233

    def test_ld_sp_ix(self, cpu):
        cpu.registers.IX = 0x1234
        write_program(cpu, [0xDD, 0xF9])
        cpu.step()
        assert cpu.registers.SP == 0x1234

    def test_push_ix(self, cpu):
        cpu.registers.SP = 0x2000
        cpu.registers.IX = 0xDEAD
        write_program(cpu, [0xDD, 0xE5])
        cpu.step()
        assert cpu.read_byte(0x1FFF) == 0xDE
        assert cpu.read_byte(0x1FFE) == 0xAD
        assert cpu.registers.SP == 0x1FFE

    def test_pop_ix(self, cpu):
        cpu.registers.SP = 0x2000
        cpu.write_byte(0x2000, 0xAD)
        cpu.write_byte(0x2001, 0xDE)
        write_program(cpu, [0xDD, 0xE1])
        cpu.step()
        assert cpu.registers.IX == 0xDEAD
        assert cpu.registers.SP == 0x2002

    def test_add_ix_bc(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.registers.BC = 0x0100
        write_program(cpu, [0xDD, 0x09])
        cpu.step()
        assert cpu.registers.IX == 0x1100

    def test_add_ix_ix(self, cpu):
        cpu.registers.IX = 0x1000
        write_program(cpu, [0xDD, 0x29])
        cpu.step()
        assert cpu.registers.IX == 0x2000

    def test_add_ix_sp(self, cpu):
        cpu.registers.IX = 0x1000
        cpu.registers.SP = 0x0100
        write_program(cpu, [0xDD, 0x39])
        cpu.step()
        assert cpu.registers.IX == 0x1100

    def test_jp_ix(self, cpu):
        cpu.registers.IX = 0x4000
        write_program(cpu, [0xDD, 0xE9])
        cpu.step()
        assert cpu.registers.PC == 0x4000

    def test_jp_iy(self, cpu):
        cpu.registers.IY = 0x5000
        write_program(cpu, [0xFD, 0xE9])
        cpu.step()
        assert cpu.registers.PC == 0x5000

    def test_ex_sp_ix(self, cpu):
        cpu.registers.SP = 0x1000
        cpu.registers.IX = 0x1234
        cpu.write_byte(0x1000, 0x78)
        cpu.write_byte(0x1001, 0x56)
        write_program(cpu, [0xDD, 0xE3])
        cpu.step()
        assert cpu.registers.IX == 0x5678
        assert cpu.read_byte(0x1000) == 0x34
        assert cpu.read_byte(0x1001) == 0x12

    def test_inc_ixh(self, cpu):
        cpu.registers.IX = 0x1200
        write_program(cpu, [0xDD, 0x24])
        cpu.step()
        assert cpu.registers.IX == 0x1300

    def test_dec_ixh(self, cpu):
        cpu.registers.IX = 0x1200
        write_program(cpu, [0xDD, 0x25])
        cpu.step()
        assert cpu.registers.IX == 0x1100

    def test_ld_ixh_n(self, cpu):
        write_program(cpu, [0xDD, 0x26, 0xAB])
        cpu.step()
        assert (cpu.registers.IX >> 8) == 0xAB

    def test_ld_ixl_n(self, cpu):
        write_program(cpu, [0xDD, 0x2E, 0xCD])
        cpu.step()
        assert (cpu.registers.IX & 0xFF) == 0xCD

    def test_ld_b_ixh(self, cpu):
        cpu.registers.IX = 0xAB00
        write_program(cpu, [0xDD, 0x44])
        cpu.step()
        assert cpu.registers.B == 0xAB

    def test_ld_b_ixl(self, cpu):
        cpu.registers.IX = 0x00CD
        write_program(cpu, [0xDD, 0x45])
        cpu.step()
        assert cpu.registers.B == 0xCD

    def test_ld_ixh_b(self, cpu):
        cpu.registers.IX = 0x0000
        cpu.registers.B = 0xAB
        write_program(cpu, [0xDD, 0x60])
        cpu.step()
        assert (cpu.registers.IX >> 8) == 0xAB

    def test_ld_ixl_b(self, cpu):
        cpu.registers.IX = 0x0000
        cpu.registers.B = 0xCD
        write_program(cpu, [0xDD, 0x68])
        cpu.step()
        assert (cpu.registers.IX & 0xFF) == 0xCD

    def test_ld_a_ixh(self, cpu):
        cpu.registers.IX = 0xAB00
        write_program(cpu, [0xDD, 0x7C])
        cpu.step()
        assert cpu.registers.A == 0xAB

    def test_ld_a_ixl(self, cpu):
        cpu.registers.IX = 0x00CD
        write_program(cpu, [0xDD, 0x7D])
        cpu.step()
        assert cpu.registers.A == 0xCD

    def test_ld_ixh_ixl(self, cpu):
        # Under DD prefix: 0xDD 0x65 = LD IXH, IXL (NOT LD IXH, L)
        # Register codes 4/5 always redirect to IXH/IXL under DD/FD prefix
        cpu.registers.IX = 0x1234  # IXH=0x12, IXL=0x34
        cpu.registers.L = 0x56
        write_program(cpu, [0xDD, 0x65])
        cpu.step()
        # IXH ← IXL = 0x34, IXL unchanged = 0x34
        assert cpu.registers.IX == 0x3434

    def test_ld_ixl_ixh(self, cpu):
        # Under DD prefix: 0xDD 0x6C = LD IXL, IXH (NOT LD IXL, H)
        cpu.registers.IX = 0x1234  # IXH=0x12, IXL=0x34
        cpu.registers.H = 0x78
        write_program(cpu, [0xDD, 0x6C])
        cpu.step()
        # IXL ← IXH = 0x12, IXH unchanged = 0x12
        assert cpu.registers.IX == 0x1212

    def test_add_a_ixh(self, cpu):
        cpu.registers.A = 0x10
        cpu.registers.IX = 0x2000
        write_program(cpu, [0xDD, 0x84])
        cpu.step()
        assert cpu.registers.A == 0x30

    def test_add_a_ixl(self, cpu):
        cpu.registers.A = 0x10
        cpu.registers.IX = 0x0020
        write_program(cpu, [0xDD, 0x85])
        cpu.step()
        assert cpu.registers.A == 0x30

    def test_add_a_ixd(self, cpu):
        cpu.registers.A = 0x10
        cpu.registers.IX = 0x1000
        cpu.write_byte(0x1010, 0x20)
        write_program(cpu, [0xDD, 0x86, 0x10])
        cpu.step()
        assert cpu.registers.A == 0x30

    def test_iy_all_operations(self, cpu):
        cpu.registers.IY = 0x1000
        cpu.write_byte(0x1010, 0xAB)
        write_program(cpu, [0xFD, 0x7E, 0x10])
        cpu.step()
        assert cpu.registers.A == 0xAB

    def test_dd_fallthrough(self, cpu):
        write_program(cpu, [0xDD, 0x00])
        cpu.step()
        assert cpu.registers.PC == 2

    def test_fd_fallthrough(self, cpu):
        write_program(cpu, [0xFD, 0x00])
        cpu.step()
        assert cpu.registers.PC == 2

    # Note: DD ED xx and FD ED xx opcodes do NOT exist on Z80!
    # The DD/FD prefix is ignored for ED opcodes - they execute the ED instruction
    # These tests are removed as they tested non-existent instructions
