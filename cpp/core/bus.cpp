#include "bus.h"
#include <cstring>

SimpleBus::SimpleBus() {
    std::memset(memory, 0, sizeof(memory));
    std::memset(io_ports, 0xFF, sizeof(io_ports));
}

uint8_t SimpleBus::bus_read(uint16_t addr, int /*t_state*/) {
    return memory[addr & 0xFFFF];
}

void SimpleBus::bus_write(uint16_t addr, uint8_t value, int /*t_state*/) {
    memory[addr & 0xFFFF] = value;
}

uint8_t SimpleBus::bus_io_read(uint16_t port, int /*t_state*/) {
    return io_ports[port & 0xFF];
}

void SimpleBus::bus_io_write(uint16_t port, uint8_t value, int /*t_state*/) {
    io_ports[port & 0xFF] = value;
}