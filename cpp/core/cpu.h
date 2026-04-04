#pragma once

#include "registers.h"
#include "bus.h"
#include "decoder.h"
#include "flags.h"
#include <cstdint>

class CPU {
public:
    Registers regs;
    Bus* bus;
    int cycles;
    int instruction_count;
    bool halted;
    uint8_t interrupt_data;
    bool interrupt_pending;
    bool nmi_pending;
    uint8_t current_opcode;
    bool _is_iy;

    uint8_t* _mem;
    bool _is_simple_bus;
    bool _owns_bus;

    Decoder decoder;

    bool _pc_modified;
    bool _is_ld_a_ir;

    CPU(Bus* bus = nullptr);
    ~CPU();
    void reset();
    int step();
    int run(int max_cycles);
    int run_instructions(int count);
    int run_frame(int t_states_per_frame);
    void add_cycles(int count) { cycles += count; }
    void trigger_interrupt(uint8_t data);
    void trigger_nmi();
    uint8_t read_reg8(int reg);
    void write_reg8(int reg, uint8_t value);

    // Cycle-accurate bus access
    inline uint8_t _bus_fetch(uint16_t addr) {
        // M1 cycle: 4 T-states
        // R increments during M1 fetch
        regs.R = (regs.R & 0x80) | ((regs.R + 1) & 0x7F);
        uint8_t val;
        if (_is_simple_bus) {
            val = _mem[addr & 0xFFFF];
        } else {
            val = bus->bus_read(addr, cycles, CycleType::M1);
        }
        cycles += 4;
        return val;
    }

    inline uint8_t _bus_read(uint16_t addr) {
        // Memory Read cycle: 3 T-states
        uint8_t val;
        if (_is_simple_bus) {
            val = _mem[addr & 0xFFFF];
        } else {
            val = bus->bus_read(addr, cycles, CycleType::MEM_RD);
        }
        cycles += 3;
        return val;
    }

    inline void _bus_write(uint16_t addr, uint8_t value) {
        // Memory Write cycle: 3 T-states
        if (_is_simple_bus) {
            _mem[addr & 0xFFFF] = value;
        } else {
            bus->bus_write(addr, value, cycles, CycleType::MEM_WR);
        }
        cycles += 3;
    }

    inline uint8_t _bus_io_read(uint16_t port) {
        // I/O Read cycle: 4 T-states
        uint8_t val = bus->bus_io_read(port, cycles);
        cycles += 4;
        return val;
    }

    inline void _bus_io_write(uint16_t port, uint8_t value) {
        // I/O Write cycle: 4 T-states
        bus->bus_io_write(port, value, cycles);
        cycles += 4;
    }

    inline void _wait(int t_states) {
        cycles += t_states;
    }

    inline bool check_condition(int cc) {
        return z80flags::COND_TABLE[(regs.F << 3) | (cc & 0x07)];
    }

    // Indexed address helper
    inline uint16_t _get_indexed_addr(uint8_t displacement) {
        int16_t disp = (int8_t)displacement;
        uint16_t base = _is_iy ? regs.IY : regs.IX;
        uint16_t addr = (base + disp) & 0xFFFF;
        regs.MEMPTR = addr;
        return addr;
    }
};
