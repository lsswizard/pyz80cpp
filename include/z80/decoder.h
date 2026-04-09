#pragma once

#include "z80.h"
#include "opcode_table.h"
#include <array>

namespace z80 {

// ============================================================
// Decoder - handles opcode prefix chains
// ============================================================

class Decoder {
public:
    Decoder();

    // Decode instruction at current PC
    // Returns reference to instruction descriptor
    const Instruction& decode(uint8_t opcode);

    // Get current prefix state
    PrefixState get_prefix_state() const { return state; }

    // Reset decoder state
    void reset() { state = PrefixState::NONE; }

    // Invalidate instruction cache (when PC changes)
    void invalidate_cache(uint16_t addr);

    // Full cache invalidation
    void invalidate_all();

private:
    PrefixState state;

    // Handle prefix opcodes
    bool is_prefix(uint8_t opcode) const;
    PrefixState get_prefix_for_opcode(uint8_t opcode) const;
};

// ============================================================
// Decoder with instruction caching
// ============================================================

class CachedDecoder : public Decoder {
public:
    CachedDecoder();

    // Decode with caching
    const Instruction& decode_cached(uint16_t pc, uint8_t* memory);

    // Invalidate cache entry
    void invalidate(uint16_t pc);

    // Clear all cached entries
    void clear_cache();

private:
    struct CacheEntry {
        OpHandler handler = nullptr;
        uint8_t cycles = 4;
        uint8_t length = 1;
        bool valid = false;
    };

    std::array<CacheEntry, 256> cache;
};

} // namespace z80