#pragma once

#include <cstdint>
#include <algorithm>

namespace z80 {

// ============================================================
// Register State
// ============================================================
struct Registers {
    // Main registers
    uint8_t A, F;
    uint8_t B, C;
    uint8_t D, E;
    uint8_t H, L;

    // Alternate (shadow) registers
    uint8_t Ap, Fp;
    uint8_t Bp, Cp;
    uint8_t Dp, Ep;
    uint8_t Hp, Lp;

    // 16-bit registers
    uint16_t IX;
    uint16_t IY;
    uint16_t SP;
    uint16_t PC;

    // Special purpose
    uint8_t I;   // Interrupt vector base
    uint8_t R;   // Memory refresh register (7-bit counter, bit 7 preserved)

    // Interrupt state
    bool IFF1;
    bool IFF2;
    uint8_t IM;  // Interrupt mode: 0, 1, or 2

    // EI two-phase enable
    bool EI_PENDING;        // Set by EI; cleared one instruction later
    bool EI_JUST_RESOLVED;  // True for the single instruction after EI fires

    // Unresolved prefix flag (DD/FD/CB prefix in progress)
    bool UnresolvedPrefix;

    // Memory pointer register (MEMPTR / WZ) — used by several instructions
    uint16_t MEMPTR;

    // Q register — tracks whether last instruction modified F (for CCF/SCF F3/F5)
    uint8_t Q;

    // ============================================================
    // 16-bit register accessors
    // ============================================================
    uint16_t BC() const { return (uint16_t(B) << 8) | C; }
    void set_BC(uint16_t v) { B = v >> 8; C = v & 0xFF; }

    uint16_t DE() const { return (uint16_t(D) << 8) | E; }
    void set_DE(uint16_t v) { D = v >> 8; E = v & 0xFF; }

    uint16_t HL() const { return (uint16_t(H) << 8) | L; }
    void set_HL(uint16_t v) { H = v >> 8; L = v & 0xFF; }

    uint16_t AF() const { return (uint16_t(A) << 8) | F; }
    void set_AF(uint16_t v) { A = v >> 8; F = v & 0xFF; }

    // ============================================================
    // Index register half-byte accessors
    // ============================================================
    uint8_t IXh() const { return IX >> 8; }
    void set_IXh(uint8_t v) { IX = (IX & 0x00FF) | (uint16_t(v) << 8); }

    uint8_t IXl() const { return IX & 0xFF; }
    void set_IXl(uint8_t v) { IX = (IX & 0xFF00) | v; }

    uint8_t IYh() const { return IY >> 8; }
    void set_IYh(uint8_t v) { IY = (IY & 0x00FF) | (uint16_t(v) << 8); }

    uint8_t IYl() const { return IY & 0xFF; }
    void set_IYl(uint8_t v) { IY = (IY & 0xFF00) | v; }

    // ============================================================
    // Shadow register swaps
    // ============================================================
    void swap_af() {
        std::swap(A, Ap);
        std::swap(F, Fp);
    }

    // EXX: swap BC↔BC', DE↔DE', HL↔HL'   (Z80 opcode 0xD9)
    // BUG FIX: the original code swapped DE↔HL instead of DE↔DE' and HL↔HL'
    void swap_all() {
        std::swap(B, Bp); std::swap(C, Cp);
        std::swap(D, Dp); std::swap(E, Ep);
        std::swap(H, Hp); std::swap(L, Lp);
    }

    // ============================================================
    // Reset state (per Z80 CPU User Manual, Table 3)
    // ============================================================
    void reset() {
        A = F = 0xFF;           // AF = 0xFFFF after reset
        B = C = D = E = H = L = 0xFF;
        Ap = Fp = Bp = Cp = Dp = Ep = Hp = Lp = 0;
        IX = IY = 0xFFFF;
        SP = 0xFFFF;
        PC = 0x0000;
        I  = 0x00;
        R  = 0x00;
        IFF1 = IFF2 = false;
        IM   = 0;
        EI_PENDING = EI_JUST_RESOLVED = false;
        UnresolvedPrefix = false;
        MEMPTR = 0;
        Q = 0;
    }
};

} // namespace z80
