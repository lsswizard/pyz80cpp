import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

from core import Z80CPU, FLAG_Z

cpu = Z80CPU()

# Program: DEC C at address 0
cpu.write_byte(0, 0x0D)
cpu.reset()
cpu.PC = 0
z_flag_before = bool(cpu.F & FLAG_Z)
print("Before: C=", cpu.C, "Z=", z_flag_before, "F=", cpu.F)
cpu.step()
z_flag_after = bool(cpu.F & FLAG_Z)
print("After: C=", cpu.C, "Z=", z_flag_after, "F=", cpu.F)
print("Cycles:", cpu.cycles, "Instructions:", cpu.instruction_count)
