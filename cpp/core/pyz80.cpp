#include <Python.h>
#include <structmember.h>
#include "cpu.h"
#include "registers.h"
#include <memory>

// ── Python callback bus ─────────────────────────────────────────────
// Bridges a Python object implementing the Z80Bus protocol to the C++ Bus interface.
class PythonBus : public Bus {
public:
    PyObject* py_obj;
    PyObject* input_cb;
    PyObject* output_cb;
    PyObject* reti_cb;
    PyObject* get_int_vector_cb;
    uint8_t memory[65536];
    uint8_t addr_marks[65536];  // Memory marks for breakpoints/self-mod code
    uint8_t io_ports[256];      // I/O port storage for standalone testing
    uint16_t last_read_addr;

    PythonBus(PyObject* obj) : py_obj(obj), input_cb(nullptr), output_cb(nullptr),
                               reti_cb(nullptr), get_int_vector_cb(nullptr),
                               last_read_addr(0) {
        Py_INCREF(py_obj);
        std::memset(memory, 0xFF, sizeof(memory));
        std::memset(addr_marks, 0, sizeof(addr_marks));
        std::memset(io_ports, 0, sizeof(io_ports));
    }

    ~PythonBus() override {
        Py_DECREF(py_obj);
        Py_XDECREF(input_cb);
        Py_XDECREF(output_cb);
        Py_XDECREF(reti_cb);
        Py_XDECREF(get_int_vector_cb);
    }

    uint8_t bus_read(uint16_t addr, int t_state, CycleType type) override {
        last_read_addr = addr;
        if (py_obj == Py_None) {
            return memory[addr & 0xFFFF];
        }
        PyObject* result = PyObject_CallMethod(py_obj, "bus_read", "iii", addr, t_state, (int)type);
        if (!result) {
            PyErr_Print();
            return 0xFF;
        }
        uint8_t val = (uint8_t)PyLong_AsLong(result);
        Py_DECREF(result);
        return val;
    }

    void bus_write(uint16_t addr, uint8_t value, int t_state, CycleType type) override {
        if (py_obj == Py_None) {
            memory[addr & 0xFFFF] = value;
            return;
        }
        PyObject* result = PyObject_CallMethod(py_obj, "bus_write", "iIii", addr, value, t_state, (int)type);
        Py_XDECREF(result);
    }

    uint8_t bus_io_read(uint16_t port, int t_state) override {
        if (input_cb) {
            PyObject* result = PyObject_CallFunction(input_cb, "I", port);
            if (!result) {
                PyErr_Print();
                return 0xFF;
            }
            uint8_t val = (uint8_t)PyLong_AsLong(result);
            Py_DECREF(result);
            return val;
        }
        if (py_obj == Py_None) {
            return io_ports[port & 0xFF];
        }
        PyObject* result = PyObject_CallMethod(py_obj, "bus_io_read", "Ii", port, t_state);
        if (!result) {
            PyErr_Print();
            return 0xFF;
        }
        uint8_t val = (uint8_t)PyLong_AsLong(result);
        Py_DECREF(result);
        return val;
    }

    void bus_io_write(uint16_t port, uint8_t value, int t_state) override {
        if (output_cb) {
            PyObject_CallFunction(output_cb, "II", port, value);
            return;
        }
        if (py_obj == Py_None) {
            io_ports[port & 0xFF] = value;
            return;
        }
        PyObject* result = PyObject_CallMethod(py_obj, "bus_io_write", "IIi", port, value, t_state);
        Py_XDECREF(result);
    }
};

typedef struct {
    PyObject_HEAD
    CPU* cpu;
    PythonBus* py_bus;  // Non-null if we're using a Python bus
} Z80CPUObject;

// ── Registers wrapper type ──────────────────────────────────────────
typedef struct {
    PyObject_HEAD
    CPU* cpu;
} RegsObject;

static void Regs_dealloc(RegsObject* self) {
    Py_TYPE(self)->tp_free((PyObject*)self);
}

// 8-bit registers
#define REG8_GETTER(name) \
static PyObject* Regs_get_##name(RegsObject* self, void* closure) { \
    return PyLong_FromLong(self->cpu->regs.name); \
} \
static int Regs_set_##name(RegsObject* self, PyObject* value, void* closure) { \
    self->cpu->regs.name = (uint8_t)PyLong_AsLong(value); \
    return 0; \
}

REG8_GETTER(A)
REG8_GETTER(F)
REG8_GETTER(B)
REG8_GETTER(C)
REG8_GETTER(D)
REG8_GETTER(E)
REG8_GETTER(H)
REG8_GETTER(L)
REG8_GETTER(Ap)
REG8_GETTER(Fp)
REG8_GETTER(Bp)
REG8_GETTER(Cp)
REG8_GETTER(Dp)
REG8_GETTER(Ep)
REG8_GETTER(Hp)
REG8_GETTER(Lp)
REG8_GETTER(I)
REG8_GETTER(R)
REG8_GETTER(Q)
REG8_GETTER(LAST_Q)

// 16-bit registers
#define REG16_GETTER(name) \
static PyObject* Regs_get_##name(RegsObject* self, void* closure) { \
    return PyLong_FromLong(self->cpu->regs.name); \
} \
static int Regs_set_##name(RegsObject* self, PyObject* value, void* closure) { \
    self->cpu->regs.name = (uint16_t)PyLong_AsLong(value); \
    return 0; \
}

REG16_GETTER(PC)
REG16_GETTER(SP)
REG16_GETTER(IX)
REG16_GETTER(IY)
REG16_GETTER(MEMPTR)

// Compound 16-bit registers
static PyObject* Regs_get_BC(RegsObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->regs.BC());
}
static int Regs_set_BC(RegsObject* self, PyObject* value, void* closure) {
    self->cpu->regs.set_BC((uint16_t)PyLong_AsLong(value));
    return 0;
}

static PyObject* Regs_get_DE(RegsObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->regs.DE());
}
static int Regs_set_DE(RegsObject* self, PyObject* value, void* closure) {
    self->cpu->regs.set_DE((uint16_t)PyLong_AsLong(value));
    return 0;
}

static PyObject* Regs_get_HL(RegsObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->regs.HL());
}
static int Regs_set_HL(RegsObject* self, PyObject* value, void* closure) {
    self->cpu->regs.set_HL((uint16_t)PyLong_AsLong(value));
    return 0;
}

static PyObject* Regs_get_AF(RegsObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->regs.AF());
}
static int Regs_set_AF(RegsObject* self, PyObject* value, void* closure) {
    self->cpu->regs.set_AF((uint16_t)PyLong_AsLong(value));
    return 0;
}

// Boolean flags
#define FLAG_GETTER(name) \
static PyObject* Regs_get_##name(RegsObject* self, void* closure) { \
    return PyBool_FromLong(self->cpu->regs.name); \
} \
static int Regs_set_##name(RegsObject* self, PyObject* value, void* closure) { \
    self->cpu->regs.name = PyObject_IsTrue(value); \
    return 0; \
}

FLAG_GETTER(IFF1)
FLAG_GETTER(IFF2)
FLAG_GETTER(EI_PENDING)
FLAG_GETTER(EI_JUST_RESOLVED)
FLAG_GETTER(UNRESOLVED_PREFIX)

// IM (0-2)
static PyObject* Regs_get_IM(RegsObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->regs.IM);
}
static int Regs_set_IM(RegsObject* self, PyObject* value, void* closure) {
    self->cpu->regs.IM = (uint8_t)PyLong_AsLong(value);
    return 0;
}

static PyGetSetDef Regs_getsetters[] = {
    {"A", (getter)Regs_get_A, (setter)Regs_set_A, NULL, NULL},
    {"F", (getter)Regs_get_F, (setter)Regs_set_F, NULL, NULL},
    {"B", (getter)Regs_get_B, (setter)Regs_set_B, NULL, NULL},
    {"C", (getter)Regs_get_C, (setter)Regs_set_C, NULL, NULL},
    {"D", (getter)Regs_get_D, (setter)Regs_set_D, NULL, NULL},
    {"E", (getter)Regs_get_E, (setter)Regs_set_E, NULL, NULL},
    {"H", (getter)Regs_get_H, (setter)Regs_set_H, NULL, NULL},
    {"L", (getter)Regs_get_L, (setter)Regs_set_L, NULL, NULL},
    {"Ap", (getter)Regs_get_Ap, (setter)Regs_set_Ap, NULL, NULL},
    {"Fp", (getter)Regs_get_Fp, (setter)Regs_set_Fp, NULL, NULL},
    {"Bp", (getter)Regs_get_Bp, (setter)Regs_set_Bp, NULL, NULL},
    {"Cp", (getter)Regs_get_Cp, (setter)Regs_set_Cp, NULL, NULL},
    {"Dp", (getter)Regs_get_Dp, (setter)Regs_set_Dp, NULL, NULL},
    {"Ep", (getter)Regs_get_Ep, (setter)Regs_set_Ep, NULL, NULL},
    {"Hp", (getter)Regs_get_Hp, (setter)Regs_set_Hp, NULL, NULL},
    {"Lp", (getter)Regs_get_Lp, (setter)Regs_set_Lp, NULL, NULL},
    {"I", (getter)Regs_get_I, (setter)Regs_set_I, NULL, NULL},
    {"R", (getter)Regs_get_R, (setter)Regs_set_R, NULL, NULL},
    {"Q", (getter)Regs_get_Q, (setter)Regs_set_Q, NULL, NULL},
    {"LAST_Q", (getter)Regs_get_LAST_Q, (setter)Regs_set_LAST_Q, NULL, NULL},
    {"PC", (getter)Regs_get_PC, (setter)Regs_set_PC, NULL, NULL},
    {"SP", (getter)Regs_get_SP, (setter)Regs_set_SP, NULL, NULL},
    {"IX", (getter)Regs_get_IX, (setter)Regs_set_IX, NULL, NULL},
    {"IY", (getter)Regs_get_IY, (setter)Regs_set_IY, NULL, NULL},
    {"MEMPTR", (getter)Regs_get_MEMPTR, (setter)Regs_set_MEMPTR, NULL, NULL},
    {"BC", (getter)Regs_get_BC, (setter)Regs_set_BC, NULL, NULL},
    {"DE", (getter)Regs_get_DE, (setter)Regs_set_DE, NULL, NULL},
    {"HL", (getter)Regs_get_HL, (setter)Regs_set_HL, NULL, NULL},
    {"AF", (getter)Regs_get_AF, (setter)Regs_set_AF, NULL, NULL},
    {"IFF1", (getter)Regs_get_IFF1, (setter)Regs_set_IFF1, NULL, NULL},
    {"IFF2", (getter)Regs_get_IFF2, (setter)Regs_set_IFF2, NULL, NULL},
    {"EI_PENDING", (getter)Regs_get_EI_PENDING, (setter)Regs_set_EI_PENDING, NULL, NULL},
    {"EI_JUST_RESOLVED", (getter)Regs_get_EI_JUST_RESOLVED, (setter)Regs_set_EI_JUST_RESOLVED, NULL, NULL},
    {"UNRESOLVED_PREFIX", (getter)Regs_get_UNRESOLVED_PREFIX, (setter)Regs_set_UNRESOLVED_PREFIX, NULL, NULL},
    {"IM", (getter)Regs_get_IM, (setter)Regs_set_IM, NULL, NULL},
    {NULL}
};

static PyTypeObject RegsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_pyz80.Regs",
    sizeof(RegsObject),
    0,
    (destructor)Regs_dealloc,
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_reserved */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash  */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,         /* tp_flags */
    "Z80 Registers",            /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    0,                          /* tp_methods */
    0,                          /* tp_members */
    Regs_getsetters,            /* tp_getset */
};

static PyObject* Regs_new(CPU* cpu) {
    RegsObject* self = PyObject_New(RegsObject, &RegsType);
    if (self) self->cpu = cpu;
    return (PyObject*)self;
}

// ── Z80CPU type ─────────────────────────────────────────────────────
static void Z80CPU_dealloc(Z80CPUObject* self) {
    if (self->cpu) delete self->cpu;
    if (self->py_bus) delete self->py_bus;
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static int Z80CPU_init(Z80CPUObject* self, PyObject* args, PyObject* kwds) {
    PyObject* py_bus_obj = nullptr;
    if (!PyArg_ParseTuple(args, "|O", &py_bus_obj)) return -1;

    self->py_bus = nullptr;

    if (py_bus_obj && py_bus_obj != Py_None) {
        self->py_bus = new PythonBus(py_bus_obj);
        self->cpu = new CPU(self->py_bus);
        self->cpu->_mem = nullptr;
        self->cpu->_is_simple_bus = false;
    } else {
        self->py_bus = new PythonBus(Py_None);
        self->cpu = new CPU(self->py_bus);
        self->cpu->_mem = self->py_bus->memory;
        self->cpu->_is_simple_bus = true;
    }
    return 0;
}

static PyObject* Z80CPU_step(Z80CPUObject* self) {
    return PyLong_FromLong(self->cpu->step());
}

static PyObject* Z80CPU_run_frame(Z80CPUObject* self, PyObject* args) {
    int t_states;
    if (!PyArg_ParseTuple(args, "i", &t_states)) return nullptr;
    return PyLong_FromLong(self->cpu->run_frame(t_states));
}

static PyObject* Z80CPU_add_cycles(Z80CPUObject* self, PyObject* args) {
    int count;
    if (!PyArg_ParseTuple(args, "i", &count)) return nullptr;
    self->cpu->add_cycles(count);
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_set_on_input_callback(Z80CPUObject* self, PyObject* args) {
    PyObject* cb;
    if (!PyArg_ParseTuple(args, "O", &cb)) return nullptr;
    if (self->py_bus) {
        Py_XDECREF(self->py_bus->input_cb);
        Py_INCREF(cb);
        self->py_bus->input_cb = cb;
    }
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_set_on_output_callback(Z80CPUObject* self, PyObject* args) {
    PyObject* cb;
    if (!PyArg_ParseTuple(args, "O", &cb)) return nullptr;
    if (self->py_bus) {
        Py_XDECREF(self->py_bus->output_cb);
        Py_INCREF(cb);
        self->py_bus->output_cb = cb;
    }
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_set_on_reti_callback(Z80CPUObject* self, PyObject* args) {
    PyObject* cb;
    if (!PyArg_ParseTuple(args, "O", &cb)) return nullptr;
    if (self->py_bus) {
        Py_XDECREF(self->py_bus->reti_cb);
        Py_INCREF(cb);
        self->py_bus->reti_cb = cb;
    }
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_set_on_get_int_vector_callback(Z80CPUObject* self, PyObject* args) {
    PyObject* cb;
    if (!PyArg_ParseTuple(args, "O", &cb)) return nullptr;
    if (self->py_bus) {
        Py_XDECREF(self->py_bus->get_int_vector_cb);
        Py_INCREF(cb);
        self->py_bus->get_int_vector_cb = cb;
    }
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_mark_addrs(Z80CPUObject* self, PyObject* args) {
    unsigned int addr, size, marks;
    if (!PyArg_ParseTuple(args, "III", &addr, &size, &marks)) return nullptr;
    if (self->py_bus) {
        for (unsigned i = 0; i < size && (addr + i) < 65536; i++) {
            self->py_bus->addr_marks[(addr + i) & 0xFFFF] |= (uint8_t)marks;
        }
    }
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_unmark_addrs(Z80CPUObject* self, PyObject* args) {
    unsigned int addr, size, marks;
    if (!PyArg_ParseTuple(args, "III", &addr, &size, &marks)) return nullptr;
    if (self->py_bus) {
        for (unsigned i = 0; i < size && (addr + i) < 65536; i++) {
            self->py_bus->addr_marks[(addr + i) & 0xFFFF] &= ~(uint8_t)marks;
        }
    }
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_get_addr_mark(Z80CPUObject* self, PyObject* args) {
    unsigned int addr;
    if (!PyArg_ParseTuple(args, "I", &addr)) return nullptr;
    if (self->py_bus) {
        return PyLong_FromLong(self->py_bus->addr_marks[addr & 0xFFFF]);
    }
    return PyLong_FromLong(0);
}

// Memory View
static PyObject* Z80CPU_get_memory_view(Z80CPUObject* self) {
    if (!self->cpu->_mem) {
        PyErr_SetString(PyExc_RuntimeError, "No direct memory available (not using SimpleBus)");
        return NULL;
    }
    return PyMemoryView_FromMemory(reinterpret_cast<char*>(self->cpu->_mem), 65536, PyBUF_WRITE);
}

// CPU methods
static PyObject* Z80CPU_reset(Z80CPUObject* self) {
    self->cpu->reset();
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_read_byte(Z80CPUObject* self, PyObject* args) {
    unsigned int addr;
    if (!PyArg_ParseTuple(args, "I", &addr)) return nullptr;
    return PyLong_FromLong(self->cpu->_bus_read((uint16_t)addr));
}

static PyObject* Z80CPU_write_byte(Z80CPUObject* self, PyObject* args) {
    unsigned int addr, value;
    if (!PyArg_ParseTuple(args, "II", &addr, &value)) return nullptr;
    self->cpu->_bus_write((uint16_t)addr, (uint8_t)value);
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_trigger_nmi(Z80CPUObject* self) {
    self->cpu->trigger_nmi();
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_trigger_interrupt(Z80CPUObject* self, PyObject* args) {
    unsigned int data;
    if (!PyArg_ParseTuple(args, "I", &data)) return nullptr;
    self->cpu->trigger_interrupt((uint8_t)data);
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_invalidate_range(Z80CPUObject* self, PyObject* args) {
    unsigned int start, end;
    if (!PyArg_ParseTuple(args, "II", &start, &end)) return nullptr;
    for (unsigned int addr = start; addr <= end && addr < 65536; ++addr) {
        self->cpu->decoder.invalidate((uint16_t)addr);
    }
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_set_memory_ptr(Z80CPUObject* self, PyObject* args) {
    PyObject* mv;
    if (!PyArg_ParseTuple(args, "O", &mv)) return nullptr;
    Py_buffer view;
    if (PyObject_GetBuffer(mv, &view, PyBUF_WRITABLE) < 0) return nullptr;
    if (view.len != 65536) {
        PyBuffer_Release(&view);
        PyErr_SetString(PyExc_ValueError, "Memory buffer must be exactly 65536 bytes");
        return nullptr;
    }
    self->cpu->_mem = (uint8_t*)view.buf;
    self->cpu->_is_simple_bus = true;
    PyBuffer_Release(&view);
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_sync_memory(Z80CPUObject* self, PyObject* args) {
    // Sync a region of machine memory into PythonBus memory
    // Args: (offset, data_bytes)
    Py_ssize_t offset;
    PyObject* data;
    if (!PyArg_ParseTuple(args, "nO", &offset, &data)) return nullptr;
    if (!self->py_bus) {
        PyErr_SetString(PyExc_RuntimeError, "No PythonBus available");
        return nullptr;
    }
    Py_buffer view;
    if (PyObject_GetBuffer(data, &view, PyBUF_SIMPLE) < 0) return nullptr;
    if (offset < 0 || offset + view.len > 65536) {
        PyBuffer_Release(&view);
        PyErr_SetString(PyExc_ValueError, "Memory region out of range");
        return nullptr;
    }
    std::memcpy(self->py_bus->memory + offset, view.buf, view.len);
    PyBuffer_Release(&view);
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_sync_memory_range(Z80CPUObject* self, PyObject* args) {
    // Sync a single range: (offset, length, value)
    Py_ssize_t offset, length;
    int value;
    if (!PyArg_ParseTuple(args, "nni", &offset, &length, &value)) return nullptr;
    if (!self->py_bus) {
        PyErr_SetString(PyExc_RuntimeError, "No PythonBus available");
        return nullptr;
    }
    if (offset < 0 || offset + length > 65536) {
        PyErr_SetString(PyExc_ValueError, "Memory region out of range");
        return nullptr;
    }
    std::memset(self->py_bus->memory + offset, value & 0xFF, length);
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_invalidate_all(Z80CPUObject* self) {
    self->cpu->decoder.invalidate_all();
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_run(Z80CPUObject* self, PyObject* args) {
    int max_cycles;
    if (!PyArg_ParseTuple(args, "i", &max_cycles)) return nullptr;
    return PyLong_FromLong(self->cpu->run(max_cycles));
}

static PyObject* Z80CPU_run_instructions(Z80CPUObject* self, PyObject* args) {
    int count;
    if (!PyArg_ParseTuple(args, "i", &count)) return nullptr;
    return PyLong_FromLong(self->cpu->run_instructions(count));
}

static PyObject* Z80CPU_io_read(Z80CPUObject* self, PyObject* args) {
    unsigned int port;
    if (!PyArg_ParseTuple(args, "I", &port)) return nullptr;
    return PyLong_FromLong(self->cpu->_bus_io_read((uint16_t)port));
}

static PyObject* Z80CPU_io_write(Z80CPUObject* self, PyObject* args) {
    unsigned int port, value;
    if (!PyArg_ParseTuple(args, "II", &port, &value)) return nullptr;
    self->cpu->_bus_io_write((uint16_t)port, (uint8_t)value);
    Py_RETURN_NONE;
}

static PyObject* Z80CPU_get_current_opcode(Z80CPUObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->current_opcode);
}

static PyObject* Z80CPU_get_regs(Z80CPUObject* self, void* closure) {
    return Regs_new(self->cpu);
}

// Direct register accessors on CPU object (machine-agnostic flat API)
#define CPU_REG8_GETSET(name, type) \
static PyObject* Z80CPU_get_##name(Z80CPUObject* self, void* closure) { \
    return PyLong_FromLong(self->cpu->regs.name); \
} \
static int Z80CPU_set_##name(Z80CPUObject* self, PyObject* value, void* closure) { \
    self->cpu->regs.name = (type)PyLong_AsLong(value); \
    return 0; \
}

CPU_REG8_GETSET(A, uint8_t)
CPU_REG8_GETSET(F, uint8_t)
CPU_REG8_GETSET(B, uint8_t)
CPU_REG8_GETSET(C, uint8_t)
CPU_REG8_GETSET(D, uint8_t)
CPU_REG8_GETSET(E, uint8_t)
CPU_REG8_GETSET(H, uint8_t)
CPU_REG8_GETSET(L, uint8_t)
CPU_REG8_GETSET(Ap, uint8_t)
CPU_REG8_GETSET(Fp, uint8_t)
CPU_REG8_GETSET(Bp, uint8_t)
CPU_REG8_GETSET(Cp, uint8_t)
CPU_REG8_GETSET(Dp, uint8_t)
CPU_REG8_GETSET(Ep, uint8_t)
CPU_REG8_GETSET(Hp, uint8_t)
CPU_REG8_GETSET(Lp, uint8_t)
CPU_REG8_GETSET(I, uint8_t)
CPU_REG8_GETSET(R, uint8_t)
CPU_REG8_GETSET(Q, uint8_t)
CPU_REG8_GETSET(LAST_Q, uint8_t)
CPU_REG8_GETSET(PC, uint16_t)
CPU_REG8_GETSET(SP, uint16_t)
CPU_REG8_GETSET(IX, uint16_t)
CPU_REG8_GETSET(IY, uint16_t)
CPU_REG8_GETSET(MEMPTR, uint16_t)

// Boolean flags on CPU
static PyObject* Z80CPU_get_IFF1(Z80CPUObject* self, void* closure) {
    return PyBool_FromLong(self->cpu->regs.IFF1);
}
static int Z80CPU_set_IFF1(Z80CPUObject* self, PyObject* value, void* closure) {
    self->cpu->regs.IFF1 = PyObject_IsTrue(value);
    return 0;
}

static PyObject* Z80CPU_get_IFF2(Z80CPUObject* self, void* closure) {
    return PyBool_FromLong(self->cpu->regs.IFF2);
}
static int Z80CPU_set_IFF2(Z80CPUObject* self, PyObject* value, void* closure) {
    self->cpu->regs.IFF2 = PyObject_IsTrue(value);
    return 0;
}

static PyObject* Z80CPU_get_IM(Z80CPUObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->regs.IM);
}
static int Z80CPU_set_IM(Z80CPUObject* self, PyObject* value, void* closure) {
    self->cpu->regs.IM = (uint8_t)PyLong_AsLong(value);
    return 0;
}

// Compound 16-bit registers
static PyObject* Z80CPU_get_BC(Z80CPUObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->regs.BC());
}
static int Z80CPU_set_BC(Z80CPUObject* self, PyObject* value, void* closure) {
    self->cpu->regs.set_BC((uint16_t)PyLong_AsLong(value));
    return 0;
}

static PyObject* Z80CPU_get_DE(Z80CPUObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->regs.DE());
}
static int Z80CPU_set_DE(Z80CPUObject* self, PyObject* value, void* closure) {
    self->cpu->regs.set_DE((uint16_t)PyLong_AsLong(value));
    return 0;
}

static PyObject* Z80CPU_get_HL(Z80CPUObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->regs.HL());
}
static int Z80CPU_set_HL(Z80CPUObject* self, PyObject* value, void* closure) {
    self->cpu->regs.set_HL((uint16_t)PyLong_AsLong(value));
    return 0;
}

static PyObject* Z80CPU_get_AF(Z80CPUObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->regs.AF());
}
static int Z80CPU_set_AF(Z80CPUObject* self, PyObject* value, void* closure) {
    self->cpu->regs.set_AF((uint16_t)PyLong_AsLong(value));
    return 0;
}

// cycles attribute
static PyObject* Z80CPU_get_cycles(Z80CPUObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->cycles);
}
static int Z80CPU_set_cycles(Z80CPUObject* self, PyObject* value, void* closure) {
    self->cpu->cycles = (int)PyLong_AsLong(value);
    return 0;
}

// halted attribute
static PyObject* Z80CPU_get_halted(Z80CPUObject* self, void* closure) {
    return PyBool_FromLong(self->cpu->halted);
}
static int Z80CPU_set_halted(Z80CPUObject* self, PyObject* value, void* closure) {
    self->cpu->halted = PyObject_IsTrue(value);
    return 0;
}

// interrupt_pending attribute
static PyObject* Z80CPU_get_interrupt_pending(Z80CPUObject* self, void* closure) {
    return PyBool_FromLong(self->cpu->interrupt_pending);
}
static int Z80CPU_set_interrupt_pending(Z80CPUObject* self, PyObject* value, void* closure) {
    self->cpu->interrupt_pending = PyObject_IsTrue(value);
    return 0;
}

// nmi_pending attribute
static PyObject* Z80CPU_get_nmi_pending(Z80CPUObject* self, void* closure) {
    return PyBool_FromLong(self->cpu->nmi_pending);
}
static int Z80CPU_set_nmi_pending(Z80CPUObject* self, PyObject* value, void* closure) {
    self->cpu->nmi_pending = PyObject_IsTrue(value);
    return 0;
}

// interrupt_data attribute
static PyObject* Z80CPU_get_interrupt_data(Z80CPUObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->interrupt_data);
}
static int Z80CPU_set_interrupt_data(Z80CPUObject* self, PyObject* value, void* closure) {
    self->cpu->interrupt_data = (uint8_t)PyLong_AsLong(value);
    return 0;
}

// instruction_count attribute
static PyObject* Z80CPU_get_instruction_count(Z80CPUObject* self, void* closure) {
    return PyLong_FromLong(self->cpu->instruction_count);
}
static int Z80CPU_set_instruction_count(Z80CPUObject* self, PyObject* value, void* closure) {
    self->cpu->instruction_count = (int)PyLong_AsLong(value);
    return 0;
}

static PyObject* Z80CPU_get_last_read_addr(Z80CPUObject* self, void* closure) {
    uint16_t addr = 0;
    if (self->py_bus) {
        addr = self->py_bus->last_read_addr;
    }
    return PyLong_FromLong(addr);
}

static PyGetSetDef Z80CPU_getsetters[] = {
    // 8-bit registers
    {"A", (getter)Z80CPU_get_A, (setter)Z80CPU_set_A, "Accumulator", NULL},
    {"F", (getter)Z80CPU_get_F, (setter)Z80CPU_set_F, "Flags", NULL},
    {"B", (getter)Z80CPU_get_B, (setter)Z80CPU_set_B, NULL, NULL},
    {"C", (getter)Z80CPU_get_C, (setter)Z80CPU_set_C, NULL, NULL},
    {"D", (getter)Z80CPU_get_D, (setter)Z80CPU_set_D, NULL, NULL},
    {"E", (getter)Z80CPU_get_E, (setter)Z80CPU_set_E, NULL, NULL},
    {"H", (getter)Z80CPU_get_H, (setter)Z80CPU_set_H, NULL, NULL},
    {"L", (getter)Z80CPU_get_L, (setter)Z80CPU_set_L, NULL, NULL},
    {"Ap", (getter)Z80CPU_get_Ap, (setter)Z80CPU_set_Ap, NULL, NULL},
    {"Fp", (getter)Z80CPU_get_Fp, (setter)Z80CPU_set_Fp, NULL, NULL},
    {"Bp", (getter)Z80CPU_get_Bp, (setter)Z80CPU_set_Bp, NULL, NULL},
    {"Cp", (getter)Z80CPU_get_Cp, (setter)Z80CPU_set_Cp, NULL, NULL},
    {"Dp", (getter)Z80CPU_get_Dp, (setter)Z80CPU_set_Dp, NULL, NULL},
    {"Ep", (getter)Z80CPU_get_Ep, (setter)Z80CPU_set_Ep, NULL, NULL},
    {"Hp", (getter)Z80CPU_get_Hp, (setter)Z80CPU_set_Hp, NULL, NULL},
    {"Lp", (getter)Z80CPU_get_Lp, (setter)Z80CPU_set_Lp, NULL, NULL},
    {"I", (getter)Z80CPU_get_I, (setter)Z80CPU_set_I, NULL, NULL},
    {"R", (getter)Z80CPU_get_R, (setter)Z80CPU_set_R, NULL, NULL},
    {"Q", (getter)Z80CPU_get_Q, (setter)Z80CPU_set_Q, NULL, NULL},
    {"LAST_Q", (getter)Z80CPU_get_LAST_Q, (setter)Z80CPU_set_LAST_Q, NULL, NULL},
    // 16-bit registers
    {"PC", (getter)Z80CPU_get_PC, (setter)Z80CPU_set_PC, "Program counter", NULL},
    {"SP", (getter)Z80CPU_get_SP, (setter)Z80CPU_set_SP, "Stack pointer", NULL},
    {"IX", (getter)Z80CPU_get_IX, (setter)Z80CPU_set_IX, "Index register X", NULL},
    {"IY", (getter)Z80CPU_get_IY, (setter)Z80CPU_set_IY, "Index register Y", NULL},
    {"MEMPTR", (getter)Z80CPU_get_MEMPTR, (setter)Z80CPU_set_MEMPTR, NULL, NULL},
    // Boolean flags
    {"IFF1", (getter)Z80CPU_get_IFF1, (setter)Z80CPU_set_IFF1, NULL, NULL},
    {"IFF2", (getter)Z80CPU_get_IFF2, (setter)Z80CPU_set_IFF2, NULL, NULL},
    {"IM", (getter)Z80CPU_get_IM, (setter)Z80CPU_set_IM, NULL, NULL},
    // Compound 16-bit registers
    {"BC", (getter)Z80CPU_get_BC, (setter)Z80CPU_set_BC, NULL, NULL},
    {"DE", (getter)Z80CPU_get_DE, (setter)Z80CPU_set_DE, NULL, NULL},
    {"HL", (getter)Z80CPU_get_HL, (setter)Z80CPU_set_HL, NULL, NULL},
    {"AF", (getter)Z80CPU_get_AF, (setter)Z80CPU_set_AF, NULL, NULL},
    // CPU state
    {"regs", (getter)Z80CPU_get_regs, NULL, "Register file", NULL},
    {"cycles", (getter)Z80CPU_get_cycles, (setter)Z80CPU_set_cycles, "T-state counter", NULL},
    {"halted", (getter)Z80CPU_get_halted, (setter)Z80CPU_set_halted, "Halt state", NULL},
    {"interrupt_pending", (getter)Z80CPU_get_interrupt_pending, (setter)Z80CPU_set_interrupt_pending, "Interrupt pending flag", NULL},
    {"nmi_pending", (getter)Z80CPU_get_nmi_pending, (setter)Z80CPU_set_nmi_pending, "NMI pending flag", NULL},
    {"interrupt_data", (getter)Z80CPU_get_interrupt_data, (setter)Z80CPU_set_interrupt_data, "Interrupt data bus value", NULL},
    {"instruction_count", (getter)Z80CPU_get_instruction_count, (setter)Z80CPU_set_instruction_count, "Instruction counter", NULL},
    {"current_opcode", (getter)Z80CPU_get_current_opcode, NULL, "Last executed opcode byte", NULL},
    {"last_read_addr", (getter)Z80CPU_get_last_read_addr, NULL, "Last memory read address", NULL},
    {NULL}
};

static PyMethodDef Z80CPU_methods[] = {
    {"step", (PyCFunction)Z80CPU_step, METH_NOARGS, "Execute one instruction"},
    {"run_frame", (PyCFunction)Z80CPU_run_frame, METH_VARARGS, "Run a frame"},
    {"add_cycles", (PyCFunction)Z80CPU_add_cycles, METH_VARARGS, "Add contention"},
    {"set_on_input_callback", (PyCFunction)Z80CPU_set_on_input_callback, METH_VARARGS, "Set input CB"},
    {"set_on_output_callback", (PyCFunction)Z80CPU_set_on_output_callback, METH_VARARGS, "Set output CB"},
    {"_get_memory_view", (PyCFunction)Z80CPU_get_memory_view, METH_NOARGS, "Get memory"},
    {"reset", (PyCFunction)Z80CPU_reset, METH_NOARGS, "Reset CPU"},
    {"read_byte", (PyCFunction)Z80CPU_read_byte, METH_VARARGS, "Read byte from memory"},
    {"write_byte", (PyCFunction)Z80CPU_write_byte, METH_VARARGS, "Write byte to memory"},
    {"trigger_nmi", (PyCFunction)Z80CPU_trigger_nmi, METH_NOARGS, "Trigger NMI"},
    {"trigger_interrupt", (PyCFunction)Z80CPU_trigger_interrupt, METH_VARARGS, "Trigger interrupt"},
    {"invalidate_range", (PyCFunction)Z80CPU_invalidate_range, METH_VARARGS, "Invalidate decoder cache range"},
    {"invalidate_all", (PyCFunction)Z80CPU_invalidate_all, METH_NOARGS, "Invalidate all decoder cache"},
    {"run", (PyCFunction)Z80CPU_run, METH_VARARGS, "Run for max T-states"},
    {"run_instructions", (PyCFunction)Z80CPU_run_instructions, METH_VARARGS, "Run N instructions"},
    {"io_read", (PyCFunction)Z80CPU_io_read, METH_VARARGS, "Read I/O port"},
    {"io_write", (PyCFunction)Z80CPU_io_write, METH_VARARGS, "Write I/O port"},
    {"set_on_reti_callback", (PyCFunction)Z80CPU_set_on_reti_callback, METH_VARARGS, "Set RETI callback"},
    {"set_on_get_int_vector_callback", (PyCFunction)Z80CPU_set_on_get_int_vector_callback, METH_VARARGS, "Set INT vector callback"},
    {"mark_addrs", (PyCFunction)Z80CPU_mark_addrs, METH_VARARGS, "Mark memory addresses"},
    {"unmark_addrs", (PyCFunction)Z80CPU_unmark_addrs, METH_VARARGS, "Unmark memory addresses"},
    {"get_addr_mark", (PyCFunction)Z80CPU_get_addr_mark, METH_VARARGS, "Get address mark"},
    {"sync_memory", (PyCFunction)Z80CPU_sync_memory, METH_VARARGS, "Sync machine memory into CPU memory"},
    {"sync_memory_range", (PyCFunction)Z80CPU_sync_memory_range, METH_VARARGS, "Fill CPU memory range with value"},
    {"set_memory_ptr", (PyCFunction)Z80CPU_set_memory_ptr, METH_VARARGS, "Point CPU memory at external buffer"},
    {NULL}
};

static PyTypeObject Z80CPUType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_pyz80.Z80CPU",            /* tp_name */
    sizeof(Z80CPUObject),       /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor)Z80CPU_dealloc, /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_reserved */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash  */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,         /* tp_flags */
    "Z80 CPU object",           /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    Z80CPU_methods,             /* tp_methods */
    0,                          /* tp_members */
    Z80CPU_getsetters,          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)Z80CPU_init,      /* tp_init */
    0,                          /* tp_alloc */
    PyType_GenericNew,          /* tp_new */
};

static struct PyModuleDef pyz80module = {
    PyModuleDef_HEAD_INIT,
    "_pyz80",
    NULL,
    -1,
    NULL,
};

PyMODINIT_FUNC PyInit__pyz80(void) {
    PyObject* m;
    if (PyType_Ready(&RegsType) < 0) return NULL;
    if (PyType_Ready(&Z80CPUType) < 0) return NULL;
    m = PyModule_Create(&pyz80module);
    if (!m) return NULL;
    Py_INCREF(&RegsType);
    PyModule_AddObject(m, "Regs", (PyObject*)&RegsType);
    Py_INCREF(&Z80CPUType);
    PyModule_AddObject(m, "Z80CPU", (PyObject*)&Z80CPUType);

    PyModule_AddIntConstant(m, "FLAG_S", 0x80);
    PyModule_AddIntConstant(m, "FLAG_Z", 0x40);
    PyModule_AddIntConstant(m, "FLAG_F5", 0x20);
    PyModule_AddIntConstant(m, "FLAG_H", 0x10);
    PyModule_AddIntConstant(m, "FLAG_F3", 0x08);
    PyModule_AddIntConstant(m, "FLAG_PV", 0x04);
    PyModule_AddIntConstant(m, "FLAG_N", 0x02);
    PyModule_AddIntConstant(m, "FLAG_C", 0x01);

    PyModule_AddIntConstant(m, "MACHINE_STATE_SIZE", 0);

    return m;
}
