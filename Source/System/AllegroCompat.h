/**
 * AllegroCompat.h
 *
 * Drop-in Allegro 4 compatibility layer for Emscripten / WebAssembly builds.
 *
 * Replaces the Allegro 4 software rasterizer with plain C++ over typed arrays:
 *   - BITMAP is backed by std::vector<uint8_t> (no OS allocator surprises)
 *   - blit/masked_blit/stretch_blit are pure C inner loops that JIT cleanly
 *   - Color tables (blenders) are 256×256 uint8 LUTs matching Allegro's layout
 *   - The 8bpp indexed → RGBA expansion is done on the JS side via typed arrays
 *     (see emscripten/shell.html ccBlit8ToCanvas) for near-zero-copy perf
 *
 * Only Allegro 4 APIs actually called by CCCP are implemented.  Missing
 * functions produce a static_assert / linker error to catch new usage early.
 *
 * Included from allegro.h when __EMSCRIPTEN__ is defined — the game source
 * doesn't need any #ifdef guards around individual Allegro calls.
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <cmath>
#include <cassert>
#include <vector>
#include <algorithm>
#include <memory>

// ---------------------------------------------------------------------------
// Allegro-compatible type aliases
// ---------------------------------------------------------------------------

typedef uint8_t  PALETTE[256][4]; // [index][R, G, B, reserved]

// Fixed-point type (16.16 format), matching Allegro's fixed
typedef int32_t fixed;

// Colour table (blender lookup) — 256×256 index → index
struct COLOR_MAP {
    uint8_t data[256][256];
};

// ---------------------------------------------------------------------------
// BITMAP — the central Allegro raster surface
// ---------------------------------------------------------------------------

struct BITMAP {
    int   w;           //!< Width in pixels
    int   h;           //!< Height in pixels
    int   clip;        //!< Clipping enabled flag
    int   cl, cr;      //!< Clip left / right (exclusive)
    int   ct, cb;      //!< Clip top  / bottom (exclusive)
    int   depth;       //!< Colour depth: 8, 24, or 32

    // Pixel data — owned by this bitmap
    std::vector<uint8_t> pixels;

    // Row-pointer array for `line[y][x]` compatibility with legacy Allegro code
    std::vector<uint8_t*> line_ptrs;

    // Convenience accessor to the Allegro-style `line` member.
    // Code that does `bmp->line[y][x]` will work through this.
    uint8_t** line;

    // Bytes per pixel (1, 3, or 4)
    int bpp() const { return (depth <= 8) ? 1 : (depth <= 24) ? 3 : 4; }
    // Bytes per row (stride)
    int stride() const { return w * bpp(); }

    // Rebuild line pointer array (call after resize)
    void rebuildLinePtrs() {
        line_ptrs.resize(h);
        uint8_t* row = pixels.data();
        int s = stride();
        for (int y = 0; y < h; ++y, row += s) {
            line_ptrs[y] = row;
        }
        line = line_ptrs.data();
    }

    BITMAP() : w(0), h(0), clip(0), cl(0), cr(0), ct(0), cb(0), depth(8), line(nullptr) {}
};

// ---------------------------------------------------------------------------
// Globals that mirror Allegro's global state
// ---------------------------------------------------------------------------

extern PALETTE                  g_AllegroCurrentPalette;  //!< Active palette (set by set_palette)
extern PALETTE                  g_AllegroDefaultPalette;  //!< Default/loaded palette
extern COLOR_MAP*               color_map;                 //!< Active blending color table

// ---------------------------------------------------------------------------
// Colour packing/unpacking helpers
// ---------------------------------------------------------------------------

// 32bpp RGBA packing (used with depth-32 bitmaps and GUI code)
inline int makecol32(int r, int g, int b) {
    return (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | (0xFF << 24);
}
inline int makeacol32(int r, int g, int b, int a) {
    return (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}
inline int getr32(int c) { return  c        & 0xFF; }
inline int getg32(int c) { return (c >>  8) & 0xFF; }
inline int getb32(int c) { return (c >> 16) & 0xFF; }
inline int geta32(int c) { return (c >> 24) & 0xFF; }

// 8bpp nearest-match palette lookup (linear search — only used at init/setup)
inline int makecol8(int r, int g, int b) {
    int best = 0, bestDist = 0x7FFFFFFF;
    for (int i = 0; i < 256; ++i) {
        int dr = (int)g_AllegroCurrentPalette[i][0] - r;
        int dg = (int)g_AllegroCurrentPalette[i][1] - g;
        int db = (int)g_AllegroCurrentPalette[i][2] - b;
        int d = dr*dr + dg*dg + db*db;
        if (d < bestDist) { bestDist = d; best = i; }
    }
    return best;
}

// Palette entry helpers (used in PostProcessMan to build the GL palette texture)
inline int getr8(int index) { return g_AllegroCurrentPalette[index & 0xFF][0]; }
inline int getg8(int index) { return g_AllegroCurrentPalette[index & 0xFF][1]; }
inline int getb8(int index) { return g_AllegroCurrentPalette[index & 0xFF][2]; }

// ---------------------------------------------------------------------------
// BITMAP creation / destruction
// ---------------------------------------------------------------------------

/// Create a bitmap of the given colour depth (8, 24, or 32).
inline BITMAP* create_bitmap_ex(int depth, int w, int h) {
    BITMAP* bmp = new BITMAP();
    bmp->w     = w;
    bmp->h     = h;
    bmp->depth = depth;
    bmp->clip  = 1;
    bmp->cl    = 0;
    bmp->cr    = w;
    bmp->ct    = 0;
    bmp->cb    = h;
    int bpp = (depth <= 8) ? 1 : (depth <= 24) ? 3 : 4;
    bmp->pixels.assign(w * h * bpp, 0);
    bmp->rebuildLinePtrs();
    return bmp;
}

/// Create an 8bpp bitmap.
inline BITMAP* create_bitmap(int w, int h) { return create_bitmap_ex(8, w, h); }

inline void destroy_bitmap(BITMAP* bmp) { delete bmp; }

// ---------------------------------------------------------------------------
// Clip rectangle management
// ---------------------------------------------------------------------------

inline void set_clip_state(BITMAP* bmp, int state) {
    bmp->clip = state;
}
inline void set_clip_rect(BITMAP* bmp, int x1, int y1, int x2, int y2) {
    bmp->cl = std::max(0, x1);
    bmp->ct = std::max(0, y1);
    bmp->cr = std::min(bmp->w, x2 + 1);
    bmp->cb = std::min(bmp->h, y2 + 1);
    bmp->clip = 1;
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------

inline void clear_to_color(BITMAP* bmp, int color) {
    if (!bmp) return;
    int bpp = bmp->bpp();
    if (bpp == 1) {
        std::memset(bmp->pixels.data(), color & 0xFF, bmp->pixels.size());
    } else if (bpp == 4) {
        uint32_t* dst = reinterpret_cast<uint32_t*>(bmp->pixels.data());
        std::fill(dst, dst + bmp->w * bmp->h, static_cast<uint32_t>(color));
    } else {
        // 24bpp — per-pixel
        uint8_t r = color & 0xFF, g = (color >> 8) & 0xFF, b = (color >> 16) & 0xFF;
        uint8_t* p = bmp->pixels.data();
        for (int i = 0, n = bmp->w * bmp->h; i < n; ++i, p += 3) {
            p[0] = r; p[1] = g; p[2] = b;
        }
    }
}

// ---------------------------------------------------------------------------
// Palette
// ---------------------------------------------------------------------------

inline void set_palette(const PALETTE pal) {
    std::memcpy(g_AllegroCurrentPalette, pal, sizeof(PALETTE));
}
inline void get_palette(PALETTE pal) {
    std::memcpy(pal, g_AllegroCurrentPalette, sizeof(PALETTE));
}

// ---------------------------------------------------------------------------
// Pixel accessors
// ---------------------------------------------------------------------------

inline void putpixel(BITMAP* bmp, int x, int y, int color) {
    if (!bmp || x < 0 || y < 0 || x >= bmp->w || y >= bmp->h) return;
    int bpp = bmp->bpp();
    uint8_t* p = bmp->pixels.data() + (y * bmp->w + x) * bpp;
    if (bpp == 1) {
        *p = color & 0xFF;
    } else if (bpp == 4) {
        *reinterpret_cast<uint32_t*>(p) = static_cast<uint32_t>(color);
    } else {
        p[0] = color & 0xFF; p[1] = (color >> 8) & 0xFF; p[2] = (color >> 16) & 0xFF;
    }
}

inline int getpixel(const BITMAP* bmp, int x, int y) {
    if (!bmp || x < 0 || y < 0 || x >= bmp->w || y >= bmp->h) return -1;
    int bpp = bmp->bpp();
    const uint8_t* p = bmp->pixels.data() + (y * bmp->w + x) * bpp;
    if (bpp == 1) return *p;
    if (bpp == 4) return static_cast<int>(*reinterpret_cast<const uint32_t*>(p));
    return p[0] | (p[1] << 8) | (p[2] << 16);
}

// ---------------------------------------------------------------------------
// Blit — basic rectangle copy (no colour key)
// ---------------------------------------------------------------------------

void blit(const BITMAP* src, BITMAP* dst,
          int src_x, int src_y,
          int dst_x, int dst_y,
          int w, int h);

// ---------------------------------------------------------------------------
// Masked blit — skips pixels whose index == 0 (palette index 0 = transparent)
// For 32bpp: skips fully-transparent pixels (alpha == 0).
// ---------------------------------------------------------------------------

void masked_blit(const BITMAP* src, BITMAP* dst,
                 int src_x, int src_y,
                 int dst_x, int dst_y,
                 int w, int h);

// ---------------------------------------------------------------------------
// Stretch blit (nearest-neighbour)
// ---------------------------------------------------------------------------

void stretch_blit(const BITMAP* src, BITMAP* dst,
                  int sx, int sy, int sw, int sh,
                  int dx, int dy, int dw, int dh);

void masked_stretch_blit(const BITMAP* src, BITMAP* dst,
                         int sx, int sy, int sw, int sh,
                         int dx, int dy, int dw, int dh);

// ---------------------------------------------------------------------------
// draw_sprite helpers
// ---------------------------------------------------------------------------

inline void draw_sprite(BITMAP* dst, const BITMAP* src, int x, int y) {
    masked_blit(src, dst, 0, 0, x, y, src->w, src->h);
}

void draw_sprite_v_flip(BITMAP* dst, const BITMAP* src, int x, int y);
void draw_sprite_h_flip(BITMAP* dst, const BITMAP* src, int x, int y);

// ---------------------------------------------------------------------------
// rotate_sprite — uses Allegro's 16.16 fixed-point angle (0x100000 = full turn)
// ---------------------------------------------------------------------------

void rotate_sprite(BITMAP* dst, const BITMAP* src, int x, int y, fixed angle);
void pivot_sprite(BITMAP* dst, const BITMAP* src, int x, int y, int cx, int cy, fixed angle);

// ---------------------------------------------------------------------------
// Fixed-point helpers (16.16 format)
// ---------------------------------------------------------------------------

inline fixed ftofix(double x) { return static_cast<fixed>(x * 65536.0 + 0.5); }
inline fixed itofix(int x)    { return x << 16; }
inline double fixtof(fixed x) { return static_cast<double>(x) / 65536.0; }
inline int fixtoi(fixed x)    { return x >> 16; }

// ---------------------------------------------------------------------------
// Blending / color tables
// ---------------------------------------------------------------------------

// Allegro blender mode constants (drawing_mode)
#define DRAW_MODE_SOLID       0
#define DRAW_MODE_TRANS       1
#define DRAW_MODE_XOR         2
#define DRAW_MODE_COPY_PATTERN 3

inline void drawing_mode(int mode, BITMAP*, int, int) {
    (void)mode; // Only DRAW_MODE_TRANS triggers color_map usage; handled in callers.
}

// Blender setter functions — set the internal blending function used when
// create_blender_table is called.  The actual blending is applied when
// building the COLOR_MAP LUT, so these only need to capture the mode.
void set_burn_blender(int r, int g, int b, int a);
void set_color_blender(int r, int g, int b, int a);
void set_difference_blender(int r, int g, int b, int a);
void set_dissolve_blender(int r, int g, int b, int a);
void set_dodge_blender(int r, int g, int b, int a);
void set_invert_blender(int r, int g, int b, int a);
void set_luminance_blender(int r, int g, int b, int a);
void set_multiply_blender(int r, int g, int b, int a);
void set_saturation_blender(int r, int g, int b, int a);
void set_screen_blender(int r, int g, int b, int a);
void set_alpha_blender();
void set_trans_blender(int r, int g, int b, int a);

// set_blender_mode_ex — used by TrueAlphaBlender
typedef unsigned long (*BLENDER_FUNC)(unsigned long, unsigned long, unsigned long);
inline void set_blender_mode_ex(BLENDER_FUNC b15, BLENDER_FUNC b16, BLENDER_FUNC b24,
                                BLENDER_FUNC b32, BLENDER_FUNC b15x, BLENDER_FUNC b16x,
                                BLENDER_FUNC b24x, int r, int g, int b, int a) {
    (void)b15; (void)b16; (void)b24; (void)b32; (void)b15x; (void)b16x; (void)b24x;
    (void)r; (void)g; (void)b; (void)a;
    // Stub: the web build uses the GL Dissolve shader for post-process effects.
}

// Build a COLOR_MAP from the current blender and palette.
void create_blender_table(COLOR_MAP* table, const PALETTE pal, void*);
void create_color_table(COLOR_MAP* table, const PALETTE pal,
                        int (*blend)(PALETTE, int, int), void*);
void create_trans_table(COLOR_MAP* table, const PALETTE pal, int r, int g, int b, void*);

// ---------------------------------------------------------------------------
// Allegro init / shutdown (no-op on web — SDL handles the window)
// ---------------------------------------------------------------------------

#define SYSTEM_NONE 0
inline int install_allegro(int, int*, int(*)(void(*)())) { return 0; }
inline void allegro_exit() {}

// loadpng init (no-op — we use libpng directly in ContentFile)
inline void loadpng_init() {}

// save_png forward declaration — implemented in FrameMan's save path
int save_png(const char* filename, BITMAP* bmp, const PALETTE pal);

// ---------------------------------------------------------------------------
// Colour conversion constants
// ---------------------------------------------------------------------------

#define COLORCONV_8_TO_32    1
#define COLORCONV_NONE       0
#define COLORCONV_EXPAND_HI_TO_TRUE 0
