"""
Z80 Core Benchmark
Compares Python vs C++ core performance.
Uses machine-agnostic API: cpu.write_byte, cpu.step(), cpu.run().
"""

import time
import sys

sys.path.insert(0, ".")

from z80_py import Z80 as Z80CPU


def create_program(cpu, program: list[int], start: int = 0) -> None:
    for i, op in enumerate(program):
        cpu.write_byte(start + i, op)


def bench_step(program, cycles_target=100_000, label=""):
    cpu = Z80CPU()
    create_program(cpu, program)
    cpu.reset()

    start = time.perf_counter()
    while cpu.get_cycles() < cycles_target:
        cpu.step()
    elapsed = time.perf_counter() - start

    instrs = cpu.get_instruction_count()
    ips = instrs / elapsed
    mips = ips / 1_000_000
    real_35 = ips / 875_000  # real Z80 @ 3.5MHz ~875K instr/sec

    print(f"  {label:30s} {mips:7.2f} MIPS  ({ips:>14,.0f} IPS)  {real_35:>7.1f}x real")
    return ips


def bench_run(program, cycles_target=1_000_000, label=""):
    cpu = Z80CPU()
    create_program(cpu, program)
    cpu.reset()

    start = time.perf_counter()
    total = 0
    runs = 0
    while total < cycles_target:
        c = cpu.run(100_000)
        total += c
        runs += 1
        if cpu.is_halted():
            cpu.reset()
    elapsed = time.perf_counter() - start

    mips = total / elapsed / 1_000_000
    print(f"  {label:30s} {mips:7.1f} MIPS  ({total:>14,} cycles)  batch run()")

    cpu.reset()
    return total / elapsed


def main():
    print("=" * 70)
    print("Z80 C++ Core Benchmark")
    print("=" * 70)

    # Programs for each benchmark
    programs = {
        "ALU loop (DEC/JNZ)": [
            0x06,
            0xFF,  # LD B, 255
            0x3E,
            0x00,  # LD A, 0
            0x80,  # ADD A, B
            0x05,  # DEC B
            0xC2,
            0x04,
            0x00,  # JP NZ, 4
            0x76,  # HALT
        ],
        "Memory (LD (HL),A / LD A,(HL))": [
            0x3E,
            0x55,  # LD A, 0x55
            0x77,  # LD (HL), A
            0x7E,  # LD A, (HL)
            0x18,
            0xFB,  # JR -5
        ],
        "Block transfer (LDI loop)": [
            0xED,
            0xA0,  # LDI
            0x18,
            0xFC,  # JR -4
        ],
        "Indexed (ADD A,(IX+d))": [
            0xDD,
            0x86,
            0x00,  # ADD A, (IX+0)
            0x18,
            0xFB,  # JR -5
        ],
        "Block search (CPI loop)": [
            0xED,
            0xA1,  # CPI
            0x18,
            0xFC,  # JR -4
        ],
        "Register copy (LD r,r' loop)": [
            0x78,  # LD A, B
            0x4F,  # LD C, A
            0x51,  # LD D, C
            0x5A,  # LD E, D
            0x63,  # LD H, E
            0x6C,  # LD L, H
            0x18,
            0xF7,  # JR -9
        ],
    }

    print("\n--- step() per-instruction benchmarks (50K cycles each) ---\n")
    for name, prog in programs.items():
        bench_step(prog, cycles_target=50_000, label=name)

    print("\n--- batch run() benchmarks (1M cycles each) ---\n")
    for name, prog in programs.items():
        bench_run(prog, cycles_target=1_000_000, label=f"{name}")

    # Large-scale batch benchmark
    print("\n--- Large-scale batch benchmark ---\n")
    halt_prog = [
        0x06,
        0x00,  # LD B, 0
        0x3C,  # INC A
        0x05,  # DEC B
        0xC2,
        0x02,
        0x00,  # JP NZ, 2
        0x76,  # HALT
    ]

    cpu = Z80CPU()
    create_program(cpu, halt_prog)
    cpu.reset()

    # Run 10M cycles
    start = time.perf_counter()
    total = 0
    while total < 10_000_000:
        c = cpu.run(100_000)
        total += c
        if cpu.is_halted():
            cpu.reset()
    elapsed = time.perf_counter() - start
    mips = total / elapsed / 1_000_000
    print(f"  10M cycles batch:  {mips:.1f} MIPS  ({elapsed:.3f}s)")

    # Run 100M cycles
    cpu.reset()
    start = time.perf_counter()
    total = 0
    while total < 100_000_000:
        c = cpu.run(1_000_000)
        total += c
        if cpu.is_halted():
            cpu.reset()
    elapsed = time.perf_counter() - start
    mips = total / elapsed / 1_000_000
    print(f"  100M cycles batch: {mips:.1f} MIPS  ({elapsed:.3f}s)")

    print("\n" + "=" * 70)
    print("Real Z80 @ 3.5MHz = ~0.875 MIPS")
    print("=" * 70)


if __name__ == "__main__":
    main()
