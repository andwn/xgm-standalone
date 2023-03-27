#ifndef MD_XGM_H
#define MD_XGM_H

#include <stdint.h>

extern void xgm_init(void);

// Start playing a BGM track
// Expects a pointer to XGC music data (xgmtool output format)
extern void xgm_music_play(const uint8_t *xgc);

// Pause / Stop any playing BGM
extern void xgm_music_pause(void);

// Register a PCM sample that can be played using xgm_pcm_play()
// Expects a pointer to raw PCM data. Signed, 8-bit, 14000Hz, and 256 byte aligned.
extern void xgm_pcm_set(uint8_t id, const uint8_t *sample, uint32_t len);

// Start playing a sound effect
// Expects an ID number registered using xgm_pcm_set() previously,
// A priority between 0 (low) and 15 (max), and a channel between 0 and 4.
// However, channel 0 is reserved for the music. So stick to 1, 2, and 3.
extern void xgm_pcm_play(uint8_t id, uint8_t priority, uint16_t channel);

// This has to run once each frame. Put it in the vertical interrupt handler,
// or just after waiting for vblank
extern void xgm_vblank_process(void);

#endif //MD_XGM_H
