#pragma once

#include "GameAppListener.hpp"
#include <ek/ds/Array.hpp>

namespace ek {

class GameAppDispatcher : public GameAppListener {
public:
    PodArray<GameAppListener*> listeners{};

    ~GameAppDispatcher() override = default;

    void onBeforeFrameBegin() override {
        for (auto* listener : listeners) {
            if(listener) listener->onBeforeFrameBegin();
        }
    }

    void onPreload() override {
        for (auto* listener : listeners) {
            if(listener) listener->onPreload();
        }
    }

    void onPreRender() override {
        for (auto* listener : listeners) {
            if(listener) listener->onPreRender();
        }
    }

    void onRenderOverlay() override {
        for (auto* listener : listeners) {
            if(listener) listener->onRenderOverlay();
        }
    }

    void onRenderFrame() override {
        for (auto* listener : listeners) {
            if(listener) listener->onRenderFrame();
        }
    }

    void onUpdate() override {
        for (auto* listener : listeners) {
            if(listener) listener->onUpdate();
        }
    }

    void onStart() override {
        for (auto* listener : listeners) {
            if(listener) listener->onStart();
        }
    }

    void onEvent(const ek_app_event& event) override {
        for (auto* listener : listeners) {
            if(listener) listener->onEvent(event);
        }
    }

    void onPostFrame() override {
        for (auto* listener : listeners) {
            if(listener) listener->onPostFrame();
        }
    }
};

}