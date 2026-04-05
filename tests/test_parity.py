#!/usr/bin/env python3
"""Parity table tests."""

import pytest
from conftest import _parity


class TestParityTable:
    @pytest.mark.parametrize(
        "val,expected",
        [
            (0x00, True),
            (0x01, False),
            (0x03, True),
            (0xFF, True),
            (0xAA, True),
            (0x55, True),
            (0x80, False),
            (0xC0, True),
        ],
    )
    def test_parity(self, val, expected):
        assert _parity(val) == expected
