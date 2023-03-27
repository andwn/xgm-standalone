#include "dma.h"
#include "sys.h"
#include "vdp.h"

#include "resources.h"

static volatile uint16_t *const vdp_data_port = (uint16_t *) 0xC00000;
static volatile uint16_t *const vdp_ctrl_port = (uint16_t *) 0xC00004;
static volatile uint32_t *const vdp_ctrl_wide = (uint32_t *) 0xC00004;

void vdp_init(void) {
    // Set the registers
    *vdp_ctrl_port = 0x8004;
    *vdp_ctrl_port = 0x8174; // Enable display
    *vdp_ctrl_port = 0x8200 | (VDP_PLANE_A >> 10); // Plane A address
    *vdp_ctrl_port = 0x8300 | (VDP_PLANE_W >> 10); // Window address
    *vdp_ctrl_port = 0x8400 | (VDP_PLANE_B >> 13); // Plane B address
    *vdp_ctrl_port = 0x8500 | (VDP_SPRITE_TABLE >> 9); // Sprite list address
    *vdp_ctrl_port = 0x8600;
    *vdp_ctrl_port = 0x8700; // Background color palette index
    *vdp_ctrl_port = 0x8800;
    *vdp_ctrl_port = 0x8900;
    *vdp_ctrl_port = 0x8A01; // Horizontal interrupt timer
    *vdp_ctrl_port = 0x8B00 | (VSCROLL_PLANE << 2) | HSCROLL_PLANE; // Scroll mode
    *vdp_ctrl_port = 0x8C81; // No interlace or shadow/highlight
    *vdp_ctrl_port = 0x8D00 | (VDP_HSCROLL_TABLE >> 10); // HScroll table address
    *vdp_ctrl_port = 0x8E00;
    *vdp_ctrl_port = 0x8F02; // Auto increment
    *vdp_ctrl_port = 0x9001; // Map size (64x32)
    *vdp_ctrl_port = 0x9100; // Window X
    *vdp_ctrl_port = 0x9200; // Window Y
    // Reset DMA queue
    dma_clear();
    // Reset the tilemaps
    vdp_map_clear(VDP_PLANE_A);
    vdp_hscroll(VDP_PLANE_A, 0);
    vdp_vscroll(VDP_PLANE_A, 0);
    vdp_map_clear(VDP_PLANE_B);
    vdp_hscroll(VDP_PLANE_B, 0);
    vdp_vscroll(VDP_PLANE_B, 0);
    // Dirty hack - blanks out start of sprite list
    vdp_tiles_load(BlankData, VDP_SPRITE_TABLE >> 5, 1);
    // Put blank tile in index 0
    vdp_tiles_load(BlankData, 0, 1);
    // (Re)load the font
    vdp_font_load(PAT_Font);
    vdp_color(0, 0x200);    // Dark blue
    vdp_color(1, 0x000);    // Black
    vdp_color(15, 0xEEE);   // White
}

// Tile patterns

void vdp_tiles_load(const uint32_t *data, uint16_t index, uint16_t num) {
    dma_now(DmaVRAM, (uint32_t) data, index << 5, num << 4, 2);
}

// Tile maps

void vdp_map_xy(uint16_t plan, uint16_t tile, uint16_t x, uint16_t y) {
    uint32_t addr = plan + ((x + (y << PLAN_WIDTH_SFT)) << 1);
    *vdp_ctrl_wide = ((0x4000 + ((addr) & 0x3FFF)) << 16) + (((addr) >> 14) | 0x00);
    *vdp_data_port = tile;
}

void vdp_map_clear(uint16_t plan) {
    uint16_t addr = plan;
    while (addr < plan + 0x1000) {
        dma_now(DmaVRAM, (uint32_t) BlankData, addr, 0x80, 2);
        addr += 0x100;
    }
}

// Palettes

void vdp_colors(uint16_t index, const uint16_t *values, uint16_t count) {
    dma_now(DmaCRAM, (uint32_t) values, index << 1, count, 2);
}

void vdp_color(uint16_t index, uint16_t color) {
    uint16_t ind = index << 1;
    *vdp_ctrl_wide = ((0xC000 + (((uint32_t) ind) & 0x3FFF)) << 16) + ((((uint32_t) ind) >> 14) | 0x00);
    *vdp_data_port = color;
}

// Scroll

void vdp_hscroll(uint16_t plan, int16_t hscroll) {
    uint32_t addr = (plan == VDP_PLANE_A) ? VDP_HSCROLL_TABLE : VDP_HSCROLL_TABLE + 2;
    *vdp_ctrl_wide = ((0x4000 + ((addr) & 0x3FFF)) << 16) + (((addr) >> 14) | 0x00);
    *vdp_data_port = hscroll;
}

void vdp_vscroll(uint16_t plan, int16_t vscroll) {
    uint32_t addr = (plan == VDP_PLANE_A) ? 0 : 2;
    *vdp_ctrl_wide = ((0x4000 + ((addr) & 0x3FFF)) << 16) + (((addr) >> 14) | 0x10);
    *vdp_data_port = vscroll;
}

// Font / Text

void vdp_font_load(const uint32_t *tiles) {
    vdp_tiles_load(tiles, TILE_FONTINDEX, 0x60);
}

void vdp_puts(uint16_t plan, const char *str, uint16_t x, uint16_t y) {
    uint32_t addr = plan + ((x + (y << PLAN_WIDTH_SFT)) << 1);
    *vdp_ctrl_wide = ((0x4000 + ((addr) & 0x3FFF)) << 16) + (((addr) >> 14) | 0x00);
    for (uint16_t i = 0; i < 64 && *str; ++i) {
        // Wrap around the plane, don't fall to next line
        if (i + x == 64) {
            addr -= x << 1;
            *vdp_ctrl_wide = ((0x4000 + ((addr) & 0x3FFF)) << 16) + (((addr) >> 14) | 0x00);
        }
        uint16_t attr = TILE_ATTR(0, 1, 0, 0, TILE_FONTINDEX + *str++ - 0x20);
        *vdp_data_port = attr;
    }
}

void vdp_text_clear(uint16_t plan, uint16_t x, uint16_t y, uint16_t len) {
    uint32_t addr = plan + ((x + (y << PLAN_WIDTH_SFT)) << 1);
    *vdp_ctrl_wide = ((0x4000 + ((addr) & 0x3FFF)) << 16) + (((addr) >> 14) | 0x00);
    while (len--) *vdp_data_port = 0;
}
