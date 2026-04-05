# Changelog

All notable changes to this project will be documented in this file.

## [2.2.0] — 2026-04-05

### Added

- Test suite reorganized into 28 small test files for better maintainability
- New tests: DD/FD prefix fallthrough, Q factor, CCF H flag, comprehensive DAA, IX/IY edge cases

### Changed

- **Test organization**: Split monolithic `test_validate_z80.py` (4800+ lines) into ~28 focused test files in `tests/`
- **Test fixtures**: New `conftest.py` with shared fixtures (`cpu`, `write_program`, `step_n`, flag helpers)
- **`pyproject.toml`**: Updated `python_files` pattern to discover `test_*.py` files

### Fixed

- **IM 0**: Now handles full instruction execution (not just RST vectors)
- **Q factor**: Fixed to check `old_f != new_f` before setting Q flags (Patrik Rak discovery)
- **NEG**: Added missing F3/F5 flag handling
- **_in_out_flags**: Added F3/F5 handling
- **MEMPTR**: Fixed on numerous instructions (JP, CALL, RET, RST, EX (SP),HL, PUSH/POP IX/IY, IN/OUT (C),r, block I/O, CP, LD SP,HL, LD rr,nn)
- **IM 2 timing**: Fixed (was returning 13T, should be 19T)
- **DD/FD fallthrough**: Fixed chained prefix handling (DD DD NOP now works)
- **Parity table**: 0xFF has even parity (corrected test expectation)
- **XOR F3/F5**: Flags come from result, not operand (matching real Z80)
- **RLD/RRD**: Carry flag preserved from previous state
- **DDCB RL/RR**: Carry flag reflects bit 0 of result (not cleared)
- **CPIR**: Sets Z flag when match is found
- **Block instructions**: Use `step_n()` for repeat behavior
- **EI deferral**: IFF1 enabled after NEXT instruction (not immediately)
- **JR backward wrap**: PC wraps to 0xFFFF (not 0x0000)

## [2.1.0] — 2026-04-04

### Added

- Comprehensive timing tests for all DD/FD prefixed instructions (IX/IY), including 16-bit loads, ALU, stack, indexed memory ops, IXH/IXL 8-bit ops, DDCB/FDCB rotates, bits, SET/RES
- IY timing test suite (mirrors IX timing tests)
- `CycleType` enum in bus interface (`M1`, `MEM_RD`, `MEM_WR`, `IO_RD`, `IO_WR`, `INT_ACK`) for cycle-accurate bus tracking
- NMI handling in `step()` with proper HALT exit and return address (PC+1 past HALT)

### Changed

- **Handler architecture**: All instruction handlers changed from `int` return type to `void` with internal cycle accounting via `_bus_fetch()`, `_bus_read()`, `_bus_write()`, and `_wait()` — enables machine-independent cycle counting
- **Bus interface**: `_bus_read`, `_bus_write`, `_bus_io_read`, `_bus_io_write` now use explicit `CycleType` parameter for bus callback differentiation
- **`_bus_fetch()`**: New 4T M1 cycle helper that increments R register — used for opcode fetches
- **DD/FD handlers**: All indexed instruction handlers now properly consume the opcode byte with `_bus_fetch()` (4T M1) and include correct internal wait states for every instruction variant
- **DDCB/FDCB handlers**: Fixed byte order (CB prefix consumed before displacement) and proper timing for ROT (23T), BIT (20T), SET/RES (23T)
- **HALT**: PC is decremented so interrupt return address is correctly PC+1 (past the HALT instruction)
- **Test fixture**: CPU registers now initialize with F=0, A=0 for deterministic conditional instruction testing
- **Timing tests**: Conditional instruction timing tests (JR Z, CALL C, RET NZ, etc.) now properly set flags via setup parameters

### Fixed

- **LD (IX+d),n**: 19T (was 15T — missing internal wait states)
- **INC/DEC (IX+d)**: 23T (was 19T — missing internal wait states)
- **LD SP,IX/IY**: 10T (was 8T — missing internal wait)
- **JP (IX/IY)**: 8T (was 4T — missing internal wait)
- **INC/DEC IX/IY**: 10T (was 6T — missing internal wait)
- **PUSH IX/IY**: 15T (was 11T — missing internal wait)
- **POP IX/IY**: 14T (was 10T — missing internal wait)
- **EX (SP),IX/IY**: 23T (was 19T — missing internal wait)
- **LD (nn),IX/IY**: 20T (was 16T — missing internal wait)
- **LD IX/IY,(nn)**: 20T (was 16T — missing internal wait)
- **INC/DEC IXH/IXL/IYH/IYL**: 8T (was 4T — missing internal wait)
- **LD IXH/IXL,n**: 11T (was 7T — missing internal wait)
- **LD r,IXH/IXL and LD IXH/IXL,r**: 8T (was 4T — missing internal wait)
- **ALU A,IXH/IXL**: 8T (was 4T — missing internal wait)
- **ALU A,(IX+d)**: 19T (was 15T — missing internal wait)
- **LD IX,(nn)**: Fixed to read memory into IX instead of writing IX to memory
- **DDCB SET/RES**: Fixed byte order and memory write operations
- **NMI**: Was completely missing from `step()` — now properly handles NMI with 11T timing, IFF1 clear, and HALT exit

## [2.0.0] — 2026-04-03

### Breaking Changes

- **Removed nanobind dependency** — bindings now use raw CPython API exclusively
- **Removed `SimpleBus` from public API** — memory is now managed internally by `PythonBus`
- **Removed `Registers` class from public API** — registers are now accessed directly on `Z80CPU`
- **Removed flag table exports** (`PARITY_TABLE`, `ADD_FLAGS`, `SUB_FLAGS`, `SBC_FLAGS`, `ADC_FLAGS`) — use flag constants instead
- **`step()` return value changed** — now returns total T-states consumed including contention delays added by bus callbacks, not just the instruction's base T-states
- **`run_frame()` no longer auto-triggers interrupts** — machines must call `trigger_interrupt()` explicitly at the correct timing

### Added

- Flat register access on CPU object: `cpu.A`, `cpu.F`, `cpu.B`, `cpu.C`, `cpu.D`, `cpu.E`, `cpu.H`, `cpu.L`, `cpu.PC`, `cpu.SP`, `cpu.IX`, `cpu.IY`, `cpu.BC`, `cpu.DE`, `cpu.HL`, `cpu.AF`, `cpu.IFF1`, `cpu.IFF2`, `cpu.IM`, and all alternate registers
- `cpu.run(max_cycles)` — batch cycle execution for performance-critical main loops
- `cpu.run_instructions(count)` — deterministic instruction stepping for debuggers
- `cpu.io_read(port)` — direct I/O port read for machine code
- `cpu.io_write(port, value)` — direct I/O port write for machine code
- `cpu.current_opcode` — read-only property exposing the last executed opcode byte
- `cpu.last_read_addr` — read-only property exposing the last memory read address (bus snooping)
- `cpu.add_cycles(count)` — inject contention T-states from machine code
- `cpu.set_on_reti_callback(cb)` — RETI callback for interrupt daisy chaining (Z80PIO/Z80CTC)
- `cpu.set_on_get_int_vector_callback(cb)` — machine provides interrupt vector on demand
- `cpu.mark_addrs(addr, size, marks)` — mark memory ranges for breakpoints/self-mod code
- `cpu.unmark_addrs(addr, size, marks)` — clear memory marks
- `cpu.get_addr_mark(addr)` — query marks at a specific address
- `FLAG_F5` and `FLAG_F3` constants for undocumented flag bits
- `MACHINE_STATE_SIZE` constant (reserved for future save state support)
- `PythonBus` with internal 64KB memory — no external bus object required

### Changed

- Build system: replaced `nanobind_add_module` with `Python_add_library`
- `pyproject.toml`: removed `nanobind>=2.0` from build requirements
- `core/__init__.py`: cleaned exports to machine-agnostic API only
- Test suite: updated all tests to use flat register API and I/O callbacks
- Benchmark tools: updated to machine-agnostic API

### Removed

- `test_z80.py` — dead test file
- nanobind dependency and all nanobind-specific code
- Fallback import of `pyz80_python` module for flag tables and `Registers`

### Fixed

- Memory view null pointer segfault when using non-SimpleBus configurations
- I/O callbacks not being invoked when CPU created without explicit bus object
- Missing `IFF1`, `IFF2`, `IM` direct access on CPU object
- `run_frame()` unconditionally triggering interrupt with hardcoded vector 0xFF
