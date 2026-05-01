#include <nanobind/nanobind.h>
#include <string>
#include "../../include/z80/z80.h"
#include "../../include/z80/bus.h"

namespace nb = nanobind;
using namespace z80;

class PyBus : public Bus {
public:
    using Bus::Bus;

    uint8_t read(uint16_t addr) override {
        return Bus::read(addr);
    }

    void write(uint16_t addr, uint8_t val) override {
        Bus::write(addr, val);
    }

    uint8_t in_(uint16_t port) override {
        return Bus::in_(port);
    }

    void out_(uint16_t port, uint8_t val) override {
        Bus::out_(port, val);
    }

    void contend(uint16_t addr, int cycles) override {
        Bus::contend(addr, cycles);
    }

    void m1_cycle() override {
        Bus::m1_cycle();
    }

    uint8_t interrupt_acknowledge() override {
        return Bus::interrupt_acknowledge();
    }

    int get_memory_wait_states(uint16_t addr) override {
        return Bus::get_memory_wait_states(addr);
    }

    int get_io_wait_states(uint16_t port) override {
        return Bus::get_io_wait_states(port);
    }
};

NB_MODULE(z80_core, m) {
    m.doc() = "Z80 CPU core — machine-independent, cycle-accurate emulator";

    // ===============================================================
    // Bus
    // ===============================================================
    nb::class_<Bus, PyBus>(m, "Bus")
        .def(nb::init<>())
        .def("read", &Bus::read)
        .def("write", &Bus::write)
        .def("in_", &Bus::in_)
        .def("out_", &Bus::out_)
        .def("contend", &Bus::contend)
        .def("m1_cycle", &Bus::m1_cycle)
        .def("interrupt_acknowledge", &Bus::interrupt_acknowledge)
        .def("get_memory_wait_states", &Bus::get_memory_wait_states)
        .def("get_io_wait_states", &Bus::get_io_wait_states)
        .def_prop_rw("memory",
            [](Bus& s) -> nb::bytes {
                return nb::bytes((char*)s.memory, 65536);
            },
            [](Bus& s, nb::bytes data) {
                size_t len = std::min(data.size(), (size_t)65536);
                memcpy(s.memory, data.data(), len);
            })
        .def_prop_rw("io_ports",
            [](Bus& s) -> nb::bytes {
                return nb::bytes((char*)s.io_ports, 65536);
            },
            [](Bus& s, nb::bytes data) {
                size_t len = std::min(data.size(), (size_t)65536);
                memcpy(s.io_ports, data.data(), len);
            })
        .def("read_byte", &Bus::read)
        .def("write_byte", &Bus::write)
        .def("in_port", &Bus::in_)
        .def("out_port", &Bus::out_);

    nb::class_<SimpleBus, Bus>(m, "SimpleBus")
        .def(nb::init<>());

    // ===============================================================
    // Registers
    // ===============================================================
    nb::class_<Registers>(m, "Registers")
        .def(nb::init<>())
        .def_rw("A", &Registers::A)
        .def_rw("F", &Registers::F)
        .def_rw("B", &Registers::B)
        .def_rw("C", &Registers::C)
        .def_rw("D", &Registers::D)
        .def_rw("E", &Registers::E)
        .def_rw("H", &Registers::H)
        .def_rw("L", &Registers::L)
        .def_rw("Ap", &Registers::Ap)
        .def_rw("Fp", &Registers::Fp)
        .def_rw("Bp", &Registers::Bp)
        .def_rw("Cp", &Registers::Cp)
        .def_rw("Dp", &Registers::Dp)
        .def_rw("Ep", &Registers::Ep)
        .def_rw("Hp", &Registers::Hp)
        .def_rw("Lp", &Registers::Lp)
        .def_rw("IX", &Registers::IX)
        .def_rw("IY", &Registers::IY)
        .def_rw("SP", &Registers::SP)
        .def_rw("PC", &Registers::PC)
        .def_rw("I", &Registers::I)
        .def_rw("R", &Registers::R)
        .def_rw("IFF1", &Registers::IFF1)
        .def_rw("IFF2", &Registers::IFF2)
        .def_rw("IM", &Registers::IM)
        .def_rw("MEMPTR", &Registers::MEMPTR)
        .def_rw("Q", &Registers::Q)
        .def_prop_rw("BC",
            [](const Registers& r) { return r.BC(); },
            [](Registers& r, uint16_t v) { r.set_BC(v); })
        .def_prop_rw("DE",
            [](const Registers& r) { return r.DE(); },
            [](Registers& r, uint16_t v) { r.set_DE(v); })
        .def_prop_rw("HL",
            [](const Registers& r) { return r.HL(); },
            [](Registers& r, uint16_t v) { r.set_HL(v); })
        .def_prop_rw("AF",
            [](const Registers& r) { return r.AF(); },
            [](Registers& r, uint16_t v) { r.set_AF(v); })
        .def_prop_rw("IXH",
            [](const Registers& r) { return r.IXh(); },
            [](Registers& r, uint8_t v) { r.set_IXh(v); })
        .def_prop_rw("IXL",
            [](const Registers& r) { return r.IXl(); },
            [](Registers& r, uint8_t v) { r.set_IXl(v); })
        .def_prop_rw("IYH",
            [](const Registers& r) { return r.IYh(); },
            [](Registers& r, uint8_t v) { r.set_IYh(v); })
        .def_prop_rw("IYL",
            [](const Registers& r) { return r.IYl(); },
            [](Registers& r, uint8_t v) { r.set_IYl(v); })
        .def("reset", &Registers::reset);

    // ===============================================================
    // Z80
    // ===============================================================
    nb::class_<Z80>(m, "Z80")
        .def(nb::init<>())
        .def(nb::init<Bus*>(), nb::keep_alive<1, 2>())
        .def("reset", &Z80::reset)
        .def("step", &Z80::step)
        .def("run", &Z80::run)
        .def("run_instructions", &Z80::run_instructions)
        .def("trigger_interrupt", &Z80::trigger_interrupt, nb::arg("data") = 0xFF)
        .def("trigger_nmi", &Z80::trigger_nmi)
        .def("is_halted", &Z80::is_halted)
        .def("has_pending_interrupt", &Z80::has_pending_interrupt)
        .def("has_pending_nmi", &Z80::has_pending_nmi)
        .def("get_cycles", &Z80::get_cycles)
        .def("get_instruction_count", &Z80::get_instruction_count)
        .def_prop_rw("registers",
            [](Z80& c) -> Registers& { return c.regs; },
            [](Z80& c, const Registers& r) { c.regs = r; })
        .def_prop_rw("halted",
            [](Z80& c) { return c.halted; },
            [](Z80& c, bool v) { c.halted = v; })
        .def_prop_rw("bus",
            [](Z80& c) -> Bus* { return c.bus_ptr; },
            [](Z80& c, Bus* b) { c.set_bus(b); })
        .def("set_state", &Z80::set_state)
        .def("read_byte", &Z80::read)
        .def("write_byte", &Z80::write)
        .def("in_port", &Z80::in)
        .def("out_port", &Z80::out);

    // ===============================================================
    // Flag constants
    // ===============================================================
    m.attr("FLAG_S")  = nb::int_(0x80);
    m.attr("FLAG_Z")  = nb::int_(0x40);
    m.attr("FLAG_F5") = nb::int_(0x20);
    m.attr("FLAG_H")  = nb::int_(0x10);
    m.attr("FLAG_F3") = nb::int_(0x08);
    m.attr("FLAG_PV") = nb::int_(0x04);
    m.attr("FLAG_N")  = nb::int_(0x02);
    m.attr("FLAG_C")  = nb::int_(0x01);
}