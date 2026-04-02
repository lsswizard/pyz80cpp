#pragma once

#include "handlers.h"

class Decoder {
    DecodeSlot _cache[65536];
public:
    Decoder();
    const DecodeSlot& decode(uint8_t* mem, uint16_t addr);
    // Decode from opcode bytes (for buses without direct memory access)
    DecodeSlot decode_from_bytes(uint8_t opcode, uint8_t b1, uint8_t b2, uint8_t b3);
    void invalidate(uint16_t addr);
    void invalidate_all();
};