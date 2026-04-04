#!/usr/bin/env python3
"""Quick flags sanity check — run standalone or via pytest."""

import sys

sys.path.insert(0, "/home/lss/builds/PythonZ80")

from cpu import Z80CPU, FLAG_Z, FLAG_N, FLAG_C, FLAG_H, FLAG_PV


def test_dec_c_flags():
    cpu = Z80CPU()
    cpu.write_byte(0, 0x0D)  # DEC C
    cpu.regs.PC = 0
    cpu.regs.C = 0x01
    cpu.step()
    assert cpu.regs.C == 0x00
    assert cpu.regs.F & FLAG_Z  # Z set
    assert cpu.regs.F & FLAG_N  # N set
    assert not (cpu.regs.F & FLAG_H)  # H clear (0x01 & 0x0F != 0)
    print("DEC C 1->0: OK  F =", hex(cpu.regs.F))


def test_dec_c_half_borrow():
    cpu = Z80CPU()
    cpu.write_byte(0, 0x0D)  # DEC C
    cpu.regs.PC = 0
    cpu.regs.C = 0x10
    cpu.step()
    assert cpu.regs.C == 0x0F
    assert not (cpu.regs.F & FLAG_Z)
    assert cpu.regs.F & FLAG_H  # half-borrow
    print("DEC C 0x10->0x0F: OK  F =", hex(cpu.regs.F))


def test_dec_c_overflow():
    cpu = Z80CPU()
    cpu.write_byte(0, 0x0D)  # DEC C
    cpu.regs.PC = 0
    cpu.regs.C = 0x80
    cpu.step()
    assert cpu.regs.C == 0x7F
    assert cpu.regs.F & FLAG_PV  # overflow
    assert cpu.regs.F & FLAG_H
    print("DEC C 0x80->0x7F: OK  F =", hex(cpu.regs.F))


if __name__ == "__main__":
    test_dec_c_flags()
    test_dec_c_half_borrow()
    test_dec_c_overflow()
    print("All flag tests passed")
