#include "audio_manager.h"
#include <ek/local_storage.h>

void play_music(string_hash_t name) {
    res_id next_music_res = name ? R_AUDIO(name) : 0;
    if (g_audio.music_res != next_music_res) {
        if (auph_is_active(g_audio.music_voice.id)) {
            auph_stop(g_audio.music_voice.id);
            g_audio.music_voice.id = 0;
        }

        if (next_music_res) {
            auph_buffer buffer = REF_RESOLVE(res_audio, next_music_res);
            if (buffer.id) {
                if (auph_is_buffer_loaded(buffer) && !auph_is_active(g_audio.music_voice.id)) {
                    g_audio.music_voice = auph_play_f(buffer,
                                                      g_audio.music_volume, 0.0f, g_audio.music_pitch,
                                                      true, false,
                                                      AUPH_BUS_MUSIC);
                }
            }
        }
        g_audio.music_res = next_music_res;
    }
}

void play_sound(string_hash_t name, float vol, float pitch) {
    if ((g_audio.prefs & AUDIO_PREF_SOUND) && name) {
        const res_id id = R_AUDIO(name);
        if (id) {
            auph_buffer snd = REF_RESOLVE(res_audio, id);
            if (snd.id) {
                auph_play_f(snd, vol, 0.0f, pitch, false, false, AUPH_BUS_SOUND);
            }
            else {
                // is not ready?
                // log_debug("audio: snd id is 0, %s", hsp_get(name));
            }
        }
        else {
            // no id
            // log_debug("audio: res is 0, %s", hsp_get(name));
        }
    }
}

void play_sound_at(string_hash_t name, const vec2_t position, float volume, float pitch) {
    UNUSED(position);
    if (g_audio.prefs & AUDIO_PREF_SOUND) {
        float rel_volume = volume;
        // TODO: positioning
        //auto spatialPanning = -1 .. 1;
        play_sound(name, rel_volume, pitch);
    }
}

void vibrate(int length) {
    if ((g_audio.prefs & AUDIO_PREF_VIBRO) && length > 0) {
        auph_vibrate(length);
    }
}

void update_audio_manager(void) {
    const uint32_t prefs = g_audio.prefs;
    auph_set_gain(AUPH_BUS_MUSIC.id, (prefs & AUDIO_PREF_MUSIC) ? 1.0f : 0.0f);
    auph_set_gain(AUPH_BUS_SOUND.id, (prefs & AUDIO_PREF_SOUND) ? 1.0f : 0.0f);

    res_id music_res = g_audio.music_res;
    if (music_res) {
        const auph_voice music_voice = g_audio.music_voice;
        const auph_buffer buffer = REF_RESOLVE(res_audio, music_res);
        if (buffer.id && auph_is_buffer_loaded(buffer) && !auph_is_active(music_voice.id)) {
            const float vol = g_audio.music_volume;
            const float rate = g_audio.music_pitch;
            g_audio.music_voice = auph_play_f(buffer, vol, 0.0f, rate, true, false, AUPH_BUS_MUSIC);
        }
    }
}

void set_music_params(float volume, float pitch) {
    g_audio.music_volume = volume;
    g_audio.music_pitch = pitch;
    if (auph_is_active(g_audio.music_voice.id)) {
        auph_set_gain(g_audio.music_voice.id, g_audio.music_volume);
        auph_set_rate(g_audio.music_voice, pitch);
    }
}

audio_manager_t g_audio;

#define AUDIO_FLAGS_KEY "audio"

void init_audio_manager(void) {
    g_audio.prefs = ek_ls_get_i(AUDIO_FLAGS_KEY, AUDIO_PREF_SOUND | AUDIO_PREF_MUSIC | AUDIO_PREF_VIBRO);
    g_audio.music_volume = 1.0f;
    g_audio.music_pitch = 1.0f;
}

bool audio_toggle_pref(uint32_t pref) {
    g_audio.prefs ^= pref;
    ek_ls_set_i(AUDIO_FLAGS_KEY, (int)g_audio.prefs);
    return g_audio.prefs & pref;
}
