#pragma once

#include <cstdint>
#include <cstring>

struct Registers {
    uint8_t  A, F, B, C, D, E, H, L;
    uint8_t  Ap, Fp, Bp, Cp, Dp, Ep, Hp, Lp;
    uint16_t IX, IY, SP, PC;
    uint8_t  I, R;
    bool     IFF1, IFF2;
    uint8_t  IM;
    bool     EI_PENDING, EI_JUST_RESOLVED, UNRESOLVED_PREFIX;
    uint16_t MEMPTR;
    uint8_t  Q, LAST_Q;

    uint16_t BC() const { return (uint16_t)((B << 8) | C); }
    void set_BC(uint16_t v) { B = (v >> 8) & 0xFF; C = v & 0xFF; }
    uint16_t DE() const { return (uint16_t)((D << 8) | E); }
    void set_DE(uint16_t v) { D = (v >> 8) & 0xFF; E = v & 0xFF; }
    uint16_t HL() const { return (uint16_t)((H << 8) | L); }
    void set_HL(uint16_t v) { H = (v >> 8) & 0xFF; L = v & 0xFF; }
    uint16_t AF() const { return (uint16_t)((A << 8) | F); }
    void set_AF(uint16_t v) { A = (v >> 8) & 0xFF; F = v & 0xFF; }

    uint8_t IXh() const { return (IX >> 8) & 0xFF; }
    void set_IXh(uint8_t v) { IX = (IX & 0x00FF) | ((uint16_t)v << 8); }
    uint8_t IYh() const { return (IY >> 8) & 0xFF; }
    void set_IYh(uint8_t v) { IY = (IY & 0x00FF) | ((uint16_t)v << 8); }
    uint8_t IXl() const { return IX & 0xFF; }
    void set_IXl(uint8_t v) { IX = (IX & 0xFF00) | v; }
    uint8_t IYl() const { return IY & 0xFF; }
    void set_IYl(uint8_t v) { IY = (IY & 0xFF00) | v; }

    void swap_shadow();
    void swap_shadow_all();
    void reset();

    uint16_t get_reg16(int pair) const;
    void set_reg16(int pair, uint16_t value);
    uint16_t get_reg16_push(int pair) const;
    void set_reg16_push(int pair, uint16_t value);
};
