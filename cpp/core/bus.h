#pragma once

#include <cstdint>
#include <cstring>

class Bus {
public:
    virtual ~Bus() = default;
    virtual uint8_t bus_read(uint16_t addr, int t_state) = 0;
    virtual void bus_write(uint16_t addr, uint8_t value, int t_state) = 0;
    virtual uint8_t bus_io_read(uint16_t port, int t_state) = 0;
    virtual void bus_io_write(uint16_t port, uint8_t value, int t_state) = 0;
};

class SimpleBus : public Bus {
public:
    uint8_t memory[65536];
    uint8_t io_ports[256];

    // Helper class for Python access to io_ports
    class IOPorts {
        uint8_t* _data;
    public:
        IOPorts(uint8_t* data) : _data(data) {}
        uint8_t get(uint8_t port) { return _data[port]; }
        void set(uint8_t port, uint8_t val) { _data[port] = val; }
    };

    IOPorts _io_ports_wrapper{io_ports};

    SimpleBus();

    uint8_t bus_read(uint16_t addr, int t_state) override;
    void bus_write(uint16_t addr, uint8_t value, int t_state) override;
    uint8_t bus_io_read(uint16_t port, int t_state) override;
    void bus_io_write(uint16_t port, uint8_t value, int t_state) override;

    uint8_t& operator[](int addr) { return memory[addr & 0xFFFF]; }
    const uint8_t& operator[](int addr) const { return memory[addr & 0xFFFF]; }

    IOPorts& get_io_ports() { return _io_ports_wrapper; }
};
