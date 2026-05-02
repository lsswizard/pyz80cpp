#pragma once

#include <cstdint>
#include <string>

namespace z80 {

// ============================================================
// Bus Interface — pure virtual, machine-independent
//
// Override read() and write() for memory.
// Override in_() and out_() for I/O.
// All timing is handled by the Z80 class; contend() is
// an optional hook for machines with memory contention.
// ============================================================
class Bus {
public:
    virtual ~Bus() = default;

    // Memory
    virtual uint8_t read(uint16_t addr) = 0;
    virtual void    write(uint16_t addr, uint8_t val) = 0;

    // I/O — override in_() / out_() (not in() / out())
    virtual uint8_t in_(uint16_t port)           { (void)port; return 0xFF; }
    virtual void    out_(uint16_t port, uint8_t val) { (void)port; (void)val; }

    // Contention — called by the CPU with the address and base cycle count
    // of the access. Override for machines with contended memory (e.g. ZX Spectrum).
    virtual void contend(uint16_t addr, int cycles) { (void)addr; (void)cycles; }

    // M1 hook — called at the start of every instruction-fetch cycle
    virtual void m1_cycle() {}

    // Interrupt acknowledge — return the vector byte placed on the data bus.
    // For IM 0 / IM 2. Typically 0xFF (NOP) or the low byte of the vector.
    virtual uint8_t interrupt_acknowledge() { return 0xFF; }

    // Optional wait-state queries (machine-independent timing extension)
    virtual int get_memory_wait_states(uint16_t addr) { (void)addr; return 0; }
    virtual int get_io_wait_states(uint16_t port)     { (void)port; return 0; }

    // Optional debug hook
    virtual std::string debug_info() const { return {}; }
};

// ============================================================
// SimpleBus — 64 KB flat memory, 256-byte I/O space
// ============================================================
class SimpleBus : public Bus {
public:
    uint8_t memory[65536] = {};
    uint8_t io_ports[256] = {};

    uint8_t read(uint16_t addr) override           { return memory[addr]; }
    void    write(uint16_t addr, uint8_t val) override { memory[addr] = val; }
    uint8_t in_(uint16_t port) override            { return io_ports[port & 0xFF]; }
    void    out_(uint16_t port, uint8_t val) override  { io_ports[port & 0xFF] = val; }
};

} // namespace z80
