
#if defined(__APPLE__)

#include <TargetConditionals.h>

#endif

#include <ek/admob_wrapper.h>
#include <ek/audio.h>
#include <ek/log.h>
#include <ek/time.h>
#include <ek/admob.h>

// implementation

#if defined(NDEBUG) || defined(__ANDROID__) || TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#define EK_ADMOB_SIMULATOR 0
#else
#define EK_ADMOB_SIMULATOR 1
#endif

struct admob_wrapper g_admob_wrapper;

static void admob_wrapper_rewarded_ad_end(void* userdata) {
    UNUSED(userdata);
    if (g_admob_wrapper.rewarded_ad_active) {
        log_info("rewarded ad end");
        auph_mute_pop();
        g_admob_wrapper.rewarded_ad_active = false;
        if (g_admob_wrapper.on_rewarded_ad_completed) {
            g_admob_wrapper.on_rewarded_ad_completed(g_admob_wrapper.user_rewarded);
            g_admob_wrapper.on_rewarded_ad_completed = NULL;
        }
    }
}

static void admob_wrapper_interstitial_end(void* userdata) {
    UNUSED(userdata);
    if (g_admob_wrapper.interstitial_active) {
        log_info("interstitial end");
        auph_mute_pop();
        g_admob_wrapper.interstitial_active = false;
        if (g_admob_wrapper.on_interstitial_completed) {
            g_admob_wrapper.on_interstitial_completed();
            g_admob_wrapper.on_interstitial_completed = NULL;
        }
    }
}

static void admob_wrapper_on_event(void* userdata, ek_admob_event_type event) {
    (void)userdata;
    switch (event) {
        case EK_ADMOB_VIDEO_REWARDED:
            g_admob_wrapper.user_rewarded = true;
            break;
        case EK_ADMOB_VIDEO_FAILED:
        case EK_ADMOB_VIDEO_CLOSED:
            admob_wrapper_rewarded_ad_end(NULL);
            break;
        case EK_ADMOB_VIDEO_LOADED:
            if (g_admob_wrapper.on_rewarded_ad_completed) {
                ek_admob_show_rewarded_ad();
            }
            break;
        case EK_ADMOB_INTERSTITIAL_CLOSED:
            admob_wrapper_interstitial_end(NULL);
            break;
        default:
            break;
    }
}

void admob_wrapper_show_interstitial(void(* callback)(void)) {
    auph_mute_push();
    g_admob_wrapper.on_interstitial_completed = callback;
    g_admob_wrapper.interstitial_active = true;
#if EK_ADMOB_SIMULATOR
    ek_set_timeout((ek_timer_callback) {admob_wrapper_interstitial_end}, 3);
#else
    if (g_admob_wrapper.supported) {
        ek_admob_show_interstitial_ad();
    }
    else {
        admob_wrapper_interstitial_end(NULL);
    }
#endif
}

void admob_wrapper_show_rewarded_ad(void(* callback)(bool)) {
    auph_mute_push();
    g_admob_wrapper.on_rewarded_ad_completed = callback;
    g_admob_wrapper.rewarded_ad_active = true;
#if EK_ADMOB_SIMULATOR
    log_info("rewarded ad begin");
    g_admob_wrapper.user_rewarded = true;
    ek_set_timeout((ek_timer_callback) {admob_wrapper_rewarded_ad_end}, 3);
#else
    g_admob_wrapper.user_rewarded = false;
    if (g_admob_wrapper.supported) {
        ek_admob_show_rewarded_ad();
    }
    else {
        admob_wrapper_rewarded_ad_end(NULL);
    }
#endif
}

void admob_wrapper_setup(bool simulation) {
    UNUSED(simulation);
    g_admob_wrapper.supported = ek_admob_supported();
#if EK_ADMOB_SIMULATOR
    g_admob_wrapper.simulated = simulation;
#else
    if (g_admob_wrapper.supported) {
        ek_admob_set_callback(admob_wrapper_on_event, NULL);
    }
#endif
}
