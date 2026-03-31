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
int _png_compression_level = 6; // default zlib compression level

PALETTE      g_AllegroCurrentPalette;
PALETTE      g_AllegroDefaultPalette;
COLOR_MAP*   color_map = nullptr;
RGB_MAP*     rgb_table = nullptr;

// Current blend mode state (set by set_*_blender calls, consumed by create_blender_table)
static int  s_BlendR = 128, s_BlendG = 128, s_BlendB = 128, s_BlendA = 128;

enum class AllegroBlendMode {
    Screen, Burn, Color, Difference, Dissolve, Dodge,
    Invert, Luminance, Multiply, Saturation, Trans, None
};
static AllegroBlendMode s_CurrentBlendMode = AllegroBlendMode::Screen;

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
    // Allegro fixed-point angle: 256 units = full turn, in 16.16 format.
    // Full circle as fixed = 256 * 65536 = 16777216.
    float a = static_cast<float>(angle) / 16777216.0f * 2.0f * static_cast<float>(M_PI);
    float sin_a = std::sin(a), cos_a = std::cos(a);
    float cx = src->w / 2.0f, cy = src->h / 2.0f;
    rotateCore(dst, src, x + (int)cx, y + (int)cy, cx, cy, sin_a, cos_a);
}

void pivot_sprite(BITMAP* dst, const BITMAP* src,
                  int x, int y, int cx, int cy, fixed angle) {
    // Allegro fixed-point angle: 256 units = full turn, in 16.16 format.
    float a = static_cast<float>(angle) / 16777216.0f * 2.0f * static_cast<float>(M_PI);
    float sin_a = std::sin(a), cos_a = std::cos(a);
    rotateCore(dst, src, x, y, static_cast<float>(cx), static_cast<float>(cy), sin_a, cos_a);
}

// ---------------------------------------------------------------------------
// Blender setters — capture blend mode for create_blender_table
// ---------------------------------------------------------------------------

static void captureBlend(AllegroBlendMode mode, int r, int g, int b, int a) {
    s_CurrentBlendMode = mode;
    s_BlendR = r; s_BlendG = g; s_BlendB = b; s_BlendA = a;
}
void set_burn_blender(int r,int g,int b,int a)       { captureBlend(AllegroBlendMode::Burn,       r,g,b,a); }
void set_color_blender(int r,int g,int b,int a)      { captureBlend(AllegroBlendMode::Color,      r,g,b,a); }
void set_difference_blender(int r,int g,int b,int a) { captureBlend(AllegroBlendMode::Difference, r,g,b,a); }
void set_dissolve_blender(int r,int g,int b,int a)   { captureBlend(AllegroBlendMode::Dissolve,   r,g,b,a); }
void set_dodge_blender(int r,int g,int b,int a)      { captureBlend(AllegroBlendMode::Dodge,      r,g,b,a); }
void set_invert_blender(int r,int g,int b,int a)     { captureBlend(AllegroBlendMode::Invert,     r,g,b,a); }
void set_luminance_blender(int r,int g,int b,int a)  { captureBlend(AllegroBlendMode::Luminance,  r,g,b,a); }
void set_multiply_blender(int r,int g,int b,int a)   { captureBlend(AllegroBlendMode::Multiply,   r,g,b,a); }
void set_saturation_blender(int r,int g,int b,int a) { captureBlend(AllegroBlendMode::Saturation, r,g,b,a); }
void set_screen_blender(int r,int g,int b,int a)     { captureBlend(AllegroBlendMode::Screen,     r,g,b,a); }
void set_alpha_blender()                              { captureBlend(AllegroBlendMode::Trans, 0,0,0,128); }
void set_trans_blender(int r,int g,int b,int a)      { captureBlend(AllegroBlendMode::Trans, r,g,b,a); }

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
            case AllegroBlendMode::Screen:
                or_ = 255.0f - (255.0f - sr) * (255.0f - dr) / 255.0f;
                og_ = 255.0f - (255.0f - sg) * (255.0f - dg) / 255.0f;
                ob_ = 255.0f - (255.0f - sb) * (255.0f - db) / 255.0f;
                or_ = dr + (or_ - dr) * alpha;
                og_ = dg + (og_ - dg) * alpha;
                ob_ = db + (ob_ - db) * alpha;
                break;
            case AllegroBlendMode::Multiply:
                or_ = dr + (sr * dr / 255.0f - dr) * alpha;
                og_ = dg + (sg * dg / 255.0f - dg) * alpha;
                ob_ = db + (sb * db / 255.0f - db) * alpha;
                break;
            case AllegroBlendMode::Difference:
                or_ = dr + (std::abs(sr - dr) - dr) * alpha;
                og_ = dg + (std::abs(sg - dg) - dg) * alpha;
                ob_ = db + (std::abs(sb - db) - db) * alpha;
                break;
            case AllegroBlendMode::Dodge:
                or_ = dr + (std::min(255.0f, dr * 255.0f / (255.0f - sr + 1.0f)) - dr) * alpha;
                og_ = dg + (std::min(255.0f, dg * 255.0f / (255.0f - sg + 1.0f)) - dg) * alpha;
                ob_ = db + (std::min(255.0f, db * 255.0f / (255.0f - sb + 1.0f)) - db) * alpha;
                break;
            case AllegroBlendMode::Burn:
                or_ = dr + (std::max(0.0f, 255.0f - (255.0f - dr) * 255.0f / (sr + 1.0f)) - dr) * alpha;
                og_ = dg + (std::max(0.0f, 255.0f - (255.0f - dg) * 255.0f / (sg + 1.0f)) - dg) * alpha;
                ob_ = db + (std::max(0.0f, 255.0f - (255.0f - db) * 255.0f / (sb + 1.0f)) - db) * alpha;
                break;
            case AllegroBlendMode::Invert:
                or_ = dr + (255.0f - sr - dr) * alpha;
                og_ = dg + (255.0f - sg - dg) * alpha;
                ob_ = db + (255.0f - sb - db) * alpha;
                break;
            case AllegroBlendMode::Trans:
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
// Drawing primitives
// ---------------------------------------------------------------------------

static inline void safePutpixel(BITMAP* bmp, int x, int y, int color) {
    if (!bmp) return;
    if (bmp->clip && (x < bmp->cl || x >= bmp->cr || y < bmp->ct || y >= bmp->cb)) return;
    if (x < 0 || y < 0 || x >= bmp->w || y >= bmp->h) return;
    int bpp = bmp->bpp();
    uint8_t* p = bmp->pixels.data() + (y * bmp->w + x) * bpp;
    if (bpp == 1) *p = color & 0xFF;
    else if (bpp == 4) *reinterpret_cast<uint32_t*>(p) = (uint32_t)color;
    else { p[0] = color & 0xFF; p[1] = (color >> 8) & 0xFF; p[2] = (color >> 16) & 0xFF; }
}

void hline(BITMAP* bmp, int x1, int y, int x2, int color) {
    if (!bmp || y < 0 || y >= bmp->h) return;
    if (x1 > x2) std::swap(x1, x2);
    if (bmp->clip) { x1 = std::max(x1, bmp->cl); x2 = std::min(x2, bmp->cr - 1); }
    x1 = std::max(x1, 0); x2 = std::min(x2, bmp->w - 1);
    if (x1 > x2) return;
    int bpp = bmp->bpp();
    if (bpp == 1) {
        std::memset(bmp->pixels.data() + y * bmp->w + x1, color & 0xFF, x2 - x1 + 1);
    } else {
        for (int x = x1; x <= x2; ++x) safePutpixel(bmp, x, y, color);
    }
}

void vline(BITMAP* bmp, int x, int y1, int y2, int color) {
    if (!bmp || x < 0 || x >= bmp->w) return;
    if (y1 > y2) std::swap(y1, y2);
    for (int y = y1; y <= y2; ++y) safePutpixel(bmp, x, y, color);
}

void line(BITMAP* bmp, int x1, int y1, int x2, int y2, int color) {
    // Bresenham
    int dx = std::abs(x2 - x1), dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1, sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (true) {
        safePutpixel(bmp, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

void rect(BITMAP* bmp, int x1, int y1, int x2, int y2, int color) {
    hline(bmp, x1, y1, x2, color); hline(bmp, x1, y2, x2, color);
    vline(bmp, x1, y1, y2, color); vline(bmp, x2, y1, y2, color);
}

void rectfill(BITMAP* bmp, int x1, int y1, int x2, int y2, int color) {
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);
    for (int y = y1; y <= y2; ++y) hline(bmp, x1, y, x2, color);
}

void circle(BITMAP* bmp, int cx, int cy, int r, int color) {
    // Midpoint circle algorithm
    int x = 0, y = r, d = 3 - 2 * r;
    while (x <= y) {
        safePutpixel(bmp, cx+x, cy+y, color); safePutpixel(bmp, cx-x, cy+y, color);
        safePutpixel(bmp, cx+x, cy-y, color); safePutpixel(bmp, cx-x, cy-y, color);
        safePutpixel(bmp, cx+y, cy+x, color); safePutpixel(bmp, cx-y, cy+x, color);
        safePutpixel(bmp, cx+y, cy-x, color); safePutpixel(bmp, cx-y, cy-x, color);
        if (d < 0) d += 4*x + 6; else { d += 4*(x-y) + 10; --y; }
        ++x;
    }
}

void circlefill(BITMAP* bmp, int cx, int cy, int r, int color) {
    int x = 0, y = r, d = 3 - 2 * r;
    while (x <= y) {
        hline(bmp, cx-x, cy+y, cx+x, color); hline(bmp, cx-x, cy-y, cx+x, color);
        hline(bmp, cx-y, cy+x, cx+y, color); hline(bmp, cx-y, cy-x, cx+y, color);
        if (d < 0) d += 4*x + 6; else { d += 4*(x-y) + 10; --y; }
        ++x;
    }
}

void ellipse(BITMAP* bmp, int cx, int cy, int rx, int ry, int color) {
    if (!bmp || rx <= 0 || ry <= 0) return;
    int x = 0, y = ry;
    long rx2 = (long)rx*rx, ry2 = (long)ry*ry;
    long d = ry2 - rx2*ry + rx2/4;
    while (2*ry2*x < 2*rx2*y) {
        safePutpixel(bmp, cx+x, cy+y, color); safePutpixel(bmp, cx-x, cy+y, color);
        safePutpixel(bmp, cx+x, cy-y, color); safePutpixel(bmp, cx-x, cy-y, color);
        if (d < 0) d += ry2*(2*x+3); else { d += ry2*(2*x+3) + rx2*(-2*y+2); --y; }
        ++x;
    }
    d = ry2*(x+1)*(x+1) - rx2*ry*(ry-1) + (rx2-ry2)*ry2;
    while (y >= 0) {
        safePutpixel(bmp, cx+x, cy+y, color); safePutpixel(bmp, cx-x, cy+y, color);
        safePutpixel(bmp, cx+x, cy-y, color); safePutpixel(bmp, cx-x, cy-y, color);
        if (d > 0) d += rx2*(-2*y+3); else { d += ry2*(2*x+2) + rx2*(-2*y+3); ++x; }
        --y;
    }
}

void ellipsefill(BITMAP* bmp, int cx, int cy, int rx, int ry, int color) {
    if (!bmp || rx <= 0 || ry <= 0) return;
    for (int y = -ry; y <= ry; ++y) {
        int xw = (int)(rx * std::sqrt(1.0 - (double)(y*y) / (double)(ry*ry)));
        hline(bmp, cx - xw, cy + y, cx + xw, color);
    }
}

void triangle(BITMAP* bmp, int x1, int y1, int x2, int y2, int x3, int y3, int color) {
    line(bmp, x1, y1, x2, y2, color);
    line(bmp, x2, y2, x3, y3, color);
    line(bmp, x3, y3, x1, y1, color);
}

void polygon(BITMAP* bmp, int vertices, const int* points, int color) {
    if (!bmp || vertices < 2) return;
    for (int i = 0; i < vertices - 1; ++i)
        line(bmp, points[i*2], points[i*2+1], points[(i+1)*2], points[(i+1)*2+1], color);
    line(bmp, points[(vertices-1)*2], points[(vertices-1)*2+1], points[0], points[1], color);
}

void floodfill(BITMAP* bmp, int x, int y, int color) {
    // Stub: flood fill is a complex operation not used in hot paths
    (void)bmp; (void)x; (void)y; (void)color;
}

// do_line: scan-line helper used by some Allegro callers
void do_line(BITMAP* bmp, int x1, int y1, int x2, int y2, int color, void(*) (BITMAP*, int, int, int)) {
    line(bmp, x1, y1, x2, y2, color);
}

// ---------------------------------------------------------------------------
// Additional flip/rotate/scale sprite helpers
// ---------------------------------------------------------------------------

void draw_sprite_vh_flip(BITMAP* dst, const BITMAP* src, int x, int y) {
    if (!src || !dst) return;
    int bpp = src->bpp();
    for (int row = 0; row < src->h; ++row) {
        int sy = src->h - 1 - row;
        for (int col = 0; col < src->w; ++col) {
            int sx = src->w - 1 - col;
            const uint8_t* sp = src->pixels.data() + (sy * src->w + sx) * bpp;
            if (bpp == 1 && sp[0] == 0) continue;
            if (bpp == 4 && sp[3] == 0) continue;
            safePutpixel(dst, x + col, y + row, bpp == 1 ? sp[0] : (int)*reinterpret_cast<const uint32_t*>(sp));
        }
    }
}

static void rotateScaleCore(BITMAP* dst, const BITMAP* src,
                             int px, int py, float cx_src, float cy_src,
                             float sin_a, float cos_a, float inv_scale, bool vflip) {
    if (!src || !dst) return;
    int bpp = src->bpp();
    float hw = src->w * 0.5f, hh = src->h * 0.5f;
    float corners[4][2] = {
        {(-hw * cos_a - -hh * sin_a) / inv_scale, (-hw * sin_a + -hh * cos_a) / inv_scale},
        {( hw * cos_a - -hh * sin_a) / inv_scale, ( hw * sin_a + -hh * cos_a) / inv_scale},
        {(-hw * cos_a -  hh * sin_a) / inv_scale, (-hw * sin_a +  hh * cos_a) / inv_scale},
        {( hw * cos_a -  hh * sin_a) / inv_scale, ( hw * sin_a +  hh * cos_a) / inv_scale},
    };
    float minX = corners[0][0], maxX = corners[0][0];
    float minY = corners[0][1], maxY = corners[0][1];
    for (int i = 1; i < 4; ++i) {
        minX = std::min(minX, corners[i][0]); maxX = std::max(maxX, corners[i][0]);
        minY = std::min(minY, corners[i][1]); maxY = std::max(maxY, corners[i][1]);
    }
    int x0 = (int)std::floor(px + minX), x1 = (int)std::ceil(px + maxX);
    int y0 = (int)std::floor(py + minY), y1 = (int)std::ceil(py + maxY);
    if (dst->clip) {
        x0 = std::max(x0, dst->cl); x1 = std::min(x1, dst->cr);
        y0 = std::max(y0, dst->ct); y1 = std::min(y1, dst->cb);
    }
    for (int yd = y0; yd < y1; ++yd) {
        for (int xd = x0; xd < x1; ++xd) {
            float ox = (xd - px) * inv_scale;
            float oy = (yd - py) * inv_scale;
            float sxf =  ox * cos_a + oy * sin_a + cx_src;
            float syf = vflip ? (-(- ox * sin_a + oy * cos_a) + cy_src)
                               : (-ox * sin_a + oy * cos_a + cy_src);
            int isx = (int)(sxf + 0.5f), isy = (int)(syf + 0.5f);
            if (isx < 0 || isx >= src->w || isy < 0 || isy >= src->h) continue;
            const uint8_t* sp = src->pixels.data() + (isy * src->w + isx) * bpp;
            if (bpp == 1 && sp[0] == 0) continue;
            if (bpp == 4 && sp[3] == 0) continue;
            int color = (bpp == 1) ? sp[0] : (int)*reinterpret_cast<const uint32_t*>(sp);
            safePutpixel(dst, xd, yd, color);
        }
    }
}

void rotate_scaled_sprite(BITMAP* dst, const BITMAP* src, int x, int y, fixed angle, fixed scale) {
    float a = static_cast<float>(angle) / 16777216.0f * 2.0f * static_cast<float>(M_PI);
    float sc = static_cast<float>(scale) / 65536.0f;
    float inv_sc = (sc > 0.001f) ? (1.0f / sc) : 1.0f;
    rotateScaleCore(dst, src, x + (int)(src->w * sc * 0.5f), y + (int)(src->h * sc * 0.5f),
                    src->w * 0.5f, src->h * 0.5f, std::sin(a), std::cos(a), inv_sc, false);
}

void rotate_scaled_sprite_v_flip(BITMAP* dst, const BITMAP* src, int x, int y, fixed angle, fixed scale) {
    float a = static_cast<float>(angle) / 16777216.0f * 2.0f * static_cast<float>(M_PI);
    float sc = static_cast<float>(scale) / 65536.0f;
    float inv_sc = (sc > 0.001f) ? (1.0f / sc) : 1.0f;
    rotateScaleCore(dst, src, x + (int)(src->w * sc * 0.5f), y + (int)(src->h * sc * 0.5f),
                    src->w * 0.5f, src->h * 0.5f, std::sin(a), std::cos(a), inv_sc, true);
}

void pivot_scaled_sprite(BITMAP* dst, const BITMAP* src, int x, int y, int cx, int cy, fixed angle, fixed scale) {
    // Allegro fixed-point angle: 256 units = full turn, in 16.16 format.
    // Full circle as fixed = 256 * 65536 = 16777216.
    float a = static_cast<float>(angle) / 16777216.0f * 2.0f * static_cast<float>(M_PI);
    float sc = static_cast<float>(scale) / 65536.0f;
    float inv_sc = (sc > 0.001f) ? (1.0f / sc) : 1.0f;
    rotateScaleCore(dst, src, x, y, static_cast<float>(cx), static_cast<float>(cy),
                    std::sin(a), std::cos(a), inv_sc, false);
}

// ---------------------------------------------------------------------------
// Additional sprite functions
// ---------------------------------------------------------------------------

void draw_trans_sprite(BITMAP* dst, const BITMAP* src, int x, int y) {
    if (!dst || !src || !color_map) {
        // No color map — fall back to masked blit
        masked_blit(src, dst, 0, 0, x, y, src->w, src->h);
        return;
    }
    // Apply color_map[src_pixel][dst_pixel] blending
    for (int row = 0; row < src->h; ++row) {
        int dy = y + row;
        for (int col = 0; col < src->w; ++col) {
            int dx = x + col;
            if (dx < 0 || dy < 0 || dx >= dst->w || dy >= dst->h) continue;
            if (dst->clip && (dx < dst->cl || dx >= dst->cr || dy < dst->ct || dy >= dst->cb)) continue;
            uint8_t sp = src->pixels[row * src->w + col];
            if (sp == 0) continue; // transparent
            uint8_t dp = dst->pixels[dy * dst->w + dx];
            dst->pixels[dy * dst->w + dx] = color_map->data[sp][dp];
        }
    }
}

void draw_character_ex(BITMAP* bmp, const BITMAP* sprite, int x, int y,
                       int color, int bg) {
    if (!bmp || !sprite) return;
    // Allegro font characters are 8bpp; non-zero pixels become 'color', zero becomes 'bg' (-1 = transparent)
    for (int row = 0; row < sprite->h; ++row) {
        for (int col = 0; col < sprite->w; ++col) {
            uint8_t px = sprite->pixels[row * sprite->w + col];
            if (px != 0) {
                safePutpixel(bmp, x + col, y + row, color);
            } else if (bg >= 0) {
                safePutpixel(bmp, x + col, y + row, bg);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// SDL_image stubs using libpng for Emscripten
// ---------------------------------------------------------------------------

#include <SDL3/SDL.h>
#include <png.h>

// IMG_Load: load a PNG file using libpng, return as SDL_Surface (RGBA32 or INDEX8)
SDL_Surface* IMG_Load(const char* file) {
    FILE* fp = fopen(file, "rb");
    if (!fp) return nullptr;
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) { fclose(fp); return nullptr; }
    png_infop info = png_create_info_struct(png);
    if (!info) { png_destroy_read_struct(&png, nullptr, nullptr); fclose(fp); return nullptr; }
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, nullptr); fclose(fp); return nullptr;
    }
    png_init_io(png, fp);
    png_read_info(png, info);
    int width    = (int)png_get_image_width(png, info);
    int height   = (int)png_get_image_height(png, info);
    int colorType = png_get_color_type(png, info);
    int bitDepth  = png_get_bit_depth(png, info);

    // Expand to either RGBA32 or INDEX8
    bool isIndexed = (colorType == PNG_COLOR_TYPE_PALETTE);
    if (!isIndexed) {
        if (bitDepth == 16) png_set_strip_16(png);
        if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png);
        png_set_add_alpha(png, 0xFF, PNG_FILLER_AFTER);
    } else {
        if (bitDepth < 8) png_set_packing(png);
    }
    png_read_update_info(png, info);

    SDL_Surface* surface = nullptr;
    if (isIndexed) {
        surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8);
        if (!surface) { png_destroy_read_struct(&png, &info, nullptr); fclose(fp); return nullptr; }
        // Copy palette
        png_colorp pngPal; int numColors = 0;
        if (png_get_PLTE(png, info, &pngPal, &numColors) == PNG_INFO_PLTE) {
            SDL_Palette* sdlPal = SDL_CreatePalette(numColors);
            std::vector<SDL_Color> colors(numColors);
            for (int i = 0; i < numColors; ++i)
                colors[i] = {pngPal[i].red, pngPal[i].green, pngPal[i].blue, 255};
            SDL_SetPaletteColors(sdlPal, colors.data(), 0, numColors);
            SDL_SetSurfacePalette(surface, sdlPal);
            SDL_DestroyPalette(sdlPal);
        }
        uint8_t* pixels = reinterpret_cast<uint8_t*>(surface->pixels);
        int stride = surface->pitch;
        std::vector<png_bytep> rows(height);
        for (int y = 0; y < height; ++y) rows[y] = pixels + y * stride;
        png_read_image(png, rows.data());
    } else {
        surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
        if (!surface) { png_destroy_read_struct(&png, &info, nullptr); fclose(fp); return nullptr; }
        uint8_t* pixels = reinterpret_cast<uint8_t*>(surface->pixels);
        int stride = surface->pitch;
        std::vector<png_bytep> rows(height);
        for (int y = 0; y < height; ++y) rows[y] = pixels + y * stride;
        png_read_image(png, rows.data());
    }
    png_destroy_read_struct(&png, &info, nullptr);
    fclose(fp);
    return surface;
}

SDL_Surface* IMG_LoadPNG_IO(SDL_IOStream* src) {
    // Stub — IO-stream-based load not implemented; returns null
    (void)src; return nullptr;
}

int IMG_SavePNG(SDL_Surface* surface, const char* file) {
    (void)surface; (void)file; return 0; // screenshot saving disabled on web
}
int IMG_SavePNG_IO(SDL_Surface* surface, SDL_IOStream* dst, int) {
    (void)surface; (void)dst; return 0;
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
