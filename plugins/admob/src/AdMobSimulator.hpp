#pragma once

#include <ek/admob_wrapper.hpp>
#include <ek/scenex/app/GameAppListener.hpp>
#include <ek/canvas.h>
#include <ek/app.h>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/audio.h>
#include <ek/time.hpp>

namespace ek {

class AdMobSimulator : public AdMobWrapper, public GameAppListener {

    bool activeRewardedAd = false;

public:

    void onRenderFrame() override {
        const bool splash = activeInterstitial || activeRewardedAd;
        const color_t color = activeRewardedAd ? ARGB(0x77FF00FF) : ARGB(0x7700FF00);

        if (splash) {
            canvas_set_empty_image();
            canvas_quad_color(0, 0, ek_app.viewport.width, ek_app.viewport.height, color);
        }
    }

    AdMobSimulator() {
        g_game_app->dispatcher.listeners.push_back(this);
    }

    ~AdMobSimulator() override {
        g_game_app->dispatcher.listeners.remove(this);
    }

    void showInterstitial(std::function<void()> callback) override {
        activeInterstitial = true;
        auph_mute_push();
        ek_set_timeout(ek::timer_func([this, cb = std::move(callback)] {
            activeInterstitial = false;
            auph_mute_pop();
            if (cb) {
                cb();
            }
        }), 3);
    }

    void showRewardedAd(std::function<void(bool)> callback) override {
        activeRewardedAd = true;
        auph_mute_push();
        ek_set_timeout(ek::timer_func([this, cb = std::move(callback)] {
            activeRewardedAd = false;
            auph_mute_pop();
            if (cb) {
                cb(true);
            }
        }), 5);
    }
};

}

