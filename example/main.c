#include <stdint.h>
#include "xgm.h"

void main(void) {
    xgm_init();

    xgm_play(BGM_Test);

    while(1) {

        vdp_wait_vblank();
        xgm_vblank_process();
    }
}
