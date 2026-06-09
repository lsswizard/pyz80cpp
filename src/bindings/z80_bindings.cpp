#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include "../../include/z80/z80.h"
#include "../../include/z80/bus.h"

namespace py = pybind11;

namespace z80 {

// ============================================================
// Python-subclassable Bus trampoline
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

    uint8_t in_(uint16_t port) override {
        PYBIND11_OVERRIDE(uint8_t, Bus, in_, port);
    }

    void out_(uint16_t port, uint8_t val) override {
        PYBIND11_OVERRIDE(void, Bus, out_, port, val);
    }

    uint8_t interrupt_acknowledge() override {
        PYBIND11_OVERRIDE(uint8_t, Bus, interrupt_acknowledge);
    }

    // FIX Issue 3: extra_cycles fallback for machines without timing tables
    int extra_cycles(AccessKind kind, uint16_t addr, int t_state) override {
        PYBIND11_OVERRIDE(int, Bus, extra_cycles, kind, addr, t_state);
    }
};

// ============================================================
// AccessKind enum binding helper
// ============================================================
AccessKind access_kind_from_int(int val) {
    switch (val) {
        case 0: return AccessKind::OpcodeFetch;
        case 1: return AccessKind::MemoryRead;
        case 2: return AccessKind::MemoryWrite;
        case 3: return AccessKind::IORead;
        case 4: return AccessKind::IOWrite;
        case 5: return AccessKind::InterruptAck;
        default: return AccessKind::OpcodeFetch;
    }
}

PYBIND11_MODULE(z80_core, m) {
    m.doc() = "Z80 CPU core — machine-independent, cycle-accurate emulator";

    // ================================================================
    // AccessKind enum
    // ================================================================
    py::enum_<AccessKind>(m, "AccessKind")
        .value("OpcodeFetch", AccessKind::OpcodeFetch)
        .value("MemoryRead", AccessKind::MemoryRead)
        .value("MemoryWrite", AccessKind::MemoryWrite)
        .value("IORead", AccessKind::IORead)
        .value("IOWrite", AccessKind::IOWrite)
        .value("InterruptAck", AccessKind::InterruptAck);

    // ================================================================
    // Bus
    // ================================================================
    py::class_<Bus, PyBus>(m, "Bus")
        .def(py::init<>())
        .def("read",  &Bus::read,  "Read byte from memory address")
        .def("write", &Bus::write, "Write byte to memory address")
        // I/O uses in_() / out_() — override these in Python subclasses
        .def("in_",   &Bus::in_,   "Read from I/O port")
        .def("out_",  &Bus::out_,  "Write to I/O port")
        .def("interrupt_acknowledge",&Bus::interrupt_acknowledge,"Return interrupt vector byte")

        // FIX Issue 3: Timing tables with PROPER OWNERSHIP (copied to owned storage)
        // FIX Issue 1: Access-type-specific timing tables
        .def("set_timing_tables",
            [](Bus& bus,
               py::buffer fetch_tbl,
               py::buffer mem_read_tbl,
               py::buffer mem_write_tbl,
               py::buffer io_read_tbl,
               py::buffer io_write_tbl) {

                // Helper to copy buffer to owned vector
                auto copy_table = [](py::buffer& buf) -> std::vector<uint8_t> {
                    py::buffer_info info = buf.request();
                    if (info.ndim != 1 || info.shape[0] == 0) {
                        return {};
                    }
                    auto* ptr = static_cast<uint8_t*>(info.ptr);
                    return std::vector<uint8_t>(ptr, ptr + info.shape[0]);
                };

                bus.fetch_delay_table_vec = copy_table(fetch_tbl);
                bus.mem_read_delay_table_vec = copy_table(mem_read_tbl);
                bus.mem_write_delay_table_vec = copy_table(mem_write_tbl);
                bus.io_read_delay_table_vec = copy_table(io_read_tbl);
                bus.io_write_delay_table_vec = copy_table(io_write_tbl);

                // Set pointers to owned data
                if (!bus.fetch_delay_table_vec.empty()) {
                    bus.fetch_delay_table = bus.fetch_delay_table_vec.data();
                    bus.timing_table_size = bus.fetch_delay_table_vec.size();
                }
                if (!bus.mem_read_delay_table_vec.empty())
                    bus.mem_read_delay_table = bus.mem_read_delay_table_vec.data();
                if (!bus.mem_write_delay_table_vec.empty())
                    bus.mem_write_delay_table = bus.mem_write_delay_table_vec.data();
                if (!bus.io_read_delay_table_vec.empty())
                    bus.io_read_delay_table = bus.io_read_delay_table_vec.data();
                if (!bus.io_write_delay_table_vec.empty())
                    bus.io_write_delay_table = bus.io_write_delay_table_vec.data();
            },
            "Set timing tables (copied into owned storage)")

        .def("clear_timing_tables",
            [](Bus& bus) {
                bus.fetch_delay_table = nullptr;
                bus.mem_read_delay_table = nullptr;
                bus.mem_write_delay_table = nullptr;
                bus.io_read_delay_table = nullptr;
                bus.io_write_delay_table = nullptr;
                bus.timing_table_size = 0;
                // Vectors are automatically cleared
            },
            "Clear all timing tables")

        // Fallback for machines without fast tables
        .def("extra_cycles",
            [](Bus& bus, int kind, uint16_t addr, int t_state) {
                return bus.extra_cycles(access_kind_from_int(kind), addr, t_state);
            },
            "Get extra wait states for access type at t_state")

        // Contention masks (machine-specific - set by Python)
        .def_readwrite("fetch_contention_mask", &Bus::fetch_contention_mask,
            "Bitmask of contended 4KB regions for opcode fetch")
        .def_readwrite("mem_read_contention_mask", &Bus::mem_read_contention_mask,
            "Bitmask of contended 4KB regions for memory read")
        .def_readwrite("mem_write_contention_mask", &Bus::mem_write_contention_mask,
            "Bitmask of contended 4KB regions for memory write")
        .def_readwrite("io_read_contention_mask", &Bus::io_read_contention_mask,
            "Bitmask of contended ports for I/O read (low 8 bits)")
        .def_readwrite("io_write_contention_mask", &Bus::io_write_contention_mask,
            "Bitmask of contended ports for I/O write (low 8 bits)")

        // Read-only timing table pointers (for debugging)
        .def_property_readonly("fetch_delay_table",
            [](Bus& bus) -> py::object {
                if (!bus.fetch_delay_table || bus.timing_table_size == 0)
                    return py::none();
                return py::bytes(reinterpret_cast<char*>(bus.fetch_delay_table),
                                 bus.timing_table_size);
            })
        .def_property_readonly("timing_table_size",
            [](Bus& bus) { return bus.timing_table_size; })

        // FAST PATH: Set C++ memory pointer for direct access (avoids Python callbacks)
        .def("set_fast_memory_ptr",
            [](Bus& bus, py::buffer buf) {
                py::buffer_info info = buf.request();
                if (info.ndim != 1 || info.shape[0] < 65536) {
                    throw std::runtime_error("Memory buffer must be at least 64KB");
                }
                bus.fast_memory_ptr = static_cast<uint8_t*>(info.ptr);
            },
            "Set C++ memory pointer for direct access (avoids Python callbacks)")
        .def("set_fast_io_read_ptr",
            [](Bus& bus, py::buffer buf) {
                py::buffer_info info = buf.request();
                if (info.ndim != 1 || info.shape[0] < 256) {
                    throw std::runtime_error("I/O buffer must be at least 256 bytes");
                }
                bus.fast_io_read_ptr = static_cast<uint8_t*>(info.ptr);
            },
            "Set C++ I/O pointer for direct access (avoids Python callbacks)")
        .def("set_fast_io_read_active",
            [](Bus& bus, int port, bool active) {
                if (port < 0 || port > 255) {
                    throw std::runtime_error("Port must be 0-255");
                }
                bus.fast_io_read_active[port] = active;
            },
            "Enable/disable fast I/O for a specific port (low 8 bits)")
        ;

    // ================================================================
    // SimpleBus
    // ================================================================
    py::class_<SimpleBus, Bus>(m, "SimpleBus")
        .def(py::init<>())
        .def("read",  &SimpleBus::read)
        .def("write", &SimpleBus::write)
        .def("in_",   &SimpleBus::in_)
        .def("out_",  &SimpleBus::out_)
        .def_property("memory",
            [](SimpleBus& s) {
                return py::bytes(reinterpret_cast<char*>(s.memory), 65536);
            },
            [](SimpleBus& s, py::bytes data) {
                std::string b = data;
                if (b.size() >= 65536)
                    memcpy(s.memory, b.data(), 65536);
            })
        .def_property("io_ports",
            [](SimpleBus& s) {
                return py::bytes(reinterpret_cast<char*>(s.io_ports), 256);
            },
            [](SimpleBus& s, py::bytes data) {
                std::string b = data;
                if (b.size() >= 256)
                    memcpy(s.io_ports, b.data(), 256);
            })
        // Convenience aliases matching common Python Z80 conventions
        .def("read_byte",  &SimpleBus::read)
        .def("write_byte", &SimpleBus::write)
        .def("in_port",    &SimpleBus::in_)
        .def("out_port",   &SimpleBus::out_);

    // ================================================================
    // Registers
    // ================================================================
    py::class_<Registers>(m, "Registers")
        .def(py::init<>())
        // Main 8-bit registers
        .def_readwrite("A",  &Registers::A)
        .def_readwrite("F",  &Registers::F)
        .def_readwrite("B",  &Registers::B)
        .def_readwrite("C",  &Registers::C)
        .def_readwrite("D",  &Registers::D)
        .def_readwrite("E",  &Registers::E)
        .def_readwrite("H",  &Registers::H)
        .def_readwrite("L",  &Registers::L)
        // Shadow registers
        .def_readwrite("Ap", &Registers::Ap)
        .def_readwrite("Fp", &Registers::Fp)
        .def_readwrite("Bp", &Registers::Bp)
        .def_readwrite("Cp", &Registers::Cp)
        .def_readwrite("Dp", &Registers::Dp)
        .def_readwrite("Ep", &Registers::Ep)
        .def_readwrite("Hp", &Registers::Hp)
        .def_readwrite("Lp", &Registers::Lp)
        // 16-bit registers
        .def_readwrite("IX", &Registers::IX)
        .def_readwrite("IY", &Registers::IY)
        .def_readwrite("SP", &Registers::SP)
        .def_readwrite("PC", &Registers::PC)
        // Special
        .def_readwrite("I",      &Registers::I)
        .def_readwrite("R",      &Registers::R)
        .def_readwrite("IFF1",   &Registers::IFF1)
        .def_readwrite("IFF2",   &Registers::IFF2)
        .def_readwrite("IM",     &Registers::IM)
        .def_readwrite("MEMPTR", &Registers::MEMPTR)
        .def_readwrite("Q",      &Registers::Q)
        // 16-bit properties (computed from pairs)
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
        // Index half-registers
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
            [](Registers& r, uint8_t v) { r.set_IYl(v); })
        .def("reset", &Registers::reset, "Reset all registers to power-on state");

    // ================================================================
    // Z80
    // ================================================================
    py::class_<Z80>(m, "Z80")
        .def(py::init<>(), "Create Z80 with internal SimpleBus")
        .def(py::init<Bus*>(), py::keep_alive<1, 2>(),
             "Create Z80 with a custom Bus (must outlive the Z80)")

        .def("reset", &Z80::reset, "Reset CPU to power-on state")

        // Execution
        // FIX Issue 2: step() returns ACTUAL elapsed cycles (base + wait)
        .def("step",             &Z80::step,             "Execute one instruction; returns T-states consumed INCLUDING wait states")
        .def("run",              &Z80::run,              "Run for at most max_cycles T-states")
        .def("run_instructions", &Z80::run_instructions, "Run exactly N instructions (stops on HALT)")

        // Interrupt interface
        .def("trigger_interrupt", &Z80::trigger_interrupt,
             "Signal a maskable interrupt with optional vector byte",
             py::arg("data") = 0xFF)
        .def("trigger_nmi",      &Z80::trigger_nmi, "Signal a non-maskable interrupt")
        .def("is_halted",        &Z80::is_halted)
        .def("has_pending_interrupt", &Z80::has_pending_interrupt)
        .def("has_pending_nmi",  &Z80::has_pending_nmi)

        // Cycle counters
        .def("get_cycles",           &Z80::get_cycles)
        .def("get_instruction_count",&Z80::get_instruction_count)

        // Register access
        .def_property("registers",
            [](Z80& c) -> Registers& { return c.regs; },
            [](Z80& c, const Registers& r) { c.regs = r; })
        .def("get_registers",
            (Registers& (Z80::*)()) &Z80::get_registers,
            py::return_value_policy::reference_internal)
        .def("set_state", &Z80::set_state,
             "Bulk-set registers from a dict, e.g. {'A': 0, 'PC': 0x100}")

        // Cycle tracking
        // FIX Issue 2: NO tstates_per_frame - Python controls frame timing
        .def_property("cycles",
            [](Z80& c) { return c.total_cycles; },
            [](Z80& c, int v) { c.total_cycles = v; })
        .def_property("t_state",
            [](Z80& c) { return c.t_state; },
            [](Z80& c, int v) { c.t_state = v; })
        .def_readwrite("trap_address", &Z80::trap_address, "Early exit address for run()")
        .def_readwrite("interrupt_data", &Z80::interrupt_data, "Data bus value during interrupt acknowledge")

        // Convenience CPU state shortcuts
        .def_property("is_halted",
            [](Z80& c) { return c.is_halted(); },
            [](Z80& c, bool v) { c.halted = v; })
        .def_property("IFF1",
            [](Z80& c) { return c.regs.IFF1; },
            [](Z80& c, bool v) { c.regs.IFF1 = v; })
        .def_property("IFF2",
            [](Z80& c) { return c.regs.IFF2; },
            [](Z80& c, bool v) { c.regs.IFF2 = v; })
        .def_property("IM",
            [](Z80& c) { return c.regs.IM; },
            [](Z80& c, uint8_t v) { c.regs.IM = v; })

        // Bus access
        .def_property("bus",
            [](Z80& c) -> Bus* { return c.bus_ptr; },
            [](Z80& c, Bus* b) { c.set_bus(b); })
        .def("set_bus", &Z80::set_bus,
             py::keep_alive<1, 2>(), "Replace the bus interface")

        // Direct memory/IO helpers (bypass T-state accounting — for testing)
        .def("read_byte",  [](Z80& c, uint16_t a) { return c.bus_ptr->read(a); })
        .def("write_byte", [](Z80& c, uint16_t a, uint8_t v) { c.bus_ptr->write(a, v); })
        .def("in_port",    [](Z80& c, uint16_t p) { return c.bus_ptr->in_(p); })
        .def("out_port",   [](Z80& c, uint16_t p, uint8_t v) { c.bus_ptr->out_(p, v); });
}

} // namespace z80
