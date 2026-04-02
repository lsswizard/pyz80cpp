#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/shared_ptr.h>
#include "cpu.h"
#include "bus.h"
#include "registers.h"
#include "flags.h"
#include <memory>

namespace nb = nanobind;

static nb::object make_bytes_view(const uint8_t* data, size_t size) {
    return nb::bytes(reinterpret_cast<const char*>(data), size);
}

// ============================================================
// PythonCallbackBus: wraps Python callables as a C++ Bus
// Uses nb::object to hold references to Python callables
// Acquires GIL before calling Python functions
// ============================================================
class PythonCallbackBus : public Bus {
public:
    nb::object _read_fn;
    nb::object _write_fn;
    nb::object _io_read_fn;
    nb::object _io_write_fn;

    PythonCallbackBus(nb::object read_fn, nb::object write_fn,
                      nb::object io_read_fn, nb::object io_write_fn)
        : _read_fn(std::move(read_fn)),
          _write_fn(std::move(write_fn)),
          _io_read_fn(std::move(io_read_fn)),
          _io_write_fn(std::move(io_write_fn)) {}

    uint8_t bus_read(uint16_t addr, int t_state) override {
        nb::gil_scoped_acquire gil;
        return nb::cast<uint8_t>(_read_fn(addr, t_state));
    }

    void bus_write(uint16_t addr, uint8_t value, int t_state) override {
        nb::gil_scoped_acquire gil;
        _write_fn(addr, value, t_state);
    }

    uint8_t bus_io_read(uint16_t port, int t_state) override {
        nb::gil_scoped_acquire gil;
        return nb::cast<uint8_t>(_io_read_fn(port, t_state));
    }

    void bus_io_write(uint16_t port, uint8_t value, int t_state) override {
        nb::gil_scoped_acquire gil;
        _io_write_fn(port, value, t_state);
    }
};

// ============================================================
// CPUWrapper: wraps CPU and keeps reference to Python bus
// ============================================================
class CPUWrapper {
public:
    nb::object _py_bus;
    CPU _cpu;

    CPUWrapper() : _cpu(nullptr) {}

    CPUWrapper(nb::object bus) : _py_bus(std::move(bus)), _cpu(nullptr) {
        Bus* cpp_bus = nullptr;
        
        // Check for SimpleBus first
        if (nb::isinstance<SimpleBus>(_py_bus)) {
            cpp_bus = &nb::cast<SimpleBus&>(_py_bus);
        } 
        // Check for our PythonCallbackBus
        else if (nb::isinstance<PythonCallbackBus>(_py_bus)) {
            cpp_bus = &nb::cast<PythonCallbackBus&>(_py_bus);
        }
        // Generic Bus - try to get the pointer
        else if (nb::isinstance<Bus>(_py_bus)) {
            cpp_bus = &nb::cast<Bus&>(_py_bus);
        }
        
        _cpu.bus = cpp_bus;
        _cpu._owns_bus = false;  // External bus - don't delete it
        // Re-check if this is a SimpleBus for fast path
        _cpu._is_simple_bus = false;
        _cpu._mem = nullptr;
        if (cpp_bus) {
            auto* sb = dynamic_cast<SimpleBus*>(cpp_bus);
            if (sb) {
                _cpu._mem = sb->memory;
                _cpu._is_simple_bus = true;
            }
        }
    }

    int step() { return _cpu.step(); }
    int run(int max_cycles) { return _cpu.run(max_cycles); }
    int run_instructions(int count) { return _cpu.run_instructions(count); }
    void reset() { _cpu.reset(); }
    void trigger_interrupt(uint8_t data) { _cpu.trigger_interrupt(data); }
    void trigger_nmi() { _cpu.trigger_nmi(); }
    uint8_t read_byte(uint16_t addr) { return _cpu._bus_read(addr, _cpu.cycles); }
    void write_byte(uint16_t addr, uint8_t val) { _cpu._bus_write(addr, val, _cpu.cycles); }
    uint8_t io_read(uint16_t port) { return _cpu._bus_io_read(port, _cpu.cycles); }
    void io_write(uint16_t port, uint8_t val) { _cpu._bus_io_write(port, val, _cpu.cycles); }
    Registers& get_regs() { return _cpu.regs; }
    Bus* get_bus() { return _cpu.bus; }

    void invalidate_range(uint16_t start, uint16_t end) {
        for (uint16_t addr = start; addr != static_cast<uint16_t>(end + 1); addr++) {
            _cpu.decoder.invalidate(addr);
        }
    }
    void invalidate_cache() { _cpu.decoder.invalidate_all(); }
    void invalidate_all() { _cpu.decoder.invalidate_all(); }
};

NB_MODULE(_pyz80, m) {
    m.attr("FLAG_S")  = (int)z80flags::FLAG_S;
    m.attr("FLAG_Z")  = (int)z80flags::FLAG_Z;
    m.attr("FLAG_F5") = (int)z80flags::FLAG_F5;
    m.attr("FLAG_H")  = (int)z80flags::FLAG_H;
    m.attr("FLAG_F3") = (int)z80flags::FLAG_F3;
    m.attr("FLAG_PV") = (int)z80flags::FLAG_PV;
    m.attr("FLAG_N")  = (int)z80flags::FLAG_N;
    m.attr("FLAG_C")  = (int)z80flags::FLAG_C;

    z80flags::init_tables();
    build_handler_tables();

    m.attr("PARITY_TABLE") = make_bytes_view(z80flags::PARITY_TABLE, 256);
    m.attr("SZ_TABLE") = make_bytes_view(z80flags::SZ_TABLE, 256);
    m.attr("SZ53_TABLE") = make_bytes_view(z80flags::SZ53_TABLE, 256);
    m.attr("SZP_TABLE") = make_bytes_view(z80flags::SZP_TABLE, 256);
    m.attr("SZ53P_TABLE") = make_bytes_view(z80flags::SZ53P_TABLE, 256);
    m.attr("SZHZP_TABLE") = make_bytes_view(z80flags::SZHZP_TABLE, 256);
    m.attr("ADD_FLAGS") = make_bytes_view(z80flags::ADD_FLAGS, 65536);
    m.attr("ADC_FLAGS") = make_bytes_view(z80flags::ADC_FLAGS, 65536);
    m.attr("SUB_FLAGS") = make_bytes_view(z80flags::SUB_FLAGS, 65536);
    m.attr("SBC_FLAGS") = make_bytes_view(z80flags::SBC_FLAGS, 65536);
    m.attr("INC_FLAGS") = make_bytes_view(z80flags::INC_FLAGS, 256);
    m.attr("DEC_FLAGS") = make_bytes_view(z80flags::DEC_FLAGS, 256);

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
        .def_rw("Memptr", &Registers::MEMPTR)
        .def_rw("Q", &Registers::Q)
        .def_rw("last_Q", &Registers::LAST_Q)
        .def_prop_rw("BC", &Registers::BC, &Registers::set_BC)
        .def_prop_rw("DE", &Registers::DE, &Registers::set_DE)
        .def_prop_rw("HL", &Registers::HL, &Registers::set_HL)
        .def_prop_rw("AF", &Registers::AF, &Registers::set_AF)
        .def_prop_rw("IXh", &Registers::IXh, &Registers::set_IXh)
        .def_prop_rw("IXl", &Registers::IXl, &Registers::set_IXl)
        .def_prop_rw("IYh", &Registers::IYh, &Registers::set_IYh)
        .def_prop_rw("IYl", &Registers::IYl, &Registers::set_IYl)
        .def("swap_shadow", &Registers::swap_shadow)
        .def("swap_shadow_all", &Registers::swap_shadow_all)
        .def("reset", &Registers::reset);

    nb::class_<Bus>(m, "Bus");

    // IOPorts wrapper for SimpleBus
    nb::class_<SimpleBus::IOPorts>(m, "IOPorts")
        .def("__getitem__", &SimpleBus::IOPorts::get)
        .def("__setitem__", &SimpleBus::IOPorts::set);

    nb::class_<SimpleBus, Bus>(m, "SimpleBus")
        .def(nb::init<>())
        .def("bus_read", &SimpleBus::bus_read)
        .def("bus_write", &SimpleBus::bus_write)
        .def("bus_io_read", &SimpleBus::bus_io_read)
        .def("bus_io_write", &SimpleBus::bus_io_write)
        .def("__getitem__", [](SimpleBus& self, uint16_t addr) {
            return self.memory[addr & 0xFFFF];
        })
        .def("__setitem__", [](SimpleBus& self, uint16_t addr, uint8_t val) {
            self.memory[addr & 0xFFFF] = val;
        })
        .def_prop_ro("io_ports", &SimpleBus::get_io_ports, nb::rv_policy::reference_internal);

    // PythonCallbackBus: wraps Python callables as a C++ Bus
    nb::class_<PythonCallbackBus, Bus>(m, "CallbackBus")
        .def(nb::init<nb::object, nb::object, nb::object, nb::object>(),
             nb::arg("bus_read"), nb::arg("bus_write"),
             nb::arg("bus_io_read"), nb::arg("bus_io_write"))
        .def("bus_read", &PythonCallbackBus::bus_read)
        .def("bus_write", &PythonCallbackBus::bus_write)
        .def("bus_io_read", &PythonCallbackBus::bus_io_read)
        .def("bus_io_write", &PythonCallbackBus::bus_io_write);

    nb::class_<CPUWrapper>(m, "Z80CPU")
        .def(nb::init<>())
        .def(nb::init<nb::object>(), nb::keep_alive<1, 2>())
        .def("step", &CPUWrapper::step)
        .def("run", &CPUWrapper::run)
        .def("run_instructions", &CPUWrapper::run_instructions)
        .def("reset", &CPUWrapper::reset)
        .def("trigger_interrupt", &CPUWrapper::trigger_interrupt, nb::arg("data") = 0xFF)
        .def("trigger_nmi", &CPUWrapper::trigger_nmi)
        .def("read_byte", &CPUWrapper::read_byte)
        .def("write_byte", &CPUWrapper::write_byte)
        .def("io_read", &CPUWrapper::io_read)
        .def("io_write", &CPUWrapper::io_write)
        .def_prop_ro("regs", &CPUWrapper::get_regs)
        .def_prop_ro("bus", &CPUWrapper::get_bus)
        .def_prop_rw("cycles",
            [](CPUWrapper& self) -> int { return self._cpu.cycles; },
            [](CPUWrapper& self, int v) { self._cpu.cycles = v; })
        .def_prop_ro("halted", [](CPUWrapper& self) -> bool { return self._cpu.halted; })
        .def_prop_ro("instruction_count", [](CPUWrapper& self) -> int { return self._cpu.instruction_count; })
        .def_prop_ro("interrupt_pending", [](CPUWrapper& self) -> bool { return self._cpu.interrupt_pending; })
        .def_prop_ro("interrupt_data", [](CPUWrapper& self) -> uint8_t { return self._cpu.interrupt_data; })
        .def("invalidate_range", &CPUWrapper::invalidate_range)
        .def("invalidate_cache", &CPUWrapper::invalidate_cache)
        .def("invalidate_all", &CPUWrapper::invalidate_all)
        .def_prop_ro("nmi_pending", [](CPUWrapper& self) -> bool { return self._cpu.nmi_pending; });
}