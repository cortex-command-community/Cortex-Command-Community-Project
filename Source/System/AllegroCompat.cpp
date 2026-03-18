/**
 * AllegroCompat.cpp
 *
 * Implementation of the Allegro 4 compatibility shim.
 * See AllegroCompat.h for the full API surface.
 */

#include "AllegroCompat.h"
#include <cmath>
#include <algorithm>

// ---------------------------------------------------------------------------
// Global state
// ---------------------------------------------------------------------------

PALETTE      g_AllegroCurrentPalette;
PALETTE      g_AllegroDefaultPalette;
COLOR_MAP*   color_map = nullptr;

// Current blend mode state (set by set_*_blender calls, consumed by create_blender_table)
static int  s_BlendR = 128, s_BlendG = 128, s_BlendB = 128, s_BlendA = 128;

enum class BlendMode {
    Screen, Burn, Color, Difference, Dissolve, Dodge,
    Invert, Luminance, Multiply, Saturation, Trans, None
};
static BlendMode s_CurrentBlendMode = BlendMode::Screen;

// ---------------------------------------------------------------------------
// Blit — plain rectangle copy with clip
// ---------------------------------------------------------------------------

void blit(const BITMAP* src, BITMAP* dst,
          int src_x, int src_y,
          int dst_x, int dst_y,
          int w, int h) {
    if (!src || !dst || w <= 0 || h <= 0) return;
    if (src->depth != dst->depth) return; // depth mismatch — caller error

    int bpp = src->bpp();

    // Clip to source bounds
    if (src_x < 0) { dst_x -= src_x; w += src_x; src_x = 0; }
    if (src_y < 0) { dst_y -= src_y; h += src_y; src_y = 0; }
    if (src_x + w > src->w) w = src->w - src_x;
    if (src_y + h > src->h) h = src->h - src_y;

    // Clip to destination bounds
    if (dst->clip) {
        if (dst_x < dst->cl) { int d = dst->cl - dst_x; src_x += d; w -= d; dst_x = dst->cl; }
        if (dst_y < dst->ct) { int d = dst->ct - dst_y; src_y += d; h -= d; dst_y = dst->ct; }
        if (dst_x + w > dst->cr) w = dst->cr - dst_x;
        if (dst_y + h > dst->cb) h = dst->cb - dst_y;
    }

    if (w <= 0 || h <= 0) return;

    int rowBytes = w * bpp;
    for (int row = 0; row < h; ++row) {
        const uint8_t* s = src->pixels.data() + ((src_y + row) * src->w + src_x) * bpp;
        uint8_t*       d = dst->pixels.data() + ((dst_y + row) * dst->w + dst_x) * bpp;
        std::memcpy(d, s, rowBytes);
    }
}

// ---------------------------------------------------------------------------
// Masked blit — colour-key transparency
// ---------------------------------------------------------------------------

void masked_blit(const BITMAP* src, BITMAP* dst,
                 int src_x, int src_y,
                 int dst_x, int dst_y,
                 int w, int h) {
    if (!src || !dst || w <= 0 || h <= 0) return;

    // Clip
    if (src_x < 0) { dst_x -= src_x; w += src_x; src_x = 0; }
    if (src_y < 0) { dst_y -= src_y; h += src_y; src_y = 0; }
    if (src_x + w > src->w) w = src->w - src_x;
    if (src_y + h > src->h) h = src->h - src_y;
    if (dst->clip) {
        if (dst_x < dst->cl) { int d = dst->cl - dst_x; src_x += d; w -= d; dst_x = dst->cl; }
        if (dst_y < dst->ct) { int d = dst->ct - dst_y; src_y += d; h -= d; dst_y = dst->ct; }
        if (dst_x + w > dst->cr) w = dst->cr - dst_x;
        if (dst_y + h > dst->cb) h = dst->cb - dst_y;
    }
    if (w <= 0 || h <= 0) return;

    int bpp = src->bpp();

    if (src->depth == 8 && dst->depth == 8) {
        // 8bpp: skip palette index 0 (Allegro colour key)
        for (int row = 0; row < h; ++row) {
            const uint8_t* s = src->pixels.data() + (src_y + row) * src->w + src_x;
            uint8_t*       d = dst->pixels.data() + (dst_y + row) * dst->w + dst_x;
            for (int col = 0; col < w; ++col) {
                uint8_t px = s[col];
                if (px != 0) d[col] = px;
            }
        }
    } else if (bpp == 4) {
        // 32bpp: skip pixels with zero alpha
        for (int row = 0; row < h; ++row) {
            const uint32_t* s = reinterpret_cast<const uint32_t*>(src->pixels.data()) +
                                (src_y + row) * src->w + src_x;
            uint32_t*       d = reinterpret_cast<uint32_t*>(dst->pixels.data()) +
                                (dst_y + row) * dst->w + dst_x;
            for (int col = 0; col < w; ++col) {
                uint32_t px = s[col];
                if ((px >> 24) != 0) d[col] = px;
            }
        }
    } else {
        // Fallback: treat as plain blit
        blit(src, dst, src_x, src_y, dst_x, dst_y, w, h);
    }
}

// ---------------------------------------------------------------------------
// Stretch blit (nearest-neighbour)
// ---------------------------------------------------------------------------

static void stretch_blit_core(const BITMAP* src, BITMAP* dst,
                               int sx, int sy, int sw, int sh,
                               int dx, int dy, int dw, int dh,
                               bool masked) {
    if (!src || !dst || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0) return;

    int bpp = src->bpp();
    // Clip destination
    int x0 = dx, y0 = dy, x1 = dx + dw, y1 = dy + dh;
    if (dst->clip) {
        x0 = std::max(x0, dst->cl); y0 = std::max(y0, dst->ct);
        x1 = std::min(x1, dst->cr); y1 = std::min(y1, dst->cb);
    }
    if (x0 >= x1 || y0 >= y1) return;

    for (int yd = y0; yd < y1; ++yd) {
        int ys = sy + (yd - dy) * sh / dh;
        if (ys < 0 || ys >= src->h) continue;
        for (int xd = x0; xd < x1; ++xd) {
            int xs = sx + (xd - dx) * sw / dw;
            if (xs < 0 || xs >= src->w) continue;
            const uint8_t* sp = src->pixels.data() + (ys * src->w + xs) * bpp;
            uint8_t*       dp = dst->pixels.data() + (yd * dst->w + xd) * bpp;
            if (masked) {
                if (bpp == 1 && sp[0] == 0) continue;
                if (bpp == 4 && sp[3] == 0) continue;
            }
            std::memcpy(dp, sp, bpp);
        }
    }
}

void stretch_blit(const BITMAP* src, BITMAP* dst,
                  int sx, int sy, int sw, int sh,
                  int dx, int dy, int dw, int dh) {
    stretch_blit_core(src, dst, sx, sy, sw, sh, dx, dy, dw, dh, false);
}

void masked_stretch_blit(const BITMAP* src, BITMAP* dst,
                         int sx, int sy, int sw, int sh,
                         int dx, int dy, int dw, int dh) {
    stretch_blit_core(src, dst, sx, sy, sw, sh, dx, dy, dw, dh, true);
}

// ---------------------------------------------------------------------------
// Sprite flip helpers
// ---------------------------------------------------------------------------

void draw_sprite_v_flip(BITMAP* dst, const BITMAP* src, int x, int y) {
    if (!src || !dst) return;
    int bpp = src->bpp();
    for (int row = 0; row < src->h; ++row) {
        int sy = src->h - 1 - row;
        int dy = y + row;
        for (int col = 0; col < src->w; ++col) {
            int dx = x + col;
            if (dx < 0 || dy < 0 || dx >= dst->w || dy >= dst->h) continue;
            const uint8_t* sp = src->pixels.data() + (sy * src->w + col) * bpp;
            if (bpp == 1 && sp[0] == 0) continue;
            if (bpp == 4 && sp[3] == 0) continue;
            uint8_t* dp = dst->pixels.data() + (dy * dst->w + dx) * bpp;
            std::memcpy(dp, sp, bpp);
        }
    }
}

void draw_sprite_h_flip(BITMAP* dst, const BITMAP* src, int x, int y) {
    if (!src || !dst) return;
    int bpp = src->bpp();
    for (int row = 0; row < src->h; ++row) {
        int dy = y + row;
        for (int col = 0; col < src->w; ++col) {
            int sx = src->w - 1 - col;
            int dx = x + col;
            if (dx < 0 || dy < 0 || dx >= dst->w || dy >= dst->h) continue;
            const uint8_t* sp = src->pixels.data() + (row * src->w + sx) * bpp;
            if (bpp == 1 && sp[0] == 0) continue;
            if (bpp == 4 && sp[3] == 0) continue;
            uint8_t* dp = dst->pixels.data() + (dy * dst->w + dx) * bpp;
            std::memcpy(dp, sp, bpp);
        }
    }
}

// ---------------------------------------------------------------------------
// rotate_sprite — bilinear sampling over a 16.16 fixed-point angle
// Allegro's coordinate system: angle 0x100000 = full rotation (2π)
// ---------------------------------------------------------------------------

static inline void rotateCore(BITMAP* dst, const BITMAP* src,
                               int dx, int dy,
                               float cx_src, float cy_src,
                               float sin_a, float cos_a) {
    if (!src || !dst) return;
    int bpp = src->bpp();
    int hw = src->w / 2, hh = src->h / 2;

    // Bounding box of rotated sprite
    float corners[4][2] = {
        {-hw * cos_a - -hh * sin_a, -hw * sin_a + -hh * cos_a},
        { hw * cos_a - -hh * sin_a,  hw * sin_a + -hh * cos_a},
        {-hw * cos_a -  hh * sin_a, -hw * sin_a +  hh * cos_a},
        { hw * cos_a -  hh * sin_a,  hw * sin_a +  hh * cos_a},
    };
    float minX = corners[0][0], maxX = corners[0][0];
    float minY = corners[0][1], maxY = corners[0][1];
    for (int i = 1; i < 4; ++i) {
        minX = std::min(minX, corners[i][0]); maxX = std::max(maxX, corners[i][0]);
        minY = std::min(minY, corners[i][1]); maxY = std::max(maxY, corners[i][1]);
    }

    int x0 = (int)std::floor(dx + minX), x1 = (int)std::ceil(dx + maxX);
    int y0 = (int)std::floor(dy + minY), y1 = (int)std::ceil(dy + maxY);
    if (dst->clip) {
        x0 = std::max(x0, dst->cl); x1 = std::min(x1, dst->cr);
        y0 = std::max(y0, dst->ct); y1 = std::min(y1, dst->cb);
    }

    for (int yd = y0; yd < y1; ++yd) {
        for (int xd = x0; xd < x1; ++xd) {
            // Inverse rotate back to source coordinates
            float ox = xd - dx;
            float oy = yd - dy;
            float sx =  ox * cos_a + oy * sin_a + cx_src;
            float sy = -ox * sin_a + oy * cos_a + cy_src;
            int isx = (int)(sx + 0.5f);
            int isy = (int)(sy + 0.5f);
            if (isx < 0 || isx >= src->w || isy < 0 || isy >= src->h) continue;
            const uint8_t* sp = src->pixels.data() + (isy * src->w + isx) * bpp;
            if (bpp == 1 && sp[0] == 0) continue;
            if (bpp == 4 && sp[3] == 0) continue;
            uint8_t* dp = dst->pixels.data() + (yd * dst->w + xd) * bpp;
            std::memcpy(dp, sp, bpp);
        }
    }
}

void rotate_sprite(BITMAP* dst, const BITMAP* src, int x, int y, fixed angle) {
    // Allegro: angle 0x100000 = full turn (2π)
    float a = static_cast<float>(angle) / 1048576.0f * 2.0f * static_cast<float>(M_PI);
    float sin_a = std::sin(a), cos_a = std::cos(a);
    float cx = src->w / 2.0f, cy = src->h / 2.0f;
    rotateCore(dst, src, x + (int)cx, y + (int)cy, cx, cy, sin_a, cos_a);
}

void pivot_sprite(BITMAP* dst, const BITMAP* src,
                  int x, int y, int cx, int cy, fixed angle) {
    float a = static_cast<float>(angle) / 1048576.0f * 2.0f * static_cast<float>(M_PI);
    float sin_a = std::sin(a), cos_a = std::cos(a);
    rotateCore(dst, src, x, y, static_cast<float>(cx), static_cast<float>(cy), sin_a, cos_a);
}

// ---------------------------------------------------------------------------
// Blender setters — capture blend mode for create_blender_table
// ---------------------------------------------------------------------------

static void captureBlend(BlendMode mode, int r, int g, int b, int a) {
    s_CurrentBlendMode = mode;
    s_BlendR = r; s_BlendG = g; s_BlendB = b; s_BlendA = a;
}
void set_burn_blender(int r,int g,int b,int a)       { captureBlend(BlendMode::Burn,       r,g,b,a); }
void set_color_blender(int r,int g,int b,int a)      { captureBlend(BlendMode::Color,      r,g,b,a); }
void set_difference_blender(int r,int g,int b,int a) { captureBlend(BlendMode::Difference, r,g,b,a); }
void set_dissolve_blender(int r,int g,int b,int a)   { captureBlend(BlendMode::Dissolve,   r,g,b,a); }
void set_dodge_blender(int r,int g,int b,int a)      { captureBlend(BlendMode::Dodge,      r,g,b,a); }
void set_invert_blender(int r,int g,int b,int a)     { captureBlend(BlendMode::Invert,     r,g,b,a); }
void set_luminance_blender(int r,int g,int b,int a)  { captureBlend(BlendMode::Luminance,  r,g,b,a); }
void set_multiply_blender(int r,int g,int b,int a)   { captureBlend(BlendMode::Multiply,   r,g,b,a); }
void set_saturation_blender(int r,int g,int b,int a) { captureBlend(BlendMode::Saturation, r,g,b,a); }
void set_screen_blender(int r,int g,int b,int a)     { captureBlend(BlendMode::Screen,     r,g,b,a); }
void set_alpha_blender()                              { captureBlend(BlendMode::Trans, 0,0,0,128); }
void set_trans_blender(int r,int g,int b,int a)      { captureBlend(BlendMode::Trans, r,g,b,a); }

// ---------------------------------------------------------------------------
// create_blender_table — builds a 256×256 indexed COLOR_MAP LUT
// This is called at startup/level-load, not per-frame, so cost is acceptable.
// ---------------------------------------------------------------------------

// Helper: clamp to [0, 255]
static inline int clamp8(int v) { return v < 0 ? 0 : v > 255 ? 255 : v; }

// Find the closest palette index to an (r,g,b) triple
static int closestPaletteEntry(const PALETTE pal, int r, int g, int b) {
    int best = 0, bestDist = 0x7FFFFFFF;
    for (int i = 1; i < 256; ++i) { // skip index 0 (transparent)
        int dr = (int)pal[i][0] - r;
        int dg = (int)pal[i][1] - g;
        int db = (int)pal[i][2] - b;
        int d = dr*dr + dg*dg + db*db;
        if (d < bestDist) { bestDist = d; best = i; }
    }
    return best;
}

void create_blender_table(COLOR_MAP* table, const PALETTE pal, void*) {
    if (!table) return;
    float alpha = s_BlendA / 255.0f;

    for (int src = 0; src < 256; ++src) {
        float sr = pal[src][0], sg = pal[src][1], sb = pal[src][2];
        for (int dst = 0; dst < 256; ++dst) {
            float dr = pal[dst][0], dg = pal[dst][1], db = pal[dst][2];
            float or_, og_, ob_;

            switch (s_CurrentBlendMode) {
            case BlendMode::Screen:
                or_ = 255.0f - (255.0f - sr) * (255.0f - dr) / 255.0f;
                og_ = 255.0f - (255.0f - sg) * (255.0f - dg) / 255.0f;
                ob_ = 255.0f - (255.0f - sb) * (255.0f - db) / 255.0f;
                or_ = dr + (or_ - dr) * alpha;
                og_ = dg + (og_ - dg) * alpha;
                ob_ = db + (ob_ - db) * alpha;
                break;
            case BlendMode::Multiply:
                or_ = dr + (sr * dr / 255.0f - dr) * alpha;
                og_ = dg + (sg * dg / 255.0f - dg) * alpha;
                ob_ = db + (sb * db / 255.0f - db) * alpha;
                break;
            case BlendMode::Difference:
                or_ = dr + (std::abs(sr - dr) - dr) * alpha;
                og_ = dg + (std::abs(sg - dg) - dg) * alpha;
                ob_ = db + (std::abs(sb - db) - db) * alpha;
                break;
            case BlendMode::Dodge:
                or_ = dr + (std::min(255.0f, dr * 255.0f / (255.0f - sr + 1.0f)) - dr) * alpha;
                og_ = dg + (std::min(255.0f, dg * 255.0f / (255.0f - sg + 1.0f)) - dg) * alpha;
                ob_ = db + (std::min(255.0f, db * 255.0f / (255.0f - sb + 1.0f)) - db) * alpha;
                break;
            case BlendMode::Burn:
                or_ = dr + (std::max(0.0f, 255.0f - (255.0f - dr) * 255.0f / (sr + 1.0f)) - dr) * alpha;
                og_ = dg + (std::max(0.0f, 255.0f - (255.0f - dg) * 255.0f / (sg + 1.0f)) - dg) * alpha;
                ob_ = db + (std::max(0.0f, 255.0f - (255.0f - db) * 255.0f / (sb + 1.0f)) - db) * alpha;
                break;
            case BlendMode::Invert:
                or_ = dr + (255.0f - sr - dr) * alpha;
                og_ = dg + (255.0f - sg - dg) * alpha;
                ob_ = db + (255.0f - sb - db) * alpha;
                break;
            case BlendMode::Trans:
            default:
                // Standard alpha blend: src * alpha + dst * (1 - alpha)
                or_ = sr * alpha + dr * (1.0f - alpha);
                og_ = sg * alpha + dg * (1.0f - alpha);
                ob_ = sb * alpha + db * (1.0f - alpha);
                break;
            }

            table->data[src][dst] = (uint8_t)closestPaletteEntry(pal,
                clamp8((int)or_), clamp8((int)og_), clamp8((int)ob_));
        }
    }
}

void create_color_table(COLOR_MAP* table, const PALETTE pal,
                        int (*blend)(PALETTE, int, int), void* cb) {
    if (!table || !blend) return;
    // PALETTE is uint8_t[256][4]; cast away const to match legacy Allegro callback signature
    PALETTE mutablePal;
    std::memcpy(mutablePal, pal, sizeof(PALETTE));
    for (int s = 0; s < 256; ++s) {
        for (int d = 0; d < 256; ++d) {
            table->data[s][d] = (uint8_t)blend(mutablePal, s, d);
        }
    }
}

void create_trans_table(COLOR_MAP* table, const PALETTE pal, int r, int g, int b, void*) {
    if (!table) return;
    float fr = r / 255.0f, fg = g / 255.0f, fb = b / 255.0f;
    for (int src = 0; src < 256; ++src) {
        float sr = pal[src][0], sg = pal[src][1], sb_ = pal[src][2];
        for (int dst = 0; dst < 256; ++dst) {
            float dr = pal[dst][0], dg = pal[dst][1], db_ = pal[dst][2];
            int nr = clamp8((int)(sr * fr + dr * (1.0f - fr)));
            int ng = clamp8((int)(sg * fg + dg * (1.0f - fg)));
            int nb = clamp8((int)(sb_ * fb + db_ * (1.0f - fb)));
            table->data[src][dst] = (uint8_t)closestPaletteEntry(pal, nr, ng, nb);
        }
    }
}

// ---------------------------------------------------------------------------
// save_png — forward-declared in AllegroCompat.h, implemented here using libpng
// For the web build we delegate to FrameMan::SaveIndexedPNG.
// ---------------------------------------------------------------------------
int save_png(const char* filename, BITMAP* bmp, const PALETTE pal) {
    // Stub — FrameMan::SaveIndexedPNG handles the actual libpng write.
    (void)filename; (void)bmp; (void)pal;
    return 0;
}
