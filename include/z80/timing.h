#pragma once

#include <cstdint>

namespace z80 {

// ============================================================
// Timing utilities and constants
// ============================================================

namespace TimingConstants {
    // Clock frequencies (Hz)
    constexpr int ZX_SPECTRUM_48K_CLOCK = 3500000;      // 3.5 MHz
    constexpr int ZX_SPECTRUM_128K_CLOCK = 3546900;     // 3.5469 MHz
    constexpr int ZX_SPECTRUM_PLUS3_CLOCK = 3546900;
    constexpr int AMSTRAD_CPC_CLOCK = 4000000;          // 4.0 MHz
    constexpr int MSX_CLOCK = 3579545;                  // 3.5795 MHz

    // T-states per frame (50Hz)
    constexpr int ZX_SPECTRUM_TSTATES_PER_FRAME = 69888;
    constexpr int AMSTRAD_CPC_TSTATES_PER_FRAME = 80000;
}

// ============================================================
// Contention model base class
// ============================================================

class ContentionModel {
public:
    virtual ~ContentionModel() = default;

    // Calculate additional wait states for an address at a given T-state
    virtual int get_contention(int t_state, uint16_t addr) {
        (void)t_state;
        (void)addr;
        return 0;  // Default: no contention
    }

    // Check if address is in contended memory region
    virtual bool is_contended(uint16_t addr) {
        (void)addr;
        return false;
    }
};

} // namespace z80