"""
Descent 3 Graphics Definitions

ddgr_color type, GR_RGB, GR_COLOR_*, color constants, 16-bit conversions.
Ported from lib/grdefs.h.
"""

from __future__ import annotations

# ── Color type ──
# ddgr_color is uint32_t: 0x00RRGGBB (top byte unused)
ddgr_color = int

# ── Color constants ──
GR_NULL        = 0xFFFFFFFF
GR_BLACK       = 0x00000000
GR_GREEN       = 0x0000FF00
GR_RED         = 0x00FF0000
GR_BLUE        = 0x000000FF
GR_DARKGRAY    = 0x00404040
GR_LIGHTGRAY   = 0x00C0C0C0
GR_WHITE       = 0x00FFFFFF

# ── Bit depths ──
BPP_DEFAULT = 0
BPP_8       = 8
BPP_15      = 15
BPP_16      = 16
BPP_24      = 24
BPP_32      = 32

# ── Opaque flag for 16-bit textures ──
OPAQUE_FLAG16 = 0x8000
OPAQUE_FLAG   = OPAQUE_FLAG16

# ── Transparent colors ──
TRANSPARENT_COLOR32  = 0x0000FF00
NEW_TRANSPARENT_COLOR = 0x0000

# ── Fixed screen dimensions ──
FIXED_SCREEN_WIDTH  = 640
FIXED_SCREEN_HEIGHT = 480


def GR_RGB(r: int, g: int, b: int) -> ddgr_color:
    """Pack 8-bit RGB into ddgr_color (0x00RRGGBB)."""
    return (r << 16) | (g << 8) | b


def GR_RGB16(r: int, g: int, b: int) -> int:
    """Pack 8-bit RGB into 16-bit 555 format."""
    return ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3)


def GR_COLOR_RED(c: ddgr_color) -> int:
    """Extract red channel from ddgr_color."""
    return (c & 0x00FF0000) >> 16


def GR_COLOR_GREEN(c: ddgr_color) -> int:
    """Extract green channel from ddgr_color."""
    return (c & 0x0000FF00) >> 8


def GR_COLOR_BLUE(c: ddgr_color) -> int:
    """Extract blue channel from ddgr_color."""
    return c & 0x000000FF


def GR_COLOR_TO_16(c: ddgr_color) -> int:
    """Convert ddgr_color to 16-bit 555 format."""
    r = ((c & 0x00FF0000) >> 16)
    g = ((c & 0x0000FF00) >> 8)
    b = (c & 0x000000FF)
    return ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3)


def GR_16_TO_COLOR(col: int) -> ddgr_color:
    """Convert 16-bit 555 to ddgr_color."""
    r = (col & 0x7C00) >> 7
    g = (col & 0x03E0) >> 2
    b = (col & 0x001F) << 3
    return GR_RGB(r, g, b)


def BPP_TO_BYTESPP(bpp: int) -> int:
    """Convert bits-per-pixel to bytes-per-pixel."""
    return (bpp + 7) >> 3
