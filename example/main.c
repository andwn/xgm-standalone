#include <stdint.h>

#include "dma.h"
#include "joy.h"
#include "sys.h"
#include "vdp.h"
#include "../m68k-src/xgm.h"

#include "resources.h"

#define SFX_ID_1   0x40
#define SFX_ID_2   0x41
#define SFX_ID_3   0x42

#define PRI_LOW     2
#define PRI_MID     5
#define PRI_HI      8

void main(void) {
    //__asm__("illegal"); // Error test

    joy_init();
    vdp_init();
    // Gotta call this first
    xgm_init();
    // XGM keeps a table of samples in memory, up to 256. Each entry has an
    // address to the start of a sample, and the length in bytes.
    // So, for each sample, we need to provide this information to xgm_pcm_set.
    // The first 64 or so entries are reserved for BGM samples, so use higher
    // numbers for the ID.
    xgm_pcm_set(SFX_ID_1, SFX_01, SFX_01_end - SFX_01);
    xgm_pcm_set(SFX_ID_2, SFX_02, SFX_02_end - SFX_02);
    xgm_pcm_set(SFX_ID_3, SFX_03, SFX_03_end - SFX_03);

    vdp_puts(VDP_PLANE_A, "XGM Standalone Test ROM", 4, 4);
    vdp_puts(VDP_PLANE_A, "Press A, B, and C for sounds.", 4, 6);
    vdp_puts(VDP_PLANE_A, "All 3 should be able to", 4, 8);
    vdp_puts(VDP_PLANE_A, "play simultaneously.", 4, 9);

    xgm_music_play(BGM_Castle);

    uint32_t timer = 0;

    while(1) {
        joy_update();
        if(joy_pressed(JOY_A)) xgm_pcm_play(SFX_ID_1, PRI_MID, 1);
        if(joy_pressed(JOY_B)) xgm_pcm_play(SFX_ID_2, PRI_MID, 2);
        if(joy_pressed(JOY_C)) xgm_pcm_play(SFX_ID_3, PRI_MID, 3);

        // This frame counter is just here to make sure we don't lock up
        char timec[2] = { 0x20 + (++timer & 0x3F), 0};
        vdp_puts(VDP_PLANE_A, timec, 4, 24);

        sys_wait_vblank();
        // This is already called as part of the vertical interrupt handler in sys.s,
        // but if you don't want to do that, you would put it here instead.
        //xgm_vblank_process();
        dma_flush();
    }
}
