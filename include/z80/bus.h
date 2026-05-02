#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <cstring>

namespace z80 {

// ============================================================
// Bus Interface — machine-independent
// ============================================================
class Bus {
public:
    virtual ~Bus() = default;

    // Direct access buffers (Option 3 - Memory-Mapped Bus)
    uint8_t memory[65536] = {};
    uint8_t io_ports[65536] = {};

    // Memory — default implementation uses the internal buffer
    virtual uint8_t read(uint16_t addr)           { return memory[addr]; }
    virtual void    write(uint16_t addr, uint8_t val) { memory[addr] = val; }

    // I/O — default implementation uses the internal buffer (16-bit)
    virtual uint8_t in_(uint16_t port)           { return io_ports[port]; }
    virtual void    out_(uint16_t port, uint8_t val) { io_ports[port] = val; }

    // Contention — called by the CPU with the address and base cycle count
    virtual void contend(uint16_t addr, int cycles) { (void)addr; (void)cycles; }

    // M1 hook — called at the start of every instruction-fetch cycle
    virtual void m1_cycle() {}

    // Interrupt acknowledge — return the vector byte placed on the data bus.
    virtual uint8_t interrupt_acknowledge() { return 0xFF; }

    // Optional wait-state queries (legacy - for flat wait states per access)
    virtual int get_memory_wait_states(uint16_t addr) { (void)addr; return 0; }
    virtual int get_io_wait_states(uint16_t port)     { (void)port; return 0; }

    // Per-M-cycle contention (for cycle-accurate machines like ZX Spectrum)
    // cycle_in_m = position within current M-cycle (0 = first T-state)
    // Returns additional wait states for this specific cycle
    virtual int get_contention_wait_states(uint16_t addr, int cycle_in_m, int m_cycle_number) {
        (void)addr; (void)cycle_in_m; (void)m_cycle_number;
        return get_memory_wait_states(addr);
    }

    virtual std::string debug_info() const { return {}; }
};

// SimpleBus is now just a synonym for Bus
class SimpleBus : public Bus {};

} // namespace z80
