# Technical Reference — Z80Core C++

## Architecture

The Z80Core is designed as a cycle-accurate, M-cycle based emulator. It uses a **Pull model** where the CPU requests data/cycles from a `Bus` interface.

```
┌──────────────────────────────────────────────┐
│  Host Application (C++ or Python)            │
│  - Implements Bus interface                  │
│  - Manages Memory, I/O devices               │
│  - Handles Video/Audio Generation            │
└──────────────────┬───────────────────────────┘
                   │
                   │ Virtual calls: read(), write(), in_(), out_()
                   │ Cycle updates: interrupt_acknowledge()
                   │
┌──────────────────▼───────────────────────────┐
│  Z80Core (C++ Static Library)                │
│  - M-cycle accurate instruction handlers      │
│  - Automated T-state accounting via Bus      │
│  - Full Register set (Main, Alt, Index)      │
│  - Internal opcode lookup tables             │
└──────────────────────────────────────────────┘
```

## Core Components

### 1. Register File (`Registers` struct)
Located in `include/z80/registers.h`.
- Supports 8-bit (`A`, `F`, `B`, `C`, ...) and 16-bit (`AF`, `BC`, `HL`, `IX`, `IY`, `PC`, `SP`) access.
- Includes undocumented `MEMPTR` (WZ) and `Q` flag factor.
- `IFF1`, `IFF2` interrupt flip-flops.

### 2. Bus Interface (`Bus` class)
Located in `include/z80/bus.h`.
- All memory and I/O access must pass through this interface.
- Methods: `read(addr)`, `write(addr, val)`, `in_(port)`, `out_(port, val)`.
- `interrupt_acknowledge()`: Called during INT service to fetch vector.
- `CycleType` enum: Distinguishes between M1 fetch, Memory Read/Write, I/O, etc.

### 3. Opcode Tables
Located in `src/opcode_table.cpp`.
- `main_table`: Standard opcodes.
- `cb_table`: Bitwise/Rotate/Shift.
- `ed_table`: Extended instructions.
- `dd_table` / `fd_table`: IX/IY prefixed instructions.
- `ddcb_table` / `fdcb_table`: Indexed bitwise operations.

## Cycle-Accuracy and Timing

Each instruction is broken down into its constituent M-cycles (Machine Cycles). The CPU core calls `cpu.wait(n)` to increment the T-state counter. Memory/IO helpers automatically add the required base T-states:

- **Opcode Fetch (M1)**: 4 T-states.
- **Memory Read/Write**: 3 T-states.
- **I/O Read/Write**: 4 T-states.

Wait states and machine-specific contention should be handled by the `Bus` implementation returning non-zero from `get_memory_wait_states()` or `get_io_wait_states()`.

## Interrupts

- **NMI**: Highest priority. Sampled at end of instruction. Jumps to `0x0066`.
- **Maskable Interrupt (INT)**: Sampled if `IFF1` is set.
  - **Mode 0**: Fetches instruction from bus (usually `RST`).
  - **Mode 1**: Jumps to `0x0038`.
  - **Mode 2**: Vectored interrupt using `I` register and bus data.

## Undocumented Features Supported

- **SLL (Shift Left Logical)**: Opcode `0xCB 0x30-0x37`.
- **IXH / IXL / IYH / IYL**: High/Low bytes of index registers as 8-bit registers.
- **MEMPTR (WZ)**: Internal 16-bit register used for certain address calculations.
- **Q Factor**: Affects undocumented F3/F5 flags during `SCF`/`CCF`.
- **Undocumented IM**: Opcode aliases for `IM 0`, `IM 1`, and `IM 2`.

## Python Bindings

Uses `nanobind` to expose the C++ classes.
- `z80_core.Z80`: Main CPU class.
- `z80_core.Registers`: Register access.
- `z80_core.SimpleBus`: A basic memory-backed bus for testing.

Building with Python support:
```bash
cmake -DENABLE_NANOBIND=ON ..
```

## Testing Strategy

1. **Unit Tests**: Test individual instructions for functional correctness.
2. **Timing Tests**: Verify T-state counts for every instruction variant.
3. **Exercisers**:
   - **ZEXDOC**: Verifies documented instruction flags and logic.
   - **ZEXALL**: Verifies all instruction (including undocumented) flags and logic.
   - Run via `pytest tests/test_z80_exerciser.py`.
