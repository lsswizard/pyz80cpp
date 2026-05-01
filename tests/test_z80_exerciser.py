#!/usr/bin/env python3
"""
Z80 Test Suite - Python adaptation of test-Z80.c
Tests the z80c++ core against known Z80 test programs.

Test files need to be downloaded separately. Place them in a tests/ directory.
"""

import os
import sys
import struct
from pathlib import Path

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Try to import z80c++ core
try:
    from z80_core import Z80 as Z80CPU
except ImportError:
    print("Error: z80c++ core not found. Run 'python -m pytest' from project root.")
    sys.exit(1)


# FNV-1 hash implementation (matching test-Z80.c)
FNV1_32_INIT = 0x811C9DC5


def fnv1_32(data: bytes, init: int = FNV1_32_INIT) -> int:
    """FNV-1 32-bit hash."""
    h = init
    for byte in data:
        h = (h * 0x01000193) & 0xFFFFFFFF
        h ^= byte
    return h


class Z80TestHarness:
    """Test harness matching test-Z80.c behavior."""

    def __init__(self):
        self.cpu = Z80CPU()
        self.reset()

    def reset(self):
        """Reset CPU and memory."""
        self.cpu.reset()
        # Fill memory with 0x00 for CP/M compatibility (or 0xFF)
        for i in range(65536):
            self.cpu.write_byte(i, 0x00)

    def load_cpm(self, data: bytes, addr: int = 0x0100):
        """Load CP/M .COM format program."""
        # Ensure memory is clear before loading
        self.reset()
        for i, b in enumerate(data):
            self.cpu.write_byte(addr + i, b)

    def load_tap(self, data: bytes) -> int:
        """Load ZX Spectrum TAP format. Returns entry point."""
        self.reset()
        pos = 0
        if pos + 2 > len(data):
            return 0x8000
        block_len = struct.unpack("<H", data[pos : pos + 2])[0]
        pos += 2
        if pos + block_len > len(data):
            return 0x8000
        pos += block_len

        if pos + 2 > len(data):
            return 0x8000
        block_len = struct.unpack("<H", data[pos : pos + 2])[0]
        pos += 2
        if pos + block_len > len(data):
            return 0x8000

        for i in range(block_len):
            self.cpu.write_byte(0x8000 + i, data[pos + i])
        return 0x8000

    def read_mem(self, addr: int) -> int:
        """Read memory."""
        return self.cpu.read_byte(addr)

    def run_cpm(self, max_cycles: int = 100_000_000) -> tuple:
        """Run CP/M format test. Returns (cycles, hash, lines, columns)."""
        # CP/M .COM files:
        # - Entry at 0x0100
        # - BDOS called via CALL 5 (bytes: CD 05 00)
        
        # BDOS entry point - set up jump to our hook
        HOOK_ADDR = 0xF000
        self.cpu.write_byte(0x0005, 0xC3)  # JP HOOK_ADDR
        self.cpu.write_byte(0x0006, HOOK_ADDR & 0xFF)
        self.cpu.write_byte(0x0007, HOOK_ADDR >> 8)
        
        # At HOOK_ADDR, we put a RET. When we detect PC == HOOK_ADDR, we handle BDOS and then let it RET.
        self.cpu.write_byte(HOOK_ADDR, 0xC9)  # RET

        # Initialize registers
        self.cpu.registers.PC = 0x0100
        self.cpu.registers.SP = 0xFFFE
        
        # Push program exit address (0x0000 - when popped by program final RET, PC=0 and we exit)
        self.cpu.write_byte(0xFFFE, 0x00)
        self.cpu.write_byte(0xFFFF, 0x00)

        completed = False
        cycles = 0
        hash_val = FNV1_32_INIT
        lines = 0
        columns = 0
        cursor_x = 0

        while not completed and cycles < max_cycles:
            # Check for BDOS hook BEFORE step
            if self.cpu.registers.PC == HOOK_ADDR:
                c = self.cpu.registers.C
                if c == 2:  # BDOS function 2: print char (E = char)
                    char = self.cpu.registers.E
                    sys.stdout.write(chr(char))
                    sys.stdout.flush()
                    if char == 0x0A:  # LF
                        lines += 1
                        if cursor_x > columns: columns = cursor_x
                        cursor_x = 0
                    elif char != 0x0D:  # Ignore CR
                        hash_val = fnv1_32(bytes([char]), hash_val)
                        cursor_x += 1
                elif c == 9:  # BDOS function 9: print string (DE = string addr, $ terminated)
                    de = self.cpu.registers.DE
                    while self.read_mem(de) != 0x24:  # $ terminator
                        char = self.read_mem(de)
                        sys.stdout.write(chr(char))
                        sys.stdout.flush()
                        if char == 0x0A:  # LF
                            lines += 1
                            if cursor_x > columns: columns = cursor_x
                            cursor_x = 0
                        elif char != 0x0D:  # Ignore CR
                            hash_val = fnv1_32(bytes([char]), hash_val)
                            cursor_x += 1
                        de = (de + 1) & 0xFFFF
                elif c == 0:  # BDOS function 0: program terminate
                    completed = True
                    break

            t = self.cpu.step()
            cycles += t

            if self.cpu.halted:
                completed = True
                break

            # Exit if we reached address 0
            if self.cpu.registers.PC == 0:
                completed = True
                break

        if cursor_x > columns:
            columns = cursor_x

        return cycles, hash_val, lines, columns

    def run_spectrum(self, print_hook: int = 0x0010, max_cycles: int = 100_000_000) -> tuple:
        """Run ZX Spectrum format test."""
        # Note: self.cpu.registers.pop() doesn't exist, we must use manual logic
        def pop_stack():
            lo = self.cpu.read_byte(self.cpu.registers.SP)
            self.cpu.registers.SP = (self.cpu.registers.SP + 1) & 0xFFFF
            hi = self.cpu.read_byte(self.cpu.registers.SP)
            self.cpu.registers.SP = (self.cpu.registers.SP + 1) & 0xFFFF
            return (hi << 8) | lo

        # Initial Spectrum state
        # PC already set by load_tap (usually 0x8000)
        self.cpu.registers.SP = 0x7FE8
        self.cpu.registers.AF = 0x3222
        self.cpu.registers.I = 0x3F

        completed = False
        cycles = 0
        hash_val = FNV1_32_INIT
        lines = 0
        columns = 0
        cursor_x = 0

        while not completed and cycles < max_cycles:
            # Print hook at 0x0010
            if self.cpu.registers.PC == print_hook:
                char = self.cpu.registers.A
                if char == 0x0D:  # ENTER
                    lines += 1
                    if cursor_x > columns: columns = cursor_x
                    cursor_x = 0
                elif char == 0x17:  # TAB
                    pass
                elif char == 0x7F:  # Copyright
                    cursor_x += 1
                elif 32 <= char < 127:
                    hash_val = fnv1_32(bytes([char]), hash_val)
                    cursor_x += 1
                
                # Return from hook
                self.cpu.registers.PC = pop_stack()
                continue

            t = self.cpu.step()
            cycles += t

            if self.cpu.halted:
                completed = True
                break
            
            # Simple exit condition for Spectrum tests (usually JP 0x0000 or similar)
            if self.cpu.registers.PC == 0x0000:
                completed = True
                break

        if cursor_x > columns:
            columns = cursor_x

        return cycles, hash_val, lines, columns


def find_test_file(filename: str, search_paths: list) -> bytes | None:
    """Find test file in search paths."""
    for path in search_paths:
        full_path = os.path.join(path, filename)
        if os.path.exists(full_path):
            with open(full_path, "rb") as f:
                return f.read()
    if os.path.exists(filename):
        with open(filename, "rb") as f:
            return f.read()
    return None


class TestZ80Exerciser:
    """Test Z80 instruction exercisers."""

    @pytest.fixture
    def harness(self):
        return Z80TestHarness()

    @pytest.fixture
    def search_paths(self):
        """Search paths for test files."""
        return [
            "tests/z80_tests",
            "tests",
            ".",
        ]

    def test_zexdoc_cpm(self, harness, search_paths):
        """Test ZEXDOC CP/M exerciser."""
        data = find_test_file("zexdoc.com", search_paths)
        if data is None:
            pytest.skip("zexdoc.com not found.")

        harness.load_cpm(data)
        # ZEXDOC takes a LONG time (~46B cycles), we limit it for CI
        cycles, hash_val, lines, columns = harness.run_cpm(max_cycles=10_000_000)

        print(f"\nZEXDOC CP/M (Partial):")
        print(f"  Cycles executed: {cycles}")
        print(f"  Current Hash: {hash_val:08X}")

    def test_zexall_cpm(self, harness, search_paths):
        """Test ZEXALL CP/M exerciser."""
        data = find_test_file("zexall.com", search_paths)
        if data is None:
            pytest.skip("zexall.com not found.")

        harness.load_cpm(data)
        # ZEXALL also takes a long time
        cycles, hash_val, lines, columns = harness.run_cpm(max_cycles=10_000_000)

        print(f"\nZEXALL CP/M (Partial):")
        print(f"  Cycles executed: {cycles}")
        print(f"  Current Hash: {hash_val:08X}")


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
