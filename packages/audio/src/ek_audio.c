#if 0
// for refactoring enable other formats
#define AUPH_WAV
#define AUPH_OGG
#define AUPH_FORCE_NATIVE_NULL_DEVICE
#endif

#define AUPH_MP3

#include <auph/auph.c>

// resources

#include <ek/audio.h>

struct res_audio res_audio;

void audio_setup(void) {
    auph_setup();

    struct res_audio* R = &res_audio;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}
