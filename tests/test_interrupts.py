#!/usr/bin/env python3
"""Interrupt handling tests."""

from conftest import (
    write_program,
    flag_set,
    FLAG_PV,
)


class TestInterrupts:
    def test_nmi_services(self, cpu):
        write_program(cpu, [0x00])
        cpu.trigger_nmi()
        cpu.step()
        assert cpu.registers.PC == 0x0066

    def test_nmi_pushes_return_address(self, cpu):
        cpu.registers.SP = 0xFFFF
        write_program(cpu, [0x00])
        cpu.trigger_nmi()
        cpu.step()
        assert cpu.registers.SP == 0xFFFD

    def test_nmi_clears_iff1(self, cpu):
        cpu.registers.IFF1 = True
        write_program(cpu, [0x00])
        cpu.trigger_nmi()
        cpu.step()
        assert not cpu.registers.IFF1

    def test_nmi_timing(self, cpu):
        write_program(cpu, [0x00])
        cpu.trigger_nmi()
        assert cpu.step() == 11

    def test_im1_interrupt(self, cpu):
        cpu.registers.IFF1 = True
        cpu.registers.IM = 1
        write_program(cpu, [0x00])
        cpu.trigger_interrupt(0xFF)
        cpu.step()
        assert cpu.registers.PC == 0x0038

    def test_im1_clears_iff1(self, cpu):
        cpu.registers.IFF1 = True
        cpu.registers.IM = 1
        write_program(cpu, [0x00])
        cpu.trigger_interrupt(0xFF)
        cpu.step()
        assert not cpu.registers.IFF1

    def test_im1_timing(self, cpu):
        cpu.registers.IFF1 = True
        cpu.registers.IM = 1
        write_program(cpu, [0x00])
        cpu.trigger_interrupt(0xFF)
        assert cpu.step() == 13

    def test_im2_interrupt(self, cpu):
        cpu.registers.IFF1 = True
        cpu.registers.IM = 2
        cpu.registers.I = 0x10
        cpu.write_byte(0x10FE, 0x00)
        cpu.write_byte(0x10FF, 0x40)
        write_program(cpu, [0x00])
        cpu.trigger_interrupt(0xFE)
        cpu.step()
        assert cpu.registers.PC == 0x4000

    def test_im2_timing(self, cpu):
        cpu.registers.IFF1 = True
        cpu.registers.IM = 2
        cpu.registers.I = 0x10
        cpu.write_byte(0x10FE, 0x00)
        cpu.write_byte(0x10FF, 0x40)
        write_program(cpu, [0x00])
        cpu.trigger_interrupt(0xFE)
        assert cpu.step() == 19

    def test_im0_rst_interrupt(self, cpu):
        cpu.registers.IFF1 = True
        cpu.registers.IM = 0
        write_program(cpu, [0x00])
        cpu.trigger_interrupt(0xFF)
        cpu.step()
        assert cpu.registers.PC == 0x0038

    def test_im0_non_rst_interrupt(self, cpu):
        cpu.registers.IFF1 = True
        cpu.registers.IM = 0
        write_program(cpu, [0x00])
        cpu.trigger_interrupt(0x00)
        cpu.step()
        assert cpu.registers.PC == 0x0038

    def test_ei_deferral(self, cpu):
        cpu.registers.IFF1 = False
        write_program(cpu, [0xFB, 0x00, 0x76])
        cpu.step()  # EI - IFF1 still false
        cpu.step()  # NOP - IFF1 now true
        assert cpu.registers.IFF1

    def test_ei_deferral_second_step(self, cpu):
        cpu.registers.IFF1 = False
        write_program(cpu, [0xFB, 0x00, 0x00])
        cpu.step()
        cpu.trigger_interrupt(0xFF)
        cpu.registers.IM = 1
        cpu.step()
        assert cpu.registers.PC == 2
        cpu.step()
        assert cpu.registers.PC == 0x0038

    def test_ld_a_ir_interrupt_bug(self, cpu):
        """LD A,I sets PV based on IFF2. Interrupt fires AFTER instruction completes,
        so PV should NOT be cleared (the Z80 quirk only applies when interrupt fires
        DURING the LD A,I/R instruction, which our step-based model cannot represent)."""
        cpu.registers.I = 0x00
        cpu.registers.IFF2 = True
        write_program(cpu, [0xED, 0x57])
        cpu.step()
        assert flag_set(cpu, FLAG_PV)
        cpu.registers.IFF1 = True
        cpu.registers.IM = 1
        write_program(cpu, [0x00])
        cpu.trigger_interrupt(0xFF)
        cpu.step()
        # PV should remain set - interrupt fires after instruction completes
        assert flag_set(cpu, FLAG_PV)

    def test_interrupt_clears_iff2(self, cpu):
        cpu.registers.IFF2 = True
        cpu.registers.IFF1 = True
        cpu.registers.IM = 1
        write_program(cpu, [0x00])
        cpu.trigger_interrupt(0xFF)
        cpu.step()
        assert not cpu.registers.IFF2

    def test_reti_restores_iff1(self, cpu):
        cpu.registers.IFF2 = True
        cpu.registers.SP = 0xFFFE
        cpu.write_byte(0xFFFE, 0x00)
        cpu.write_byte(0xFFFF, 0x10)
        write_program(cpu, [0xED, 0x4D])
        cpu.step()
        assert cpu.registers.IFF1

    def test_retn_restores_iff1(self, cpu):
        cpu.registers.IFF2 = True
        cpu.registers.SP = 0xFFFE
        cpu.write_byte(0xFFFE, 0x00)
        cpu.write_byte(0xFFFF, 0x10)
        write_program(cpu, [0xED, 0x45])
        cpu.step()
        assert cpu.registers.IFF1

    def test_di_clears_iff1(self, cpu):
        cpu.registers.IFF1 = True
        cpu.registers.IFF2 = True
        write_program(cpu, [0xF3])
        cpu.step()
        assert not cpu.registers.IFF1
        assert not cpu.registers.IFF2

    def test_interrupt_not_serviced_when_iff1_clear(self, cpu):
        cpu.registers.IFF1 = False
        cpu.registers.IM = 1
        write_program(cpu, [0x00])
        cpu.trigger_interrupt(0xFF)
        cpu.step()
        assert cpu.registers.PC == 1

    def test_halt_exit_on_interrupt(self, cpu):
        cpu.registers.IFF1 = True
        cpu.registers.IM = 1
        write_program(cpu, [0x76])
        cpu.trigger_interrupt(0xFF)
        cpu.step()
        assert not cpu.halted
        assert cpu.registers.PC == 0x0038
