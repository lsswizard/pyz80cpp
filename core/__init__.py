"""
Z80 Core Module - C++ accelerated version

This package provides a cycle-exact Z80 CPU emulator implemented in C++
with a thin Python wrapper via nanobind for maximum performance.
"""

try:
    from ._pyz80 import Z80CPU, SimpleBus, Registers
    from ._pyz80 import FLAG_S, FLAG_Z, FLAG_H, FLAG_PV, FLAG_N, FLAG_C
    from ._pyz80 import PARITY_TABLE, ADD_FLAGS, SUB_FLAGS, SBC_FLAGS, ADC_FLAGS
except ImportError:
    # Fallback to Python implementation if C++ extension not built
    import sys
    import os

    sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "pyz80_python"))
    from pyz80_python import Z80CPU, SimpleBus, Registers
    from pyz80_python import FLAG_S, FLAG_Z, FLAG_H, FLAG_PV, FLAG_N, FLAG_C
    from pyz80_python import PARITY_TABLE, ADD_FLAGS, SUB_FLAGS, SBC_FLAGS, ADC_FLAGS

__all__ = [
    "Z80CPU",
    "Registers",
    "SimpleBus",
    "FLAG_S",
    "FLAG_Z",
    "FLAG_H",
    "FLAG_PV",
    "FLAG_N",
    "FLAG_C",
    "PARITY_TABLE",
    "ADD_FLAGS",
    "SUB_FLAGS",
    "SBC_FLAGS",
    "ADC_FLAGS",
]
