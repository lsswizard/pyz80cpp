# Z80Core — Machine-Agnostic Cycle-Exact Z80 CPU Core

A high-performance Z80 CPU emulator implemented in C++ as a reusable library. The core is **100% machine-independent** — all machine-specific logic (memory mapping, I/O, contention, interrupts) lives in the host application.

## Features

- **Cycle-exact execution** — every instruction matches Zilog T-state specifications
- **Machine-agnostic design** — zero machine-specific code in the C++ core
- **Modular handler architecture** — handlers organized by category (ALU, block, I/O, jump, load)
- **Flat register API** — `cpu.A`, `cpu.PC`, `cpu.HL`, `cpu.IFF1`, etc.
- **Clean header/include separation** — proper C++ library layout
- **Optional Python bindings** — nanobind bindings available via `-DENABLE_NANOBIND=ON`
- **CMake build system** — modern CMake with find_package support
- **No external dependencies** — standalone C++ core
- **Latest v2.4.1** — Improved flag accuracy, timing, and MEMPTR handling (see CHANGELOG.md)

## Quick Start

```cpp
#include <z80/z80.h>

Z80CPU cpu;

// Write a program: LD A, 0x42 / HALT
cpu.writeByte(0, 0x3E);
cpu.writeByte(1, 0x42);
cpu.writeByte(2, 0x76);

cpu.reset();
cpu.setPC(0);

// Execute
cpu.step();      // LD A, 0x42 — returns 7 T-states
cpu.getA();      // 0x42

cpu.step();      // HALT — returns 4 T-states
cpu.isHalted(); // true
```

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

**Requirements:** CMake 3.16+, C++17 compiler

## Running Tests

```bash
cmake .. -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
ctest --output-on-failure
```

Or with GTest directly:

```bash
./z80_tests
```

## Machine Integration

The CPU core has **no knowledge** of any specific machine. To emulate a system (ZX Spectrum, CPC, MSX, etc.), implement machine logic by subclassing `Z80Bus`:

```cpp
#include <z80/z80.h>
#include <z80/bus.h>

class MyMachine : public z80::Z80Bus {
public:
    uint8_t read(uint16_t addr) override {
        return memory[addr];
    }

    void write(uint16_t addr, uint8_t value) override {
        memory[addr] = value;
    }

    uint8_t portRead(uint16_t port) override {
        return 0xFF;  // Keyboard, PSG, etc.
    }

    void portWrite(uint16_t port, uint8_t value) override {
        // Handle I/O devices
    }

    void onReti() override {
        // Release interrupt daisy chain (Z80PIO/Z80CTC)
    }

    uint8_t getInterruptVector() override {
        return 0xFF;  // IM2 vector
    }

private:
    uint8_t memory[65536];
};
```

See `include/z80/*.h` for the complete API reference.

## License

MIT
