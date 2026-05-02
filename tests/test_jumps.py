#!/usr/bin/env python3
"""Comprehensive jump, call, and return instruction tests."""
import pytest
from conftest import write_program, FLAG_Z, FLAG_C, FLAG_PV, FLAG_S


class TestJp:
    """JP nn - Unconditional jump."""
    def test_jp_nn(self, cpu):
        """JP nn — unconditional jump."""
        write_program(cpu, [0xC3, 0x00, 0x20])
        cpu.step()
        assert cpu.registers.PC == 0x2000

    def test_jp_nn_zero(self, cpu):
        """JP 0x0000."""
        write_program(cpu, [0xC3, 0x00, 0x00])
        cpu.step()
        assert cpu.registers.PC == 0x0000

    def test_jp_nn_ffff(self, cpu):
        """JP 0xFFFF."""
        write_program(cpu, [0xC3, 0xFF, 0xFF])
        cpu.step()
        assert cpu.registers.PC == 0xFFFF


class TestJpConditional:
    """JP cc,nn - Conditional jumps."""
    @pytest.mark.parametrize("opcode,flag,flag_val,taken", [
        (0xC2, FLAG_Z, 0, True), (0xC2, FLAG_Z, FLAG_Z, False),
        (0xCA, FLAG_Z, FLAG_Z, True), (0xCA, FLAG_Z, 0, False),
        (0xD2, FLAG_C, 0, True), (0xD2, FLAG_C, FLAG_C, False),
        (0xDA, FLAG_C, FLAG_C, True), (0xDA, FLAG_C, 0, False),
        (0xE2, FLAG_PV, 0, True), (0xE2, FLAG_PV, FLAG_PV, False),
        (0xEA, FLAG_PV, FLAG_PV, True), (0xEA, FLAG_PV, 0, False),
        (0xF2, FLAG_S, 0, True), (0xF2, FLAG_S, FLAG_S, False),
        (0xFA, FLAG_S, FLAG_S, True), (0xFA, FLAG_S, 0, False),
    ])
    def test_jp_cc_nn(self, cpu, opcode, flag, flag_val, taken):
        """JP cc,nn — conditional jump."""
        cpu.registers.F = flag_val
        write_program(cpu, [opcode, 0x00, 0x10])
        cpu.step()
        if taken:
            assert cpu.registers.PC == 0x1000
        else:
            assert cpu.registers.PC == 3


class TestJpHLIXIY:
    """JP (HL), JP (IX), JP (IY) - Jump to address in register."""
    def test_jp_hl(self, cpu):
        """JP (HL) — jump to address in HL."""
        cpu.registers.HL = 0x3000
        write_program(cpu, [0xE9])
        cpu.step()
        assert cpu.registers.PC == 0x3000

    def test_jp_ix(self, cpu):
        """JP (IX) — jump to address in IX."""
        cpu.registers.IX = 0x4000
        write_program(cpu, [0xDD, 0xE9])
        cpu.step()
        assert cpu.registers.PC == 0x4000

    def test_jp_iy(self, cpu):
        """JP (IY) — jump to address in IY."""
        cpu.registers.IY = 0x5000
        write_program(cpu, [0xFD, 0xE9])
        cpu.step()
        assert cpu.registers.PC == 0x5000


class TestJr:
    """JR e - Unconditional relative jump."""
    def test_jr_forward(self, cpu):
        """JR +4 (forward)."""
        write_program(cpu, [0x18, 0x04])
        cpu.step()
        assert cpu.registers.PC == 6  # PC = 2 + 4

    def test_jr_backward(self, cpu):
        """JR -2 (backward)."""
        write_program(cpu, [0x18, 0xFE], 0x0010)
        cpu.step()
        assert cpu.registers.PC == 0x0010  # PC = 0x10 + 2 - 2

    def test_jr_zero(self, cpu):
        """JR 0 (no effect)."""
        write_program(cpu, [0x18, 0x00])
        cpu.step()
        assert cpu.registers.PC == 2


class TestJrConditional:
    """JR cc,e - Conditional relative jumps."""
    @pytest.mark.parametrize("opcode,flag_val,taken", [
        (0x20, 0, True), (0x20, FLAG_Z, False),  # JR NZ
        (0x28, FLAG_Z, True), (0x28, 0, False),  # JR Z
        (0x30, 0, True), (0x30, FLAG_C, False),  # JR NC
        (0x38, FLAG_C, True), (0x38, 0, False),  # JR C
    ])
    def test_jr_cc_e(self, cpu, opcode, flag_val, taken):
        """JR cc,e — conditional relative jump."""
        cpu.registers.F = flag_val
        write_program(cpu, [opcode, 0x04])
        cpu.step()
        if taken:
            assert cpu.registers.PC == 6  # PC = 2 + 4
        else:
            assert cpu.registers.PC == 2


class TestDjnz:
    """DJNZ e - Decrement B and jump if non-zero."""
    def test_djnz_branch(self, cpu):
        """DJNZ — B non-zero, branch taken."""
        cpu.registers.B = 2
        write_program(cpu, [0x10, 0x00])
        cpu.step()
        assert cpu.registers.B == 1
        assert cpu.registers.PC == 2

    def test_djnz_no_branch(self, cpu):
        """DJNZ — B becomes zero, branch not taken."""
        cpu.registers.B = 1
        write_program(cpu, [0x10, 0x04])
        cpu.step()
        assert cpu.registers.B == 0
        assert cpu.registers.PC == 2

    def test_djnz_backward_loop(self, cpu):
        """DJNZ — backward loop."""
        cpu.registers.B = 3
        write_program(cpu, [0x10, 0xFE], 0x0010)
        cpu.step()
        assert cpu.registers.B == 2
        assert cpu.registers.PC == 0x0010

    def test_djnz_count_to_zero(self, cpu):
        """DJNZ — count down to zero."""
        cpu.registers.B = 3
        write_program(cpu, [0x10, 0xFE])
        cpu.step()
        assert cpu.registers.B == 2
        cpu.step()
        assert cpu.registers.B == 1
        cpu.step()
        assert cpu.registers.B == 0
        assert cpu.registers.PC == 2

    def test_djnz_preserves_flags(self, cpu):
        """DJNZ — preserves flags."""
        cpu.registers.F = 0xFF
        cpu.registers.B = 2
        write_program(cpu, [0x10, 0x00])
        cpu.step()
        assert cpu.registers.F == 0xFF


class TestCall:
    """CALL nn - Unconditional call."""
    def test_call_nn(self, cpu):
        """CALL nn — unconditional call."""
        cpu.registers.SP = 0xFFFF
        write_program(cpu, [0xCD, 0x00, 0x10])
        cpu.step()
        assert cpu.registers.PC == 0x1000
        assert cpu.registers.SP == 0xFFFD
        assert cpu.read_byte(0xFFFE) == 0x00
        assert cpu.read_byte(0xFFFD) == 0x03

    def test_call_nn_sp_wrap(self, cpu):
        """CALL with SP wrap."""
        cpu.registers.SP = 0x0000
        write_program(cpu, [0xCD, 0x00, 0x10])
        cpu.step()
        assert cpu.registers.SP == 0xFFFE


class TestCallConditional:
    """CALL cc,nn - Conditional calls."""
    @pytest.mark.parametrize("opcode,flag_val,taken", [
        (0xC4, 0, True), (0xC4, FLAG_Z, False),
        (0xCC, FLAG_Z, True), (0xCC, 0, False),
        (0xD4, 0, True), (0xD4, FLAG_C, False),
        (0xDC, FLAG_C, True), (0xDC, 0, False),
        (0xE4, 0, True), (0xE4, FLAG_PV, False),
        (0xEC, FLAG_PV, True), (0xEC, 0, False),
        (0xF4, 0, True), (0xF4, FLAG_S, False),
        (0xFC, FLAG_S, True), (0xFC, 0, False),
    ])
    def test_call_cc_nn(self, cpu, opcode, flag_val, taken):
        """CALL cc,nn — conditional call."""
        cpu.registers.SP = 0xFFFF
        cpu.registers.F = flag_val
        write_program(cpu, [opcode, 0x00, 0x20])
        cpu.step()
        if taken:
            assert cpu.registers.PC == 0x2000
            assert cpu.registers.SP == 0xFFFD
        else:
            assert cpu.registers.PC == 3
            assert cpu.registers.SP == 0xFFFF


class TestRet:
    """RET - Unconditional return."""
    def test_ret(self, cpu):
        """RET — unconditional return."""
        cpu.registers.SP = 0xFFFD
        cpu.write_byte(0xFFFD, 0x00)
        cpu.write_byte(0xFFFE, 0x30)
        write_program(cpu, [0xC9])
        cpu.step()
        assert cpu.registers.PC == 0x3000
        assert cpu.registers.SP == 0xFFFF


class TestRetConditional:
    """RET cc - Conditional returns."""
    @pytest.mark.parametrize("opcode,flag_val,taken", [
        (0xC0, 0, True), (0xC0, FLAG_Z, False),
        (0xC8, FLAG_Z, True), (0xC8, 0, False),
        (0xD0, 0, True), (0xD0, FLAG_C, False),
        (0xD8, FLAG_C, True), (0xD8, 0, False),
        (0xE0, 0, True), (0xE0, FLAG_PV, False),
        (0xE8, FLAG_PV, True), (0xE8, 0, False),
        (0xF0, 0, True), (0xF0, FLAG_S, False),
        (0xF8, FLAG_S, True), (0xF8, 0, False),
    ])
    def test_ret_cc(self, cpu, opcode, flag_val, taken):
        """RET cc — conditional return."""
        cpu.registers.SP = 0xFFFD
        cpu.write_byte(0xFFFD, 0x00)
        cpu.write_byte(0xFFFE, 0x30)
        cpu.registers.F = flag_val
        write_program(cpu, [opcode])
        cpu.step()
        if taken:
            assert cpu.registers.PC == 0x3000
            assert cpu.registers.SP == 0xFFFF
        else:
            assert cpu.registers.PC == 1
            assert cpu.registers.SP == 0xFFFD


class TestRst:
    """RST n - Restart (software interrupt)."""
    @pytest.mark.parametrize("opcode,target", [
        (0xC7, 0x00), (0xCF, 0x08), (0xD7, 0x10), (0xDF, 0x18),
        (0xE7, 0x20), (0xEF, 0x28), (0xF7, 0x30), (0xFF, 0x38),
    ])
    def test_rst(self, cpu, opcode, target):
        """RST n — restart to fixed address."""
        cpu.registers.SP = 0xFFFF
        write_program(cpu, [opcode])
        cpu.step()
        assert cpu.registers.PC == target
        assert cpu.registers.SP == 0xFFFD
        assert cpu.read_byte(0xFFFE) == 0x00
        assert cpu.read_byte(0xFFFD) == 0x01


class TestRetiRetn:
    """RETI and RETN - Return from interrupt."""
    def test_reti(self, cpu):
        """RETI — return from interrupt."""
        cpu.registers.IFF2 = True  # IFF1 will be restored from IFF2
        cpu.registers.SP = 0xFFFD
        cpu.write_byte(0xFFFD, 0x00)
        cpu.write_byte(0xFFFE, 0x30)
        write_program(cpu, [0xED, 0x4D])
        cpu.step()
        assert cpu.registers.PC == 0x3000
        assert cpu.registers.SP == 0xFFFF
        assert cpu.registers.IFF1  # RETI restores IFF1 from IFF2

    def test_retn(self, cpu):
        """RETN — return from NMI."""
        cpu.registers.IFF2 = True
        cpu.registers.SP = 0xFFFD
        cpu.write_byte(0xFFFD, 0x00)
        cpu.write_byte(0xFFFE, 0x30)
        write_program(cpu, [0xED, 0x45])
        cpu.step()
        assert cpu.registers.PC == 0x3000
        assert cpu.registers.SP == 0xFFFF
        assert cpu.registers.IFF1  # RETN restores IFF1 from IFF2


class TestJumpTiming:
    """Verify cycle counts for jump instructions."""
    def test_jp_nn_cycles(self, cpu):
        """JP nn takes 10 cycles."""
        write_program(cpu, [0xC3, 0x00, 0x10])
        assert cpu.step() == 10

    def test_jp_cc_taken_cycles(self, cpu):
        """JP cc,nn (taken) takes 10 cycles."""
        write_program(cpu, [0xC2, 0x00, 0x10])
        assert cpu.step() == 10

    def test_jp_cc_not_taken_cycles(self, cpu):
        """JP cc,nn (not taken) takes 10 cycles."""
        cpu.registers.F = FLAG_Z
        write_program(cpu, [0xC2, 0x00, 0x10])
        assert cpu.step() == 10

    def test_jr_cycles(self, cpu):
        """JR e takes 12 cycles."""
        write_program(cpu, [0x18, 0x00])
        assert cpu.step() == 12

    def test_djnz_taken_cycles(self, cpu):
        """DJNZ (taken) takes 13 cycles."""
        cpu.registers.B = 2
        write_program(cpu, [0x10, 0x00])
        assert cpu.step() == 13

    def test_djnz_not_taken_cycles(self, cpu):
        """DJNZ (not taken) takes 8 cycles."""
        cpu.registers.B = 1
        write_program(cpu, [0x10, 0x00])
        assert cpu.step() == 8

    def test_call_cycles(self, cpu):
        """CALL nn takes 17 cycles."""
        cpu.registers.SP = 0xFFFF
        write_program(cpu, [0xCD, 0x00, 0x10])
        assert cpu.step() == 17

    def test_ret_cycles(self, cpu):
        """RET takes 10 cycles."""
        cpu.registers.SP = 0xFFFD
        cpu.write_byte(0xFFFD, 0x00)
        cpu.write_byte(0xFFFE, 0x30)
        write_program(cpu, [0xC9])
        assert cpu.step() == 10

    def test_rst_cycles(self, cpu):
        """RST n takes 11 cycles."""
        cpu.registers.SP = 0xFFFF
        write_program(cpu, [0xC7])
        assert cpu.step() == 11
