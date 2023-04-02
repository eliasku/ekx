#include "AppBox.hpp"

#include <ek/ds/String.hpp>
#include <ek/app.h>
#include <ek/scenex/base/node.h>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/base/interactiv.h>
#include <billing.hpp>
#include <ekx/app/audio_manager.h>
#include <ek/game_services.h>
#include <ekx/app/localization.h>
#include "Ads.hpp"
#include "ek/scenex/base/NodeEvents.hpp"
#include "ek/local_storage.h"

namespace ek {

AppBox::AppBox(AppBoxConfig config_) :
        config{config_} {

    // unlock abort()

    billing::initialize(config.billing_key);
    ek_admob_init(config.admob);
    ads_init(config.ads);
    ek_game_services_init();

    // initialize translations
    // TODO: wtf
    lang_name_t lang = {};
    lang_name_t default_lang = {};
    default_lang.str[0] = 'e';
    default_lang.str[1] = 'n';
    default_lang.str[2] = 0;
    int n = ek_ls_get_s("selected_lang", lang.str, sizeof(lang_name_t));
    EK_ASSERT(sizeof(lang_name_t) <= sizeof(ek_app.lang));
    if (n < 2) {
        memcpy(lang.str, ek_app.lang, sizeof(lang_name_t));
    }
    // trim to 2-wide code
    lang.str[2] = 0;
    if (lang.str[0] == 0) {
        lang = default_lang;
    }
    if (!set_language(lang)) {
        set_language(default_lang);
    }
}

void set_state_on_off(entity_t e, bool enabled) {
    auto on = find(e, H("state_on"));
    auto off = find(e, H("state_off"));
    set_visible(on, enabled);
    set_visible(off, !enabled);
}

void AppBox::initDefaultControls(entity_t e) {
    {
        // VERSION
        entity_t e_version = find(e, H("version"));
        if (e_version.id) {
#ifndef NDEBUG
            set_text_f(e_version, "%s #%s_d", config.version_name, config.version_code);
            interactive_add(e_version);
            ecs::add<Button>(e_version);
            ecs::add<NodeEventHandler>(e_version).on(BUTTON_EVENT_CLICK, [](const NodeEventData& ) {
                // force crash
                volatile uint8_t* invalid_ptr = (uint8_t*)NULL;
                *invalid_ptr = 0;
            });
#else
            set_text_f(e_version, "%s #%s", config.version_name, config.version_code);
#endif
        }
    }
    {
        // PRIVACY POLICY
        entity_t e_pp = find(e, H("privacy_policy"));
        if (e_pp.id) {
            entity_t lbl = find(e_pp, H("label"));
            if (lbl.id) {
                auto* txt = ecs::try_get<Text2D>(lbl);
                if (txt) {
                    txt->hitFullBounds = true;
                }
            }
            interactive_add(e_pp);
            ecs::add<Button>(e_pp);
            ecs::add<NodeEventHandler>(e_pp).on(BUTTON_EVENT_CLICK, [](const NodeEventData& ) {
                ek_app_open_url(g_app_box->config.privacy_policy_url);
            });
        }
    }

    // Purchases
    {
        entity_t btn = find(e, H("remove_ads"));
        if (btn.id) {
            if (g_ads->removed) {
                set_visible(btn, false);
            } else {
                g_ads->onRemoved << [btn] {
                    if (is_entity(btn)) {
                        set_visible(btn, false);
                    }
                };
                ecs::add<NodeEventHandler>(btn).on(BUTTON_EVENT_CLICK, [](const NodeEventData& ) {
                    g_ads->purchaseRemoveAds();
                });
            }
        }
    }
    {
        entity_t btn = find(e, H("restore_purchases"));
        if (btn.id) {
            ecs::add<NodeEventHandler>(btn).on(BUTTON_EVENT_CLICK, [](const NodeEventData& ) {
                billing::getPurchases();
            });
        }
    }

    // Settings
    {
        {
            entity_t btn = find(e, H("sound"));
            if (btn.id) {
                ecs::add<NodeEventHandler>(btn).on(BUTTON_EVENT_CLICK, [](const NodeEventData& event) {
                    set_state_on_off(event.source, audio_toggle_pref(AUDIO_PREF_SOUND));
                });
                set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_SOUND);
            }
        }
        {
            entity_t btn = find(e, H("music"));
            if (btn.id) {
                ecs::add<NodeEventHandler>(btn).on(BUTTON_EVENT_CLICK, [](const NodeEventData& event) {
                    set_state_on_off(event.source, audio_toggle_pref(AUDIO_PREF_MUSIC));
                });
                set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_MUSIC);
            }
        }
        {
            entity_t btn = find(e, H("vibro"));
            if (btn.id) {
                ecs::add<NodeEventHandler>(btn).on(BUTTON_EVENT_CLICK, [](const NodeEventData& event) {
                    set_state_on_off(event.source, audio_toggle_pref(AUDIO_PREF_VIBRO));
                    if (g_audio.prefs & AUDIO_PREF_VIBRO) {
                        vibrate(50);
                    }
                });
                set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_VIBRO);
            }
        }

        initLanguageButton(e);
    }
}

void AppBox::shareWithAppLink(const char* text) {
    String msg = text;
    msg += " ";
    msg += config.app_link_url;
    ek_app_share(msg.c_str());
}

void AppBox::rateUs() const {
#ifdef __ANDROID__
    char buf[1024];
    ek_snprintf(buf, 1024, "market://details?id=%s", config.app_id);
    ek_app_open_url(buf);
#endif // __ANDROID__

#ifdef __APPLE__
    char buf[1024];
    ek_snprintf(buf, 1024, "itms-apps://itunes.apple.com/us/app/apple-store/id%s?mt=8&action=write-review",
                config.app_id);
    ek_app_open_url(buf);
#endif // __APPLE__
}

/// download app feature

void wrap_button(entity_t e, string_hash_t tag, const char* link) {
    entity_t x = find(e, tag);
    if (link && *link) {
        ecs::add<Button>(x);
        ecs::add<NodeEventHandler>(x).on(BUTTON_EVENT_CLICK, [link](const NodeEventData& ) {
            ek_app_open_url(link);
        });
    } else {
        set_visible(e, false);
    }
}

void AppBox::initDownloadAppButtons(entity_t) {
//    auto banner = sg_create("gfx", "cross_banner");
//    setName(banner, "banner");
//    layout_wrapper{banner}.aligned(0.5f, 0.0f, 1.0f, 0.0f);
//
//    wrap_button(banner, "google_play", config.downloadApp.googlePlay);
//    wrap_button(banner, "app_store", config.downloadApp.appStore);
//
//    append(e, banner);
}

void AppBox::initLanguageButton(entity_t e) {
    entity_t btn = find(e, H("language"));
    if (btn.id) {
        ecs::add<NodeEventHandler>(btn).on(BUTTON_EVENT_CLICK, [](const NodeEventData& ) {
            uint32_t index = s_localization.lang_index;
            uint32_t num = s_localization.lang_num;
            // check if langs are available
            if (index < num && num != 0 &&
                set_language_index((index + 1) % num)) {
                const char* lang_name = s_localization.languages[s_localization.lang_index].name.str;
                ek_ls_set_s("selected_lang", lang_name);
            }
        });
    }
}

}

ek::AppBox* g_app_box = nullptr;

void init_app_box(ek::AppBoxConfig config) {
    EK_ASSERT(!g_app_box);
    g_app_box = new ek::AppBox(config);
}
