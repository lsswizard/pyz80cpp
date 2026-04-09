#pragma once

#include <cstdint>
#include <cstring>

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
    uint8_t I;    // Interrupt vector base
    uint8_t R;    // Refresh register

    // Interrupt state
    bool IFF1;
    bool IFF2;
    uint8_t IM;   // Interrupt mode (0, 1, or 2)

    // State flags for EI/DI
    bool EI_PENDING;       // EI instruction executed
    bool EI_JUST_RESOLVED; // EI just completed (one instruction delay)
    bool UNRESOLVED_PREFIX; // DD or FD prefix not resolved

    // Memory pointer (used for certain instructions)
    uint16_t MEMPTR;

    // Q register for flag tracking
    uint8_t Q;
    uint8_t LAST_Q;

    // ============================================================
    // 16-bit register accessors
    // ============================================================
    uint16_t BC() const { return (B << 8) | C; }
    void set_BC(uint16_t v) { B = (v >> 8) & 0xFF; C = v & 0xFF; }

    uint16_t DE() const { return (D << 8) | E; }
    void set_DE(uint16_t v) { D = (v >> 8) & 0xFF; E = v & 0xFF; }

    uint16_t HL() const { return (H << 8) | L; }
    void set_HL(uint16_t v) { H = (v >> 8) & 0xFF; L = v & 0xFF; }

    uint16_t AF() const { return (A << 8) | F; }
    void set_AF(uint16_t v) { A = (v >> 8) & 0xFF; F = v & 0xFF; }

    // ============================================================
    // Index register byte accessors
    // ============================================================
    uint8_t IXh() const { return (IX >> 8) & 0xFF; }
    void set_IXh(uint8_t v) { IX = (IX & 0x00FF) | (uint16_t(v) << 8); }

    uint8_t IXl() const { return IX & 0xFF; }
    void set_IXl(uint8_t v) { IX = (IX & 0xFF00) | v; }

    uint8_t IYh() const { return (IY >> 8) & 0xFF; }
    void set_IYh(uint8_t v) { IY = (IY & 0x00FF) | (uint16_t(v) << 8); }

    uint8_t IYl() const { return IY & 0xFF; }
    void set_IYl(uint8_t v) { IY = (IY & 0xFF00) | v; }

    // ============================================================
    // Shadow register operations
    // ============================================================
    void swap_af() {
        uint8_t temp_a = A; uint8_t temp_f = F;
        A = Ap; F = Fp;
        Ap = temp_a; Fp = temp_f;
    }

    void swap_de_hl() {
        uint16_t temp_de = DE();
        uint16_t temp_hl = HL();
        set_DE(temp_hl);
        set_HL(temp_de);
    }

    void swap_all() {
        swap_af();
        swap_de_hl();
        // Swap BC shadow
        uint8_t temp_b = B; uint8_t temp_c = C;
        B = Bp; C = Cp;
        Bp = temp_b; Cp = temp_c;
    }

    // ============================================================
    // Reset state
    // ============================================================
    void reset() {
        A = F = B = C = D = E = H = L = 0;
        Ap = Fp = Bp = Cp = Dp = Ep = Hp = Lp = 0;
        IX = IY = SP = PC = 0;
        I = R = 0;
        IFF1 = IFF2 = false;
        IM = 0;
        EI_PENDING = EI_JUST_RESOLVED = UNRESOLVED_PREFIX = false;
        MEMPTR = 0;
        Q = LAST_Q = 0;
    }
};

} // namespace z80