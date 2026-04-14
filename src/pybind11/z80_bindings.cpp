#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include "../include/z80/z80.h"
#include "../include/z80/bus.h"

namespace py = pybind11;

namespace z80 {

// ============================================================
// Python bindings for Z80 CPU
// ============================================================

class PyBus : public Bus {
public:
    using Bus::Bus;

    uint8_t read(uint16_t addr) override {
        PYBIND11_OVERRIDE_PURE(uint8_t, Bus, read, addr);
    }

    void write(uint16_t addr, uint8_t val) override {
        PYBIND11_OVERRIDE_PURE(void, Bus, write, addr, val);
    }

    uint8_t in(uint16_t port) override {
        PYBIND11_OVERRIDE_PURE(uint8_t, Bus, in, port);
    }

    void out(uint16_t port, uint8_t val) override {
        PYBIND11_OVERRIDE_PURE(void, Bus, out, port, val);
    }

    void contend(uint16_t addr, int cycles) override {
        PYBIND11_OVERRIDE(void, Bus, contend, addr, cycles);
    }

    int get_memory_wait_states(uint16_t addr) override {
        PYBIND11_OVERRIDE(int, Bus, get_memory_wait_states, addr);
    }

    int get_io_wait_states(uint16_t port) override {
        PYBIND11_OVERRIDE(int, Bus, get_io_wait_states, port);
    }
};

PYBIND11_MODULE(z80_py, m) {
    m.doc() = "Z80 CPU Core Python bindings";

    // ============================================================
    // Bus class
    // ============================================================
    py::class_<Bus, PyBus>(m, "Bus")
        .def(py::init<>())
        .def("read", &Bus::read, "Read from memory address")
        .def("write", &Bus::write, "Write to memory address")
        .def("in", &Bus::in, "Read from I/O port")
        .def("out", &Bus::out, "Write to I/O port")
        .def("in_port", &Bus::in, "Read from I/O port (alias)")
        .def("out_port", &Bus::out, "Write to I/O port (alias)")
        .def("in_", &Bus::in, "Read from I/O port (alias)")
        .def("out_", &Bus::out, "Write to I/O port (alias)")
        .def("contend", &Bus::contend, "Simulate memory contention")
        .def("m1_cycle", &Bus::m1_cycle, "M1 cycle hook for R register")
        .def("interrupt_acknowledge", &Bus::interrupt_acknowledge, "Interrupt acknowledge");

    // ============================================================
    // SimpleBus class
    // ============================================================
    py::class_<SimpleBus, Bus>(m, "SimpleBus")
        .def(py::init<>())
        .def("read", &SimpleBus::read, "Read from memory")
        .def("write", &SimpleBus::write, "Write to memory")
        .def("in", &SimpleBus::in, "Read from port")
        .def("out", &SimpleBus::out, "Write to port")
        .def("in_port", &SimpleBus::in, "Read from port (alias)")
        .def("out_port", &SimpleBus::out, "Write to port (alias)")
        .def("in_", &SimpleBus::in, "Read from port (alias)")
        .def("out_", &SimpleBus::out, "Write to port (alias)")
        .def_property("memory", 
            [](SimpleBus& self) { return py::bytes(reinterpret_cast<char*>(self.memory), 65536); },
            [](SimpleBus& self, py::bytes data) { 
                std::string s = data;
                if (s.size() >= 65536) {
                    memcpy(self.memory, s.data(), 65536);
                }
            })
        .def_property("io_ports",
            [](SimpleBus& self) { return py::bytes(reinterpret_cast<char*>(self.io_ports), 256); },
            [](SimpleBus& self, py::bytes data) {
                std::string s = data;
                if (s.size() >= 256) {
                    memcpy(self.io_ports, s.data(), 256);
                }
            })
        // Aliases for I/O port access
        .def("in_", &SimpleBus::in, "Read from I/O port")
        .def("out", &SimpleBus::out, "Write to I/O port");

    // ============================================================
    // Registers struct
    // ============================================================
    py::class_<Registers>(m, "Registers")
        .def(py::init<>())
        .def_readwrite("A", &Registers::A)
        .def_readwrite("F", &Registers::F)
        .def_readwrite("B", &Registers::B)
        .def_readwrite("C", &Registers::C)
        .def_readwrite("D", &Registers::D)
        .def_readwrite("E", &Registers::E)
        .def_readwrite("H", &Registers::H)
        .def_readwrite("L", &Registers::L)
        .def_readwrite("IX", &Registers::IX)
        .def_readwrite("IY", &Registers::IY)
        .def_readwrite("SP", &Registers::SP)
        .def_readwrite("PC", &Registers::PC)
        .def_readwrite("I", &Registers::I)
        .def_readwrite("R", &Registers::R)
        .def_readwrite("IFF1", &Registers::IFF1)
        .def_readwrite("IFF2", &Registers::IFF2)
        .def_readwrite("IM", &Registers::IM)
        .def_property("BC", 
            [](const Registers& r) { return r.BC(); },
            [](Registers& r, uint16_t v) { r.set_BC(v); })
        .def_property("DE",
            [](const Registers& r) { return r.DE(); },
            [](Registers& r, uint16_t v) { r.set_DE(v); })
        .def_property("HL",
            [](const Registers& r) { return r.HL(); },
            [](Registers& r, uint16_t v) { r.set_HL(v); })
        .def_property("AF",
            [](const Registers& r) { return r.AF(); },
            [](Registers& r, uint16_t v) { r.set_AF(v); })
        .def_property("IXH",
            [](const Registers& r) { return r.IXh(); },
            [](Registers& r, uint8_t v) { r.set_IXh(v); })
        .def_property("IXL",
            [](const Registers& r) { return r.IXl(); },
            [](Registers& r, uint8_t v) { r.set_IXl(v); })
        .def_property("IYH",
            [](const Registers& r) { return r.IYh(); },
            [](Registers& r, uint8_t v) { r.set_IYh(v); })
        .def_property("IYL",
            [](const Registers& r) { return r.IYl(); },
            [](Registers& r, uint8_t v) { r.set_IYl(v); });

    // ============================================================
    // Z80 CPU class
    // ============================================================
    py::class_<Z80>(m, "Z80")
        .def(py::init<>(), "Create Z80 CPU with default SimpleBus")
        .def(py::init<Bus*>(), "Create Z80 CPU with optional bus")
        .def("reset", &Z80::reset, "Reset the CPU")
        .def("step", &Z80::step, "Execute one instruction, returns T-states")
        .def("run", &Z80::run, "Run for specified number of T-states")
        .def("run_instructions", &Z80::run_instructions, "Run N instructions")
        
        // Interrupt handling
        .def("trigger_interrupt", &Z80::trigger_interrupt, "Request interrupt with data")
        .def("trigger_nmi", &Z80::trigger_nmi, "Request NMI")
        .def("is_halted", &Z80::is_halted, "Check if CPU is halted")
        .def("has_pending_interrupt", &Z80::has_pending_interrupt, "Check for pending interrupt")
        .def("has_pending_nmi", &Z80::has_pending_nmi, "Check for pending NMI")
        
        // CPU state properties
        .def_property("halted", 
            [](Z80& self) { return self.halted; },
            [](Z80& self, bool v) { self.halted = v; })
        .def_property("IFF1", 
            [](Z80& self) { return self.regs.IFF1; },
            [](Z80& self, bool v) { self.regs.IFF1 = v; })
        .def_property("IFF2", 
            [](Z80& self) { return self.regs.IFF2; },
            [](Z80& self, bool v) { self.regs.IFF2 = v; })
        .def_property("IM", 
            [](Z80& self) { return self.regs.IM; },
            [](Z80& self, uint8_t v) { self.regs.IM = v; })
        .def_property("bus",
            [](Z80& self) -> Bus* { return self.bus_ptr; },
            [](Z80& self, Bus* b) { self.set_bus(b); })
        
        // Register access
        .def_property("registers", 
            [](Z80& self) -> Registers& { return self.regs; },
            [](Z80& self, Registers& r) { self.regs = r; })
        .def_property("regs", 
            [](Z80& self) -> Registers& { return self.regs; },
            [](Z80& self, Registers& r) { self.regs = r; })
        .def("get_registers", (Registers& (Z80::*)()) &Z80::get_registers, "Get register state")
        
        // Memory/IO access
        .def("set_bus", &Z80::set_bus, "Set bus interface")
        .def("read_memory", [](Z80& self, uint16_t addr) { return self.read(addr); }, "Read memory")
        .def("write_memory", [](Z80& self, uint16_t addr, uint8_t val) { self.write(addr, val); }, "Write memory")
        .def("write_byte", [](Z80& self, uint16_t addr, uint8_t val) { self.write(addr, val); }, "Write byte to memory")
        .def("read_byte", [](Z80& self, uint16_t addr) { return self.read(addr); }, "Read byte from memory")
        .def("in_port", [](Z80& self, uint16_t port) { return self.in(port); }, "Read from I/O port")
        .def("out_port", [](Z80& self, uint16_t port, uint8_t val) { self.out(port, val); }, "Write to I/O port")
        
        // Cycle info
        .def("get_cycles", &Z80::get_cycles, "Get total T-states executed")
        .def("get_instruction_count", &Z80::get_instruction_count, "Get instructions executed");
}

} // namespace z80
