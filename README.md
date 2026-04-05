# PyZ80 — Machine-Agnostic Cycle-Exact Z80 CPU Core

A high-performance Z80 CPU emulator implemented in C++ with raw CPython bindings. The core is **100% machine-independent** — all machine-specific logic (memory mapping, I/O, contention, interrupts) lives in Python.

## Features

- **Cycle-exact execution** — every instruction matches Zilog T-state specifications
- **Machine-agnostic design** — zero machine-specific code in the C++ core
- **Flat register API** — `cpu.A`, `cpu.PC`, `cpu.HL`, `cpu.IFF1`, etc. directly on the CPU object
- **I/O callbacks** — `set_on_input_callback()` / `set_on_output_callback()` for machine I/O
- **RETI callback** — interrupt daisy chaining support (Z80PIO, Z80CTC)
- **Interrupt vector callback** — machine provides vector on demand during INT acknowledge
- **Memory marking** — `mark_addrs()` / `unmark_addrs()` for breakpoints and self-modifying code detection
- **Contention support** — `add_cycles()` for ULA wait states and memory contention
- **Interrupt control** — machine-triggered `trigger_interrupt(data)` and `trigger_nmi()`
- **Full instruction set** — all 256 base opcodes + CB, ED, DD, FD prefixes + DDCB/FDCB
- **Undocumented behavior** — Q factor, F3/F5 flags, DD/FD prefix fallthrough
- **No external dependencies** — raw CPython API, no nanobind or other binding libraries

## Quick Start

```python
from core import Z80CPU

cpu = Z80CPU()

# Write a program: LD A, 0x42 / HALT
cpu.write_byte(0, 0x3E)
cpu.write_byte(1, 0x42)
cpu.write_byte(2, 0x76)

cpu.reset()
cpu.PC = 0

# Execute
cpu.step()      # LD A, 0x42 — returns 7 T-states
print(hex(cpu.A))  # 0x42

cpu.step()      # HALT — returns 4 T-states
print(cpu.halted)  # True
```

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cp _pyz80.so ../core/_pyz80.cpython-314-x86_64-linux-gnu.so
```

**Requirements:** CMake 3.20+, C++20 compiler, Python 3.12+

## Running Tests

```bash
python3 -m pytest tests/ -v
```

762 tests covering loads, ALU, flags, jumps, calls, stack, I/O, block ops, indexed, interrupts, timing, undocumented flags, DD/FD fallthrough, DDCB/FDCB, DAA, Q factor, edge cases, and integration.

## Machine Integration

The CPU core has **no knowledge** of any specific machine. To emulate a system (ZX Spectrum, CPC, MSX, etc.), implement machine logic in Python:

```python
from core import Z80CPU

class MyMachine:
    def __init__(self):
        self.cpu = Z80CPU()
        self.cpu.set_on_input_callback(self.io_read)
        self.cpu.set_on_output_callback(self.io_write)
        self.cpu.set_on_reti_callback(self.on_reti)  # Daisy chain support
        self.cpu.set_on_get_int_vector_callback(self.get_int_vector)

    def io_read(self, port):
        return 0xFF

    def io_write(self, port, value):
        pass

    def on_reti(self):
        # Release interrupt daisy chain (Z80PIO/Z80CTC)
        pass

    def get_int_vector(self):
        # Return interrupt vector for IM2
        return 0xFF

    def run_frame(self):
        self.cpu.trigger_interrupt(0xFF)
        self.cpu.run_frame(69888)  # Spectrum: 69888 T-states per frame
```

See `TECHNICAL.md` for the complete API reference and integration guide.

## License

MIT
