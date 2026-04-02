import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

from core import Z80CPU, SimpleBus, FLAG_Z

bus = SimpleBus()
# Program: DEC C at address 0
bus[0] = 0x0D
cpu = Z80CPU(bus)
cpu.reset()
cpu.regs.PC = 0
z_flag_before = bool(cpu.regs.F & FLAG_Z)
print("Before: C=", cpu.regs.C, "Z=", z_flag_before, "F=", cpu.regs.F)
cpu.step()
z_flag_after = bool(cpu.regs.F & FLAG_Z)
print("After: C=", cpu.regs.C, "Z=", z_flag_after, "F=", cpu.regs.F)
print("Cycles:", cpu.cycles, "Instructions:", cpu.instruction_count)
