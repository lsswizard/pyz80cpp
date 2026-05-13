#include "../include/z80/z80.h"
#include "../include/z80/opcode_table.h"
#include <cstring>
#include <algorithm>
#include <unordered_map>

namespace z80 {

// ============================================================
// Constructor / Destructor
// ============================================================
Z80::Z80(Bus* bus)
    : bus_ptr(bus)
    , owns_bus(false)
{
    if (!bus_ptr) {
        bus_ptr = new SimpleBus();
        owns_bus = true;
    }

    OpcodeTable::init();
    reset();
}

Z80::~Z80() {
    if (owns_bus) delete bus_ptr;
}

void Z80::set_bus(Bus* new_bus) {
    if (owns_bus) delete bus_ptr;
    bus_ptr  = new_bus ? new_bus : new SimpleBus();
    owns_bus = (new_bus == nullptr);
}

// ============================================================
// Reset  (Z80 CPU User Manual §2.4)
// AF=0xFFFF, other regs undefined; IFF=0; IM=0; PC=0000
// ============================================================
void Z80::reset() {
    regs.reset();
    total_cycles      = 0;
    t_state           = 0;
    instruction_count = 0;
    halted            = false;
    interrupt_pending = false;
    nmi_pending       = false;
    interrupt_data    = 0xFF;
    current_opcode    = 0;
    prefix_ix         = false;
    ddcb_displacement = 0;
    ddcb_opcode       = 0;
    trap_address      = 0xFFFF;
}

// ============================================================
// step() — execute one "event": NMI, INT, HALT tick, or one instruction
//
// Timing references (Z80 CPU User Manual):
//   NMI response  : 11 T-states  (5 M1 ack + 3+3 push)
//   IM 0 response : 13 T-states  (6 INT-ack M1 + 3+3 push + 1 internal)
//   IM 1 response : 13 T-states  (6 INT-ack M1 + 3+3 push + 1 internal)
//   IM 2 response : 19 T-states  (6 INT-ack M1 + 3+3 push + 3+3 vector + 1 internal)
//
// push() contributes 3+3 = 6 T-states via two write() calls.
// wait(N) adds N idle T-states.
// ============================================================
int Z80::step() {
    int start = total_cycles;

    // --------------------------------------------------------
    // NMI — highest priority; not maskable
    // Timing: 5 (M1 ack) + 6 (push PC) = 11 T-states
    // --------------------------------------------------------
    if (nmi_pending) {
        nmi_pending = false;
        
        if (halted) {
            halted = false;
            regs.PC = (regs.PC + 1) & 0xFFFF;
        }

        // IFF1 is saved to IFF2 then cleared; IFF2 is unmodified by NMI per spec
        regs.IFF2 = regs.IFF1;
        regs.IFF1 = false;

        // R incremented during NMI M1-like acknowledge cycle
        regs.R = (regs.R & 0x80) | ((regs.R + 1) & 0x7F);

        wait(5);          // NMI acknowledge cycle
        push(regs.PC);    // 3+3 T-states
        regs.PC     = 0x0066;
        regs.MEMPTR = 0x0066;
        return total_cycles - start;
    }

    // --------------------------------------------------------
    // Maskable interrupt (INT)
    //
    // Suppressed when:
    //   • IFF1 is clear
    //   • EI_JUST_RESOLVED: the one instruction immediately after EI
    //   • UnresolvedPrefix: CPU is mid-prefix (DD/FD/CB)
    // --------------------------------------------------------
    if (interrupt_pending && regs.IFF1
            && !regs.EI_JUST_RESOLVED
            && !regs.UnresolvedPrefix) {
        interrupt_pending = false;
        regs.IFF1 = regs.IFF2 = false;

        if (halted) {
            halted = false;
            regs.PC = (regs.PC + 1) & 0xFFFF;
        }

        // INT-acknowledge M1 cycle: 6 T-states (4 base + 2 extra wait).
        // R IS incremented during the acknowledge cycle per Z80 spec.
        regs.R = (regs.R & 0x80) | ((regs.R + 1) & 0x7F);
        uint8_t vec = bus_ptr->interrupt_acknowledge();
        wait(6);

        switch (regs.IM) {
            case 0:
                // IM 0: device places an instruction (typically RST n) on the bus.
                // Only RST is handled here; full instruction execution is not supported.
                // Total: 6 (ack) + 6 (push) + 1 (internal) = 13 T-states.
                push(regs.PC);
                regs.PC     = vec & 0x38;  // RST vector from data bus
                regs.MEMPTR = regs.PC;
                wait(1);
                break;

            case 1:
                // IM 1: unconditional jump to 0x0038.
                // Total: 6 (ack) + 6 (push) + 1 (internal) = 13 T-states.
                push(regs.PC);
                regs.PC     = 0x0038;
                regs.MEMPTR = 0x0038;
                wait(1);
                break;

            case 2: {
                // IM 2: vectored via I register.
                // Vector address = (I << 8) | (vec & 0xFE)  (bit 0 always 0).
                // Total: 6 (ack) + 6 (push) + 3+3 (vector reads) + 1 (internal) = 19 T-states.
                push(regs.PC);
                uint16_t va = (uint16_t(regs.I) << 8) | (vec & 0xFE);
                // FIX: use this->read() so wait-states and contend hook are applied
                uint8_t lo = read(va);
                uint8_t hi = read((va + 1) & 0xFFFF);
                regs.PC     = uint16_t(lo) | (uint16_t(hi) << 8);
                regs.MEMPTR = regs.PC;
                wait(1);
                break;
            }

            default:
                // Undefined IM — hardware acts as IM 1
                push(regs.PC);
                regs.PC     = 0x0038;
                regs.MEMPTR = 0x0038;
                wait(1);
                break;
        }

        return total_cycles - start;
    }

    // --------------------------------------------------------
    // HALT — re-execute NOP (4 T-states) until interrupt fires
    // R IS incremented every HALT NOP per spec
    // --------------------------------------------------------
    if (halted) {
        regs.R = (regs.R & 0x80) | ((regs.R + 1) & 0x7F);
        wait(4);
        return 4;
    }

    // --------------------------------------------------------
    // Normal instruction
    // --------------------------------------------------------
    execute_instruction();
    return total_cycles - start;
}

// ============================================================
// execute_instruction
// ============================================================
void Z80::execute_instruction() {
    // EI two-phase (Part A): clear the one-instruction delay flag
    if (regs.EI_JUST_RESOLVED)
        regs.EI_JUST_RESOLVED = false;

    // Fetch opcode (M1: 4 T-states, R incremented inside fetch_opcode())
    uint8_t opcode = fetch_opcode();
    current_opcode = opcode;

    if (opcode == 0xCB) {
        opcode = fetch_opcode();   // second M1: R also incremented
        current_opcode = opcode;
        const Instruction& ins = OpcodeTable::get_cb(opcode);
        if (ins.exec) ins.exec(*this);

    } else if (opcode == 0xED) {
        opcode = fetch_opcode();   // second M1: R also incremented
        current_opcode = opcode;
        const Instruction& ins = OpcodeTable::get_ed(opcode);
        if (ins.exec) ins.exec(*this);

    } else if (opcode == 0xDD) {
        prefix_ix = true;
        opcode = fetch_opcode();   // second M1: R also incremented

        if (opcode == 0xCB) {
            ddcb_displacement = (int8_t)fetch_byte();
            ddcb_opcode       = fetch_opcode();  // DDCB opcode is an M1 cycle (4 T, R++)
            current_opcode    = ddcb_opcode;
            const Instruction& ins = OpcodeTable::get_ddcb(ddcb_opcode);
            if (ins.exec) ins.exec(*this);
        } else {
            current_opcode = opcode;
            const Instruction& ins = OpcodeTable::get_dd(opcode);
            if (ins.exec) {
                ins.exec(*this);
            } else {
                // Unrecognised DD-opcode falls through to main table
                const Instruction& mi = OpcodeTable::get_main(opcode);
                if (mi.exec) mi.exec(*this);
            }
        }
        prefix_ix = false;

    } else if (opcode == 0xFD) {
        prefix_ix = false;
        opcode = fetch_opcode();   // second M1: R also incremented

        if (opcode == 0xCB) {
            ddcb_displacement = (int8_t)fetch_byte();
            ddcb_opcode       = fetch_opcode();  // FDCB opcode is an M1 cycle (4 T, R++)
            current_opcode    = ddcb_opcode;
            const Instruction& ins = OpcodeTable::get_fdcb(ddcb_opcode);
            if (ins.exec) ins.exec(*this);
        } else {
            current_opcode = opcode;
            const Instruction& ins = OpcodeTable::get_fd(opcode);
            if (ins.exec) {
                ins.exec(*this);
            } else {
                const Instruction& mi = OpcodeTable::get_main(opcode);
                if (mi.exec) mi.exec(*this);
            }
        }
        prefix_ix = false;

    } else {
        const Instruction& ins = OpcodeTable::get_main(opcode);
        if (ins.exec) ins.exec(*this);
    }

    // EI two-phase (Part B): if EI was just executed, enable IFFs
    if (regs.EI_PENDING) {
        regs.EI_PENDING       = false;
        regs.EI_JUST_RESOLVED = true;
        regs.IFF1 = regs.IFF2 = true;
    }

    instruction_count++;
}

// ============================================================
// run() / run_instructions()
// ============================================================
int Z80::run(int max_cycles) {
    // Run until total_cycles reaches target.
    // The caller passes the frame-relative cycle limit as max_cycles.
    // Note: t_state is NOT reset here — Python must reset it at frame boundaries.
    int start_total = total_cycles;
    int target = start_total + max_cycles;

    while (total_cycles < target) {
        if (regs.PC == trap_address) break;
        step();
    }

    return total_cycles - start_total;
}
int Z80::run_instructions(int count) {
    int start = instruction_count;
    for (int i = 0; i < count && !halted; ++i)
        step();
    return instruction_count - start;
}

// ============================================================
// Interrupt triggers
// ============================================================
void Z80::trigger_interrupt(uint8_t data) {
    interrupt_data    = data;
    interrupt_pending = true;
}

void Z80::trigger_nmi() {
    nmi_pending = true;
}

// ============================================================
// 8-bit register read/write
// reg encoding: 0=B 1=C 2=D 3=E 4=H 5=L 6=(HL) 7=A
// Extended (DD/FD context): 8=IXH/IYH  9=IXL/IYL
// ============================================================
uint8_t Z80::read_reg8(int reg) {
    switch (reg) {
        case 0: return regs.B;
        case 1: return regs.C;
        case 2: return regs.D;
        case 3: return regs.E;
        case 4: return regs.H;
        case 5: return regs.L;
        case 6: return read(regs.HL());
        case 7: return regs.A;
        case 8: return prefix_ix ? regs.IXh() : regs.IYh();
        case 9: return prefix_ix ? regs.IXl() : regs.IYl();
        default: return 0;
    }
}

void Z80::write_reg8(int reg, uint8_t val) {
    switch (reg) {
        case 0: regs.B = val; break;
        case 1: regs.C = val; break;
        case 2: regs.D = val; break;
        case 3: regs.E = val; break;
        case 4: regs.H = val; break;
        case 5: regs.L = val; break;
        case 6: write(regs.HL(), val); break;
        case 7: regs.A = val; break;
        case 8:
            if (prefix_ix) regs.set_IXh(val); else regs.set_IYh(val);
            break;
        case 9:
            if (prefix_ix) regs.set_IXl(val); else regs.set_IYl(val);
            break;
        default: break;
    }
}

// ============================================================
// set_state — bulk register load from Python dict
// ============================================================
void Z80::set_state(const std::unordered_map<std::string, int>& s) {
    auto get = [&](const char* k, auto cur) {
        auto it = s.find(k);
        return it != s.end() ? decltype(cur)(it->second) : cur;
    };
    regs.A  = get("A",  regs.A);
    regs.F  = get("F",  regs.F);
    regs.B  = get("B",  regs.B);
    regs.C  = get("C",  regs.C);
    regs.D  = get("D",  regs.D);
    regs.E  = get("E",  regs.E);
    regs.H  = get("H",  regs.H);
    regs.L  = get("L",  regs.L);
    regs.Ap = get("Ap", regs.Ap);
    regs.Fp = get("Fp", regs.Fp);
    regs.Bp = get("Bp", regs.Bp);
    regs.Cp = get("Cp", regs.Cp);
    regs.Dp = get("Dp", regs.Dp);
    regs.Ep = get("Ep", regs.Ep);
    regs.Hp = get("Hp", regs.Hp);
    regs.Lp = get("Lp", regs.Lp);
    regs.IX = get("IX", regs.IX);
    regs.IY = get("IY", regs.IY);
    regs.SP = get("SP", regs.SP);
    regs.PC = get("PC", regs.PC);
    regs.I  = get("I",  regs.I);
    regs.R  = get("R",  regs.R);
    regs.IFF1 = get("IFF1", (int)regs.IFF1);
    regs.IFF2 = get("IFF2", (int)regs.IFF2);
    regs.IM   = get("IM",   regs.IM);
}

} // namespace z80
