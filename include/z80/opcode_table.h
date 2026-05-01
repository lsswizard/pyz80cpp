#pragma once

#include "z80.h"
#include <array>

namespace z80 {

// ============================================================
// OpcodeTable — static storage for all instruction tables
// ============================================================
class OpcodeTable {
public:
    static std::array<Instruction, 256> main_table;
    static std::array<Instruction, 256> cb_table;
    static std::array<Instruction, 256> ed_table;
    static std::array<Instruction, 256> dd_table;
    static std::array<Instruction, 256> fd_table;
    static std::array<Instruction, 256> ddcb_table;
    static std::array<Instruction, 256> fdcb_table;

    static void init();

    static const Instruction& get_main(uint8_t op) { return main_table[op]; }
    static const Instruction& get_cb  (uint8_t op) { return cb_table[op];   }
    static const Instruction& get_ed  (uint8_t op) { return ed_table[op];   }
    static const Instruction& get_dd  (uint8_t op) { return dd_table[op];   }
    static const Instruction& get_fd  (uint8_t op) { return fd_table[op];   }
    static const Instruction& get_ddcb(uint8_t op) { return ddcb_table[op]; }
    static const Instruction& get_fdcb(uint8_t op) { return fdcb_table[op]; }
};

} // namespace z80
