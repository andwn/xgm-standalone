#ifndef MD_VDP_H
#define MD_VDP_H

#include <stdint.h>

#define VDP_PLANE_W             0xB000U
#define VDP_PLANE_A             0xC000U
#define VDP_PLANE_B             0xE000U
#define VDP_SPRITE_TABLE        0xF800U
#define VDP_HSCROLL_TABLE       0xFC00U

#define PLAN_WIDTH              64
#define PLAN_HEIGHT             32
#define PLAN_WIDTH_SFT          6
#define PLAN_HEIGHT_SFT         5

#define HSCROLL_PLANE           0
#define VSCROLL_PLANE           0

#define TILE_SIZE               32

#define TILE_FONTINDEX          ((VDP_PLANE_W >> 5) - 96)

#define TILE_ATTR(pal, prio, flipV, flipH, index)                               \
    ((((uint16_t)flipH) << 11) | (((uint16_t)flipV) << 12) |                    \
    (((uint16_t)pal) << 13) | (((uint16_t)prio) << 15) | ((uint16_t)index))

// Set defaults, clear everything
void vdp_init(void);

// Tile patterns
void vdp_tiles_load(const uint32_t *data, uint16_t index, uint16_t num);

// Tile maps
void vdp_map_xy(uint16_t plan, uint16_t tile, uint16_t x, uint16_t y);

void vdp_map_clear(uint16_t plan);

// Palettes
void vdp_colors(uint16_t index, const uint16_t *values, uint16_t count);

void vdp_color(uint16_t index, uint16_t color);

// Scrolling
void vdp_hscroll(uint16_t plan, int16_t hscroll);

void vdp_vscroll(uint16_t plan, int16_t vscroll);

// Text
void vdp_font_load(const uint32_t *tiles);

void vdp_puts(uint16_t plan, const char *str, uint16_t x, uint16_t y);

void vdp_text_clear(uint16_t plan, uint16_t x, uint16_t y, uint16_t len);

#endif
