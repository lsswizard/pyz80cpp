# Z80Core — Machine-Agnostic Cycle-Exact Z80 CPU Core

A high-performance Z80 CPU emulator implemented in C++ as a reusable library. The core is **100% machine-independent** — all machine-specific logic (memory mapping, I/O, contention, interrupts) lives in the host application via a Bus interface.

## Features

- **Cycle-accurate execution** — every instruction matches Zilog T-state specifications using internal M-cycle wait states.
- **Machine-agnostic design** — zero machine-specific code in the C++ core; all interactions go through a `Bus` interface.
- **Modular handler architecture** — instruction handlers organized by category (ALU, block, I/O, jump, load).
- **Comprehensive Instruction Support** — includes all documented and many undocumented instructions (e.g., `SLL`, `IXH/IXL` register access, undocumented `IM` modes).
- **Optional Python bindings** — `nanobind` bindings available for high-performance Python integration.
- **Modern C++ build system** — uses CMake 3.16+ and C++17.
- **High test coverage** — over 1390 tests, including ZEXDOC/ZEXALL exercisers.

## Quick Start (C++)

```cpp
#include <z80/z80.h>
#include <z80/bus.h>

using namespace z80;

// Simple memory-only bus
SimpleBus bus;
Z80 cpu(&bus);

// Write a program: LD A, 0x42 / HALT
bus.write(0x0000, 0x3E); // LD A, n
bus.write(0x0001, 0x42); 
bus.write(0x0002, 0x76); // HALT

cpu.reset();
cpu.regs.PC = 0;

// Execute
cpu.step();      // LD A, 0x42 — returns 7 T-states
// cpu.regs.A is 0x42

cpu.step();      // HALT — returns 4 T-states
// cpu.is_halted() is true
```

## Python Integration

If compiled with `-DENABLE_NANOBIND=ON`:

```python
from z80_core import Z80, SimpleBus

bus = SimpleBus()
cpu = Z80(bus)

# Load machine code
bus.write(0x100, 0x00) # NOP

cpu.registers.PC = 0x100
t_states = cpu.step()
print(f"Executed in {t_states} cycles")
```

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_NANOBIND=ON
make -j$(nproc)
```

**Requirements:** CMake 3.16+, C++17 compiler (GCC 9+, Clang 10+).

## Running Tests

```bash
pytest tests/ -v
```

Tests include:
- Detailed instruction-by-instruction verification.
- Timing-accurate checks for every M-cycle.
- Z80 exercisers (ZEXDOC, ZEXALL) for verifying full ALU and flag logic.

## Machine Integration

Emulate any Z80-based system by subclassing the `Bus` class:

```cpp
class MyMachineBus : public z80::Bus {
public:
    uint8_t read(uint16_t addr) override { /* ... */ }
    void write(uint16_t addr, uint8_t val) override { /* ... */ }
    uint8_t in_(uint16_t port) override { /* ... */ }
    void out_(uint16_t port, uint8_t val) override { /* ... */ }
    
    // Optional: model wait states or contention
    int get_memory_wait_states(uint16_t addr) override { return 0; }
};
```

## License

MIT
