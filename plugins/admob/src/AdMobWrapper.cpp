#include <ek/admob_wrapper.hpp>
#include <ek/audio.h>
#include "AdMobNull.hpp"

// implementation
#if defined(__APPLE__)

#include <TargetConditionals.h>

#endif

#if defined(__ANDROID__) || TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#define EK_ADMOB_SIMULATOR 0
#else
#define EK_ADMOB_SIMULATOR 1
#endif

#if EK_ADMOB_SIMULATOR

#include "AdMobSimulator.hpp"

#endif

namespace ek {

AdMobWrapper::AdMobWrapper() {
    ek_admob_set_callback([](void* userdata, auto event) {
        ((AdMobWrapper*)userdata)->onAdmobEvent(event);
    }, this);
}

void AdMobWrapper::showInterstitial(std::function<void()> callback) {
    auph_mute_push();
    interstitialCompletedCallback = std::move(callback);
    activeInterstitial = true;
    ek_admob_show_interstitial_ad();
}

void AdMobWrapper::showRewardedAd(std::function<void(bool)> callback) {
    auph_mute_push();
    rewardedAdCompletedCallback = std::move(callback);
    userRewarded = false;
    ek_admob_show_rewarded_ad();
}

void AdMobWrapper::completeRewardedAd(bool rewarded) {
    if (rewardedAdCompletedCallback) {
        auph_mute_pop();
        rewardedAdCompletedCallback(rewarded);
        rewardedAdCompletedCallback = nullptr;
    }
}

void AdMobWrapper::onAdmobEvent(ek_admob_event_type event) {
    switch (event) {
        case EK_ADMOB_VIDEO_REWARDED:
            userRewarded = true;
            break;
        case EK_ADMOB_VIDEO_FAILED:
        case EK_ADMOB_VIDEO_CLOSED:
            completeRewardedAd(userRewarded);
            break;
        case EK_ADMOB_VIDEO_LOADED:
            if (rewardedAdCompletedCallback) {
                ek_admob_show_rewarded_ad();
            }
            break;
        case EK_ADMOB_INTERSTITIAL_CLOSED:
            if(activeInterstitial) {
                auph_mute_pop();
                activeInterstitial = false;
                if (interstitialCompletedCallback) {
                    interstitialCompletedCallback();
                    interstitialCompletedCallback = nullptr;
                }
            }
            break;
        default:
            break;
    }
}

AdMobWrapper* AdMobWrapper::create(bool devMode) {
    (void) devMode;

#if EK_ADMOB_SIMULATOR
    if (devMode) {
        return new AdMobSimulator();
    }
#endif

    if (ek_admob_supported()) {
        return new AdMobWrapper();
    }
    return new AdMobNull();
}

}
