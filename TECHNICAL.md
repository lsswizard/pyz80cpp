# Technical Reference — Z80Core C++

## Architecture

The Z80Core is a cycle-accurate, machine-independent Z80 CPU emulator. The core contains **zero machine-specific code** — all memory, I/O, and timing behavior is injected through the `Bus` interface.

```
┌──────────────────────────────────────────────┐
│  Host Application (C++ or Python)            │
│  - Implements Bus interface                  │
│  - Manages Memory, I/O, paging, devices      │
│  - Builds timing tables for contention       │
│  - Handles Video/Audio Generation            │
└──────────────────┬───────────────────────────┘
                   │
                   │ Virtual calls: read(), write(), in_(), out_()
                   │ Timing: access-type-specific delay tables + masks
                   │
┌──────────────────▼───────────────────────────┐
│  Z80Core (C++ Static Library)                │
│  - Cycle-accurate instruction handlers       │
│  - Base T-state accounting                   │
│  - Wait states from Bus timing tables        │
│  - Full Register set (Main, Alt, Index)      │
│  - Internal opcode lookup tables             │
│  - step() returns actual cycles (base+wait)  │
└──────────────────────────────────────────────┘
```

## Core Components

### 1. Register File (`Registers` struct)
Located in `include/z80/registers.h`.
- Supports 8-bit (`A`, `F`, `B`, `C`, ...) and 16-bit (`AF`, `BC`, `HL`, `IX`, `IY`, `PC`, `SP`) access.
- Includes undocumented `MEMPTR` (WZ) and `Q` flag factor.
- `IFF1`, `IFF2` interrupt flip-flops; `EI_PENDING` / `EI_JUST_RESOLVED` for two-phase EI.

### 2. Bus Interface (`Bus` class)
Located in `include/z80/bus.h`.
- All memory and I/O access passes through this interface.
- Methods: `read(addr)`, `write(addr, val)`, `in_(port)`, `out_(port, val)`.
- `interrupt_acknowledge()`: Called during INT service to fetch vector.
- `extra_cycles(kind, addr, t_state)`: Access-type-specific wait state query (overridable fallback).
- **Timing tables**: `fetch_delay_table`, `mem_read_delay_table`, `mem_write_delay_table`, `io_read_delay_table`, `io_write_delay_table` — 5 pre-computed tables copied into owned `std::vector` storage.
- **Contention masks**: Per-access-type 16-bit bitmask (1 bit per 4KB region) to determine which regions are contended.
- **Fast path**: `fast_memory_ptr` for direct memory reads (bypasses virtual call).

### 3. Opcode Tables
Located in `src/opcode_table.cpp`.
- `main_table`: Standard opcodes.
- `cb_table`: Bitwise/Rotate/Shift.
- `ed_table`: Extended instructions.
- `dd_table` / `fd_table`: IX/IY prefixed instructions.
- `ddcb_table` / `fdcb_table`: Indexed bitwise operations.

## Cycle-Accuracy and Timing

Each access type has a base T-state count plus optional wait states from the Bus timing tables:

| Access Type | Base T-States | Named Constant |
|-------------|---------------|----------------|
| Opcode Fetch (M1) | 4 | `AccessKind::OpcodeFetch` |
| Memory Read | 3 | `AccessKind::MemoryRead` |
| Memory Write | 3 | `AccessKind::MemoryWrite` |
| I/O Read | 4 | `AccessKind::IORead` |
| I/O Write | 4 | `AccessKind::IOWrite` |
| Interrupt Acknowledge | 6 | `AccessKind::InterruptAck` |

Wait states are computed per access by looking up the current T-state in the corresponding delay table. The CPU calls `get_extra_cycles(kind, addr, t_state)` which:
1. Checks the contention mask for the address region
2. Looks up the pre-computed delay table value for the current T-state
3. Falls back to the virtual `extra_cycles()` method if no fast table is set

The handler-level `cpu.wait(n)` call adds idle internal T-states. `step()` returns the sum of all base + wait + idle T-states consumed.

### Machine-Specific Timing (Python Side)
Machines build 5 timing tables (one per access type) and copy them into the Bus:
```python
bus.set_timing_tables(fetch_tbl, mem_r_tbl, mem_w_tbl, io_r_tbl, io_w_tbl)
bus.fetch_contention_mask = 0x0008  # 0x4000-0x7FFF
```

Tables are rebuilt when paging changes (port 0x7FFD writes affect bank contention).
48K timing is static — build once at init. 128K timing requires invalidation on
paging changes (`_timing_dirty` flag).

## Interrupts

- **NMI**: Highest priority (11 T-states). Jumps to `0x0066`.
- **Maskable Interrupt (INT)**: Sampled if `IFF1` is set and `EI_JUST_RESOLVED` is clear.
  - **Mode 0**: Fetches instruction from bus — RST handled (13 T-states).
  - **Mode 1**: Jumps to `0x0038` (13 T-states).
  - **Mode 2**: Vectored interrupt using `I` register and bus data (19 T-states).
- Interrupts are sampled at instruction boundaries (not mid-instruction).
- EI defers interrupt enable by one instruction (two-phase via `EI_PENDING`/`EI_JUST_RESOLVED`).

## Undocumented Features Supported

- **SLL (Shift Left Logical)**: Opcode `0xCB 0x30-0x37`.
- **IXH / IXL / IYH / IYL**: High/Low bytes of index registers as 8-bit registers.
- **MEMPTR (WZ)**: Internal 16-bit register used for address calculations.
- **Q Factor**: Affects undocumented F3/F5 flags during `SCF`/`CCF`.
- **Undocumented IM**: Opcode aliases for `IM 0`, `IM 1`, and `IM 2`.
- **DDCB/FDCB register storage**: Result written to both (IX+d) and destination register.

## Python Bindings

Uses **pybind11** to expose the C++ classes (nanobind is also available but not default).
- `z80_core.Z80`: Main CPU class.
- `z80_core.Registers`: Register access.
- `z80_core.SimpleBus`: A basic memory-backed bus for testing.
- `z80_core.AccessKind`: Enum with `OpcodeFetch`, `MemoryRead`, `MemoryWrite`, `IORead`, `IOWrite`, `InterruptAck`.

Building with Python support:
```bash
cmake -B build -DENABLE_PYBIND11=ON
cmake --build build
cp build/z80_core*.so /path/to/site-packages/
```

## Testing Strategy

1. **Unit Tests** (1390+): Test individual instructions for functional correctness.
2. **Timing Tests** (262): Verify T-state counts for every instruction variant.
3. **Exercisers**: ZEXDOC / ZEXALL — verifies documented and undocumented instruction flags and logic.
