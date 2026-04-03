# Changelog

All notable changes to this project will be documented in this file.

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
- `cpu.add_cycles(count)` — inject contention T-states from machine code
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
