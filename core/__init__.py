"""
Z80 Core Module - C++ accelerated version

This package provides a cycle-exact Z80 CPU emulator implemented in C++
with a thin Python wrapper via raw CPython API for maximum performance.

The core is machine-agnostic — implement machines in Python by providing
I/O callbacks and bus logic.
"""

try:
    from ._pyz80 import Z80CPU
    from ._pyz80 import FLAG_S, FLAG_Z, FLAG_H, FLAG_PV, FLAG_N, FLAG_C
    from ._pyz80 import FLAG_F5, FLAG_F3
    from ._pyz80 import MACHINE_STATE_SIZE
except ImportError:
    import sys
    import os

    sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "pyz80_python"))
    from pyz80_python import Z80CPU, SimpleBus
    from pyz80_python import FLAG_S, FLAG_Z, FLAG_H, FLAG_PV, FLAG_N, FLAG_C
    from pyz80_python import FLAG_F5, FLAG_F3

    MACHINE_STATE_SIZE = 0

__all__ = [
    "Z80CPU",
    "FLAG_S",
    "FLAG_Z",
    "FLAG_F5",
    "FLAG_H",
    "FLAG_F3",
    "FLAG_PV",
    "FLAG_N",
    "FLAG_C",
    "MACHINE_STATE_SIZE",
]
