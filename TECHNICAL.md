# Technical Reference — PyZ80 C++ Core

## Architecture

```
┌──────────────────────────────────────────────┐
│  Python Machine Code                         │
│  - Memory mapping (ROM/RAM/IO)               │
│  - I/O device emulation (keyboard, PSG, etc.)│
│  - Contention timing (ULA, wait states)      │
│  - Interrupt timing and vector assertion     │
│  - Video/audio sync                          │
└──────────────────┬───────────────────────────┘
                   │ Python callbacks
                   │ trigger_interrupt(data)
                   │ trigger_nmi()
                   │ add_cycles(count)
                   │ io_read(port) / io_write(port, value)
┌──────────────────▼───────────────────────────┐
│  C++ Z80 Core (machine-agnostic)             │
│  - Cycle-exact instruction execution         │
│  - Decoder with opcode cache                 │
│  - Flag computation tables                   │
│  - All instruction variants                  │
│  - EI deferral, Q factor, undocumented       │
└──────────────────────────────────────────────┘
```

The C++ core contains **zero** machine-specific code. All machine behavior is implemented in Python via callbacks and direct API calls.

## Building

### Requirements

- CMake 3.20+
- C++20 compatible compiler (GCC 11+, Clang 13+)
- Python 3.12+ with development headers

### Build Steps

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cp _pyz80.so ../core/_pyz80.cpython-314-x86_64-linux-gnu.so
```

The `.so` filename must match your Python version's ABI tag. Use `python3 -c "import sysconfig; print(sysconfig.get_config_var('EXT_SUFFIX'))"` to find the correct suffix.

### Build System

- `CMakeLists.txt` uses `Python_add_library()` from `FindPython` — no nanobind or other binding libraries
- `z80_core` static library contains the pure C++ CPU core
- `_pyz80` shared module links against `z80_core` and Python

## Python API Reference

### Z80CPU

```python
from core import Z80CPU

cpu = Z80CPU()                    # Creates CPU with internal 64KB memory
cpu = Z80PythonBus(py_bus_obj)    # Creates CPU with custom Python bus object
```

#### Execution Methods

| Method | Returns | Description |
|---|---|---|
| `step()` | `int` | Execute one instruction. Returns T-states consumed (including contention). |
| `run(max_cycles)` | `int` | Execute instructions until `max_cycles` T-states consumed or halted. Returns actual T-states consumed. |
| `run_instructions(count)` | `int` | Execute exactly `count` instructions or until halted. Returns total T-states. |
| `run_frame(t_states)` | `int` | Execute until `t_states` T-states are reached from current cycle count. |
| `add_cycles(count)` | `None` | Add `count` T-states to the cycle counter (for contention injection). |
| `reset()` | `None` | Reset CPU to power-on state. |

#### Register Access (Flat API)

All registers are directly readable and writable as properties on the CPU object:

**8-bit registers:** `A`, `F`, `B`, `C`, `D`, `E`, `H`, `L`
**Alternate registers:** `Ap`, `Fp`, `Bp`, `Cp`, `Dp`, `Ep`, `Hp`, `Lp`
**Control registers:** `I`, `R`, `Q`, `LAST_Q`
**16-bit registers:** `PC`, `SP`, `IX`, `IY`, `MEMPTR`
**Compound registers:** `BC`, `DE`, `HL`, `AF` (read/write as 16-bit values)
**Interrupt flags:** `IFF1`, `IFF2`, `IM`

```python
cpu.A = 0x42        # Set accumulator
cpu.PC = 0x8000     # Set program counter
cpu.HL = 0xC000     # Set HL pair
cpu.IFF1 = True     # Enable interrupts
cpu.IM = 2          # Set interrupt mode 2
```

#### CPU State

| Property | Type | Description |
|---|---|---|
| `cycles` | `int` | T-state counter (read/write) |
| `halted` | `bool` | Whether CPU is in HALT state (read/write) |
| `instruction_count` | `int` | Total instructions executed (read/write) |
| `interrupt_pending` | `bool` | Maskable interrupt pending flag (read/write) |
| `nmi_pending` | `bool` | NMI pending flag (read/write) |
| `interrupt_data` | `int` | Data bus value for next interrupt (read/write) |
| `current_opcode` | `int` | Last executed opcode byte (after prefix resolution, read-only) |
| `last_read_addr` | `int` | Last memory address read by the CPU (read-only, for bus snooping) |
| `regs` | `Regs` | Nested register object (alternative to flat API) |

#### Memory Access

| Method | Description |
|---|---|
| `read_byte(addr)` | Read a byte from memory at `addr`. Goes through bus callbacks if using a Python bus. |
| `write_byte(addr, value)` | Write a byte to memory at `addr`. Goes through bus callbacks if using a Python bus. |
| `_get_memory_view()` | Returns a writable `memoryview` of the internal 64KB memory array. Fast bulk access. |

#### I/O Access

| Method | Description |
|---|---|
| `io_read(port)` | Read from I/O port. Invokes the input callback if set. |
| `io_write(port, value)` | Write to I/O port. Invokes the output callback if set. |

#### Interrupt Control

| Method | Description |
|---|---|
| `trigger_interrupt(data)` | Assert the INT pin with the given data bus value. The interrupt is serviced on the next `step()` if IFF1 is set. |
| `trigger_nmi()` | Assert the NMI pin. The NMI is serviced on the next `step()`. |

#### I/O Callbacks

These callbacks are invoked by the CPU when executing `IN`/`OUT` instructions:

```python
cpu.set_on_input_callback(lambda port: value)
cpu.set_on_output_callback(lambda port, value: None)
```

The callbacks receive the 16-bit port address. The input callback must return an 8-bit value. The output callback receives the value being written.

#### Advanced Callbacks

| Method | Description |
|---|---|
| `set_on_reti_callback(cb)` | Called when RETI executes. Used for interrupt daisy chain release (Z80PIO, Z80CTC). |
| `set_on_get_int_vector_callback(cb)` | Called during INT acknowledge. Returns the interrupt vector byte for IM2. Alternative to `trigger_interrupt(data)`. |

```python
cpu.set_on_reti_callback(lambda: pio_release_daisy_chain())
cpu.set_on_get_int_vector_callback(lambda: 0x42)  # Vector byte for IM2
```

#### Memory Marking

Mark memory ranges with bit flags for breakpoints, self-modifying code detection, or code/data tagging:

| Method | Description |
|---|---|
| `mark_addrs(addr, size, marks)` | Set bit flags on a memory range. |
| `unmark_addrs(addr, size, marks)` | Clear bit flags from a memory range. |
| `get_addr_mark(addr)` | Get the current mark byte at an address. |

```python
# Mark ROM region
cpu.mark_addrs(0x0000, 0x4000, 0x01)  # 0x01 = ROM

# Mark code region for self-mod detection
cpu.mark_addrs(0x8000, 0x2000, 0x02)  # 0x02 = CODE

# Check if address is marked
if cpu.get_addr_mark(0x8100) & 0x02:
    cpu.invalidate_all()  # Self-modifying code detected
```

#### Decoder Cache

For self-modifying code, invalidate the decoder cache after writing to memory:

| Method | Description |
|---|---|
| `invalidate_all()` | Invalidate all cached opcode decodings. |
| `invalidate_range(start, end)` | Invalidate cached decodings for the given address range. |

### Constants

```python
from core import FLAG_S, FLAG_Z, FLAG_H, FLAG_PV, FLAG_N, FLAG_C, FLAG_F5, FLAG_F3
```

| Constant | Value | Description |
|---|---|---|
| `FLAG_S` | `0x80` | Sign flag (bit 7) |
| `FLAG_Z` | `0x40` | Zero flag (bit 6) |
| `FLAG_F5` | `0x20` | Undocumented bit 5 |
| `FLAG_H` | `0x10` | Half-carry flag (bit 4) |
| `FLAG_F3` | `0x08` | Undocumented bit 3 |
| `FLAG_PV` | `0x04` | Parity/Overflow flag (bit 2) |
| `FLAG_N` | `0x02` | Add/Subtract flag (bit 1) |
| `FLAG_C` | `0x01` | Carry flag (bit 0) |
| `MACHINE_STATE_SIZE` | `0` | Reserved for future save state support |

## Timing Model

### T-State Accuracy

Every instruction executes in exactly the number of T-states specified by the Zilog Z80 User Manual. The `step()` method returns the actual T-states consumed, which may be higher than the base instruction T-states if contention was injected via `add_cycles()` during execution.

### Contention

Memory contention (e.g., ZX Spectrum ULA access patterns) is modeled by calling `add_cycles(n)` from within I/O callbacks or from the machine's main loop:

```python
def bus_read(self, addr, t_state):
    # Add ULA contention for certain address ranges
    if 0x4000 <= addr < 0x8000:
        self.cpu.add_cycles(ula_contention(addr, t_state))
    return self.memory[addr]
```

### Interrupt Timing

The CPU does **not** auto-generate interrupts. The machine must:

1. Call `trigger_interrupt(data)` to assert INT with the appropriate data bus value, or set `set_on_get_int_vector_callback(cb)` to provide the vector on demand
2. The interrupt is sampled on the next `step()` after the current instruction completes
3. If IFF1 is set, the interrupt is acknowledged (EI deferral is respected)

For IM 2, the `data` parameter (or the value returned by the vector callback) determines the vector table address: `vector_addr = (I << 8) | (data & 0xFE)`.

### Interrupt Daisy Chaining

Systems with multiple Z80 peripherals (Z80PIO, Z80CTC, Z80SIO) use a daisy chain to arbitrate interrupts. Set the RETI callback to release the chain:

```python
cpu.set_on_reti_callback(lambda: daisy_chain_released())
```

### EI Deferral

After executing `EI`, interrupts are not accepted until after the next instruction completes. This is modeled internally via `EI_PENDING` and `EI_JUST_RESOLVED` flags.

## Undocumented Behavior

### Q Factor

The Q factor tracks whether the previous instruction modified flags. This affects the behavior of `SCF` and `CCF` on undocumented F3/F5 bits:

- If the previous instruction modified flags, `SCF`/`CCF` copy the F3/F5 bits from the A register
- Otherwise, they preserve the existing F3/F5 bits

### F3/F5 Flags

Undocumented bits 3 and 5 of the F register are set based on the corresponding bits of the result (for most ALU operations) or the operand (for `CP`).

### DD/FD Prefix Fallthrough

When a `DD` or `FD` prefix is followed by an instruction that doesn't use IX/IY, the prefix is consumed (2 T-states, R+2) and the base instruction executes normally.

### LD A,I / LD A,R Interrupt Bug

If an interrupt occurs immediately after `LD A,I` or `LD A,R`, the P/V flag is cleared because IFF2 is cleared before P/V can be set from the interrupt response.

## C++ Core Internals

### File Structure

```
cpp/core/
  cpu.h          — CPU class declaration, inline bus access helpers
  cpu.cpp        — step(), run(), run_frame(), interrupt handling
  registers.h    — Registers struct with compound accessors
  bus.h          — Bus interface, SimpleBus implementation
  flags.h        — Flag computation tables (COND_TABLE, etc.)
  decoder.h      — Opcode decoder with caching
  handlers.cpp   — All instruction handler implementations
```

### Decoder

The decoder caches decoded instructions for fast lookup. When `_is_simple_bus` is true, it decodes directly from the memory array. For custom buses, it falls back to handler table lookups.

Self-modifying code requires calling `invalidate_all()` or `invalidate_range()` after memory writes.

### Handler System

Instructions are implemented as handler functions that take a `CPU&` reference and return T-states. Handlers are organized into tables:

- `base_handlers[256]` — standard opcodes
- `cb_handlers[256]` — CB-prefixed (bit operations, rotates, shifts)
- `ed_handlers[256]` — ED-prefixed (extended instructions)
- `dd_handlers[256]` / `fd_handlers[256]` — DD/FD-prefixed (IX/IY)
- `ddcb_handlers[256]` / `fdcb_handlers[256]` — DDCB/FDCB (indexed bit ops)
- `dd_ed_handlers[256]` / `fd_ed_handlers[256]` — DD ED / FD ED prefixed

### Fast Path

When using the default `PythonBus` (no custom bus object), `_is_simple_bus` is enabled and the decoder reads directly from the internal memory array, avoiding virtual function calls.

## Testing

```bash
python3 -m pytest tests/ -v
```

445 tests covering:
- 8-bit and 16-bit loads
- All ALU operations (ADD, ADC, SUB, SBC, AND, OR, XOR, CP)
- INC/DEC (8-bit and 16-bit)
- Rotates and shifts (RLCA, RRCA, RLA, RRA, CB rotates/shifts)
- Bit operations (BIT, SET, RES)
- Jumps (JP, JR, DJNZ, conditional variants)
- Calls and returns (CALL, RET, RST, conditional variants)
- Stack operations (PUSH, POP)
- Exchange (EX, EXX)
- Block operations (LDI, LDIR, CPI, INI, IND, OUTI, OUTD, INIR, OTIR)
- I/O instructions (IN, OUT)
- Interrupts (NMI, IM 0/1/2, DI, EI, REtn)
- Indexed operations (IX, IY, DDCB, FDCB)
- ED instructions (LD I/R, RLD, RRD, NEG, IM)
- HALT, NOP, SLL
- DAA (BCD adjust)
- Timing verification for every instruction
- Undocumented flags (F3, F5, Q factor)
- DD/FD prefix fallthrough
- Edge cases (wrap-around, overflow, R register)
- Integration tests (memcpy, array sum, call/ret)
