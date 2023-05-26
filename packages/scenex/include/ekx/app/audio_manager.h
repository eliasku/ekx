#ifndef EKX_AUDIO_MANAGER_H
#define EKX_AUDIO_MANAGER_H

#include <ek/audio.h>
#include <ek/math.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    AUDIO_PREF_SOUND = 1,
    AUDIO_PREF_MUSIC = 2,
    AUDIO_PREF_VIBRO = 4,
};

typedef struct audio_manager_t {
    uint32_t prefs;
    res_id /* auph_buffer */ music_res;
    auph_voice music_voice;
    float music_volume;
    float music_pitch;
} audio_manager_t;

void play_music(string_hash_t name);

void set_music_params(float volume, float pitch);

void play_sound(string_hash_t name, float volume, float pitch);

void play_sound_at(string_hash_t name, vec2_t position, float volume, float pitch);

void vibrate(int length);

void update_audio_manager(void);

extern audio_manager_t g_audio;

void init_audio_manager(void);

bool audio_toggle_pref(uint32_t pref);

#ifdef __cplusplus
}
#endif

#endif // EKX_AUDIO_MANAGER_H
