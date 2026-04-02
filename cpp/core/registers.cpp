#include "registers.h"

void Registers::swap_shadow() {
    uint8_t t;
    t = A; A = Ap; Ap = t;
    t = F; F = Fp; Fp = t;
}

void Registers::swap_shadow_all() {
    uint8_t t;
    t = B; B = Bp; Bp = t;
    t = C; C = Cp; Cp = t;
    t = D; D = Dp; Dp = t;
    t = E; E = Ep; Ep = t;
    t = H; H = Hp; Hp = t;
    t = L; L = Lp; Lp = t;
}

void Registers::reset() {
    A = 0xFF; F = 0xFF;
    B = C = 0;
    D = E = 0;
    H = L = 0;
    Ap = Fp = Bp = Cp = 0;
    Dp = Ep = Hp = Lp = 0;
    IX = 0xFFFF;
    IY = 0xFFFF;
    SP = 0xFFFF;
    PC = 0;
    I = 0x00;
    R = 0;
    IFF1 = IFF2 = false;
    IM = 0;
    EI_PENDING = false;
    EI_JUST_RESOLVED = false;
    UNRESOLVED_PREFIX = false;
    MEMPTR = 0;
    Q = 0;
    LAST_Q = 0;
}

uint16_t Registers::get_reg16(int pair) const {
    switch (pair) {
        case 0: return BC();
        case 1: return DE();
        case 2: return HL();
        default: return SP;
    }
}

void Registers::set_reg16(int pair, uint16_t value) {
    value &= 0xFFFF;
    switch (pair) {
        case 0: set_BC(value); break;
        case 1: set_DE(value); break;
        case 2: set_HL(value); break;
        default: SP = value; break;
    }
}

uint16_t Registers::get_reg16_push(int pair) const {
    switch (pair) {
        case 0: return BC();
        case 1: return DE();
        case 2: return HL();
        default: return AF();
    }
}

void Registers::set_reg16_push(int pair, uint16_t value) {
    value &= 0xFFFF;
    switch (pair) {
        case 0: set_BC(value); break;
        case 1: set_DE(value); break;
        case 2: set_HL(value); break;
        default: set_AF(value); break;
    }
}
