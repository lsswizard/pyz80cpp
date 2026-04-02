from core._pyz80 import Z80CPU, SimpleBus, Registers
from core._pyz80 import FLAG_S, FLAG_Z, FLAG_H, FLAG_PV, FLAG_N, FLAG_C


def run_prog(instructions, reg_setter=None):
    cpu = Z80CPU()
    bus = cpu.bus
    addr = 0
    for op in instructions:
        if isinstance(op, int):
            bus[addr] = op & 0xFF
            addr += 1
        else:
            for b in op:
                bus[addr] = b & 0xFF
                addr += 1
    bus[addr] = 0x76
    cpu.reset()
    if reg_setter:
        reg_setter(cpu)
    while not cpu.halted:
        cpu.step()
    return cpu


tests = 0


def check(msg, cpu, **conds):
    global tests
    for k, v in conds.items():
        actual = getattr(cpu.regs, k)
        if k == "F":
            assert (actual & v) == v, f"{msg}: {k}={actual:02X} missing {v:02X}"
        else:
            assert actual == v, f"{msg}: expected {k}={v:#x}, got {actual:#x}"
    tests += 1
    print(f"{msg}: PASS")


# Basic ALU
check("ADD A,B", run_prog([0x3E, 10, 0x06, 5, 0x80]), A=15)
check("SUB", run_prog([0x3E, 10, 0x06, 5, 0x90]), A=5, F=FLAG_N)
check("AND", run_prog([0x3E, 0xFF, 0xE6, 0x0F]), A=0x0F, F=FLAG_H)
check("XOR", run_prog([0x3E, 0xFF, 0xEE, 0xFF]), A=0x00, F=FLAG_Z)
check("OR", run_prog([0x3E, 0x00, 0xF6, 0xFF]), A=0xFF)
check(
    "ADC w/ carry", run_prog([0x37, 0x3E, 0xFF, 0xCE, 0x00]), A=0x00, F=FLAG_C | FLAG_Z
)
check("SBC", run_prog([0x37, 0x3E, 0x00, 0xDE, 0x01]), A=0xFE)
check("CP", run_prog([0x3E, 10, 0xFE, 10]), F=FLAG_Z)

# Misc
check("INC/DEC", run_prog([0x06, 0xFF, 0x04, 0x05]), B=0xFF)
check("PUSH/POP", run_prog([0x3E, 0xAB, 0xF5, 0x01, 0, 0, 0xF1]), A=0xAB)
check("NEG", run_prog([0x3E, 0x05, 0xED, 0x44]), A=0xFB, F=FLAG_N)
check(
    "EX DE,HL",
    run_prog([0x11, 0x34, 0x12, 0x21, 0x78, 0x56, 0xEB]),
    DE=0x5678,
    HL=0x1234,
)
check("SCF", run_prog([0x37]), F=FLAG_C)
check("CCF", run_prog([0x37, 0x3F]), F=0)
check("CPL", run_prog([0x3E, 0x0F, 0x2F]), A=0xF0, F=FLAG_H | FLAG_N)
check("IM 1", run_prog([0xED, 0x56]), IM=1)
check("IM 2", run_prog([0xED, 0x5E]), IM=2)

# CB prefix
check("RLC", run_prog([0x3E, 0x80, 0xCB, 0x07]), A=0x01, F=FLAG_C)
check("SRL A", run_prog([0x3E, 0xFF, 0xCB, 0x3F]), A=0x7F, F=FLAG_C)
check("BIT", run_prog([0x3E, 0x00, 0xCB, 0x47]), F=FLAG_H | FLAG_PV | FLAG_Z)
check("SET", run_prog([0x3E, 0x00, 0xCB, 0xC7]), A=0x01)
check("RES", run_prog([0x3E, 0xFF, 0xCB, 0x87]), A=0xFE)
check("SLL A", run_prog([0x3E, 0x00, 0xCB, 0x37]), A=0x01)

# DD prefix - IX tests
cpu = Z80CPU()
bus = cpu.bus
for i, b in enumerate([0xDD, 0x7E, 0x02, 0x76]):
    bus[i] = b
bus[0x0502] = 0xAB
cpu.reset()
cpu.regs.IX = 0x0500
cpu.step()
assert cpu.regs.A == 0xAB
tests += 1
print("LD A,(IX+d): PASS")

cpu = Z80CPU()
bus = cpu.bus
for i, b in enumerate([0xDD, 0x34, 0x01, 0x76]):
    bus[i] = b
bus[0x0501] = 0xFF
cpu.reset()
cpu.regs.IX = 0x0500
cpu.step()
assert bus[0x0501] == 0x00
tests += 1
print("INC (IX+d): PASS")

cpu = Z80CPU()
bus = cpu.bus
for i, b in enumerate([0xDD, 0x09, 0x76]):
    bus[i] = b
cpu.reset()
cpu.regs.IX = 0x1000
cpu.regs.B = 1
cpu.regs.C = 2
cpu.step()
assert cpu.regs.IX == 0x1102
tests += 1
print("ADD IX,BC: PASS")

cpu = Z80CPU()
bus = cpu.bus
for i, b in enumerate([0xDD, 0xE5, 0x76]):
    bus[i] = b
cpu.reset()
cpu.regs.IX = 0xABCD
cpu.regs.SP = 0xFFFE
cpu.step()
assert cpu.regs.SP == 0xFFFC
tests += 1
print("PUSH IX: PASS")

cpu = Z80CPU()
bus = cpu.bus
for i, b in enumerate([0xDD, 0xE1, 0x76]):
    bus[i] = b
cpu.reset()
cpu.regs.SP = 0xFFFC
cpu.step()
assert cpu.regs.IX == 0xABCD
tests += 1
print("POP IX: PASS")

check("LD IY,nn", run_prog([0xFD, 0x21, 0x34, 0x12]), IY=0x1234)
check(
    "LD A,IYH", run_prog([0xFD, 0x7C], lambda c: setattr(c.regs, "IY", 0xAB00)), A=0xAB
)

# Block operations
cpu2 = Z80CPU()
bus = cpu2.bus
for i, b in enumerate([0x3E, 0x12, 0x21, 0, 0x10, 0xED, 0x67]):
    bus[i] = b
bus[0x1000] = 0x34
bus[7] = 0x76
cpu2.reset()
[cpu2.step() for _ in range(10)]
assert cpu2.regs.A == 0x14 and bus[0x1000] == 0x23
tests += 1
print("RRD: PASS")

cpu2 = Z80CPU()
bus = cpu2.bus
for i, b in enumerate([0x01, 4, 0, 0x21, 0, 0x10, 0x11, 0, 0x20, 0xED, 0xB0, 0x76]):
    bus[i] = b
bus[0x1000] = 0xAA
bus[0x1001] = 0xBB
bus[0x1002] = 0xCC
bus[0x1003] = 0xDD
cpu2.reset()
[cpu2.step() for _ in range(100)]
assert bus[0x2000] == 0xAA and bus[0x2003] == 0xDD
tests += 1
print("LDIR: PASS")

cpu2 = Z80CPU()
bus = cpu2.bus
for i, b in enumerate([0x01, 4, 0, 0x21, 0, 0x10, 0x3E, 0x42, 0xED, 0xB1, 0x76]):
    bus[i] = b
bus[0x1000] = 1
bus[0x1001] = 2
bus[0x1002] = 0x42
bus[0x1003] = 3
cpu2.reset()
[cpu2.step() for _ in range(100)]
assert cpu2.regs.HL == 0x1003
tests += 1
print("CPIR: PASS")

cpu2 = Z80CPU()
bus = cpu2.bus
for i, b in enumerate([0x01, 4, 0, 0x21, 3, 0x10, 0x3E, 0x42, 0xED, 0xB9, 0x76]):
    bus[i] = b
bus[0x1000] = 1
bus[0x1001] = 2
bus[0x1002] = 0x42
bus[0x1003] = 3
cpu2.reset()
[cpu2.step() for _ in range(100)]
assert cpu2.regs.HL == 0x1001
tests += 1
print("CPDR: PASS")

# Control flow
cpu2 = Z80CPU()
bus = cpu2.bus
for i, b in enumerate([0x06, 3, 0x3C, 0x10, 0xFD, 0x76]):
    bus[i] = b
cpu2.reset()
[cpu2.step() for _ in range(100)]
assert cpu2.regs.A == 2
tests += 1
print("DJNZ: PASS")

cpu2 = Z80CPU()
bus = cpu2.bus
bus[0] = 0xCD
bus[1] = 0x10
bus[2] = 0
bus[3] = 0x76
bus[0x10] = 0x3E
bus[0x11] = 99
bus[0x12] = 0xC9
cpu2.reset()
[cpu2.step() for _ in range(100)]
assert cpu2.regs.A == 99
tests += 1
print("CALL/RET: PASS")

cpu2 = Z80CPU()
bus = cpu2.bus
bus[0] = 0x18
bus[1] = 2
bus[2] = 0
bus[3] = 0
bus[4] = 0x3E
bus[5] = 1
bus[6] = 0x76
cpu2.reset()
[cpu2.step() for _ in range(100)]
assert cpu2.regs.A == 1
tests += 1
print("JR: PASS")

# NMI
cpu2 = Z80CPU()
bus = cpu2.bus
bus[0] = 0x00
bus[1] = 0x76
bus[0x0066] = 0xC9
cpu2.reset()
cpu2.step()  # NOP
cpu2.trigger_nmi()
cpu2.step()  # NMI dispatch -> PC=0x0066
assert cpu2.regs.PC == 0x0066
cpu2.step()  # RET at 0x0066 -> PC=0x0001
assert cpu2.regs.PC == 0x0001
tests += 1
print("NMI: PASS")

# Batch run
cpu2 = Z80CPU()
bus = cpu2.bus
for i in range(10):
    bus[i] = 0x00
bus[10] = 0x76
cpu2.reset()
cycles = cpu2.run(100)
assert cpu2.instruction_count == 11
tests += 1
print("Batch run: PASS")

print(f"\n=== ALL {tests} TESTS PASSED ===")
