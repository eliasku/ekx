#include <appbox/ads.h>
#include <appbox/appbox.h>

#include <billing.h>
#include <ek/app.h>
#include <ek/game_services.h>
#include <ek/local_storage.h>
#include <ek/print.h>
#include <ek/scenex/2d/button.h>
#include <ek/scenex/2d/text2d.h>
#include <ek/scenex/base/interactive.h>
#include <ek/scenex/base/node.h>
#include <ek/scenex/base/node_events.h>
#include <ekx/app/audio_manager.h>
#include <ekx/app/localization.h>

appbox_config_t appbox_config_default(void) {
    appbox_config_t config = {0};
    config.version_name = "1.0.0";
    config.version_code = "";
    config.privacy_policy_url = "https://eliasku-games.web.app/privacy-policy";
    config.ads.sku_remove_ads = "remove_ads";
    config.ads.key0 = "ads_key_0";
    config.ads.val0 = 1111;
    config.ads.key1 = "ads_key_1";
    config.ads.val1 = 2222;
    config.admob.child_directed = EK_ADMOB_CHILD_DIRECTED_UNSPECIFIED;
    config.billing_key = "";
    // used for sharing results or app for link in the end of the message
    // TODO: could be better to share link object with text description
    config.app_link_url = "";
    // currently only for manual "rate us" feature
    config.app_id = "";

    return config;
}


appbox_context_t g_appbox;

void appbox_setup(appbox_config_t config) {
    g_appbox.config = config;

    // unlock abort()

    billing_setup(config.billing_key);
    ek_admob_init(config.admob);
    ads_init(config.ads);
    ek_game_services_init();

    // initialize translations
    // TODO: wtf
    lang_name_t lang = {{0}};
    lang_name_t default_lang = {{'e', 'n', 0}};
    int n = ek_ls_get_s("selected_lang", lang.str, sizeof(lang_name_t));
    EK_ASSERT(sizeof(lang_name_t) <= sizeof(ek_app.lang));
    if (n < 2) {
        memcpy(lang.str, ek_app.lang, sizeof(lang_name_t));
    }
    // trim to 2-wide code
    lang.str[2] = 0;
    if (!lang.str[0]) {
        lang = default_lang;
    }
    if (!set_language(lang)) {
        set_language(default_lang);
    }
}

void appbox_on_game_start(void) {
    ads_on_game_start();
}

static void set_state_on_off(entity_t e, bool enabled) {
    entity_t on = find(e, H("state_on"));
    entity_t off = find(e, H("state_off"));
    set_visible(on, enabled);
    set_visible(off, !enabled);
}

#ifndef NDEBUG
static void appbox_on_crash(const node_event_t* event) {
    UNUSED(event);
    // force crash
    volatile uint8_t* invalid_ptr = (uint8_t*)NULL;
    *invalid_ptr = 0;
}
#endif

static void appbox_on_privacy_policy(const node_event_t* event) {
    UNUSED(event);
    ek_app_open_url(g_appbox.config.privacy_policy_url);
}

static void appbox_on_vibro(const node_event_t* event) {
    set_state_on_off(event->source, audio_toggle_pref(AUDIO_PREF_VIBRO));
    if (g_audio.prefs & AUDIO_PREF_VIBRO) {
        vibrate(50);
    }
}

static void appbox_on_sound(const node_event_t* event) {
    set_state_on_off(event->source, audio_toggle_pref(AUDIO_PREF_SOUND));
}

static void appbox_on_music(const node_event_t* event) {
    set_state_on_off(event->source, audio_toggle_pref(AUDIO_PREF_MUSIC));
}

static void appbox_on_restore_purchases(const node_event_t* event) {
    UNUSED(event);
    billing_get_purchases();
}

static void appbox_on_remove_ads(const node_event_t* event) {
    UNUSED(event);
    ads_purchase_remove_ads();
}

static void appbox_on_ads_removed(signal_slot_t* slot, void* event) {
    UNUSED(event);
    entity_t btn = entity_id(slot->context.u64);
    if (is_entity(btn)) {
        set_visible(btn, false);
    }
}

void appbox_init_default_controls(entity_t e) {
    {
        // VERSION
        entity_t e_version = find(e, H("version"));
        if (e_version.id) {
#ifndef NDEBUG
            set_text_f(e_version, "%s #%s_d", g_appbox.config.version_name, g_appbox.config.version_code);
            add_interactive(e_version);
            add_button(e_version);
            add_node_event_listener(e_version, BUTTON_EVENT_CLICK, appbox_on_crash);
#else
            set_text_f(e_version, "%s #%s", g_appbox.config.version_name, g_appbox.config.version_code);
#endif
        }
    }
    {
        // PRIVACY POLICY
        entity_t e_pp = find(e, H("privacy_policy"));
        if (e_pp.id) {
            add_interactive(e_pp);
            add_button(e_pp);
            add_node_event_listener(e_pp, BUTTON_EVENT_CLICK, appbox_on_privacy_policy);
        }
    }

    // Purchases
    {
        entity_t btn = find(e, H("remove_ads"));
        if (btn.id) {
            if (g_ads.removed) {
                set_visible(btn, false);
            } else {
                signal_slot_t* slot = sig_connect(&g_ads.on_removed, appbox_on_ads_removed, 0);
                slot->context.u64 = btn.id;
                slot->once = true;
                add_node_event_listener(btn, BUTTON_EVENT_CLICK, appbox_on_remove_ads);
            }
        }
    }
    {
        entity_t btn = find(e, H("restore_purchases"));
        if (btn.id) {
            add_node_event_listener(btn, BUTTON_EVENT_CLICK, appbox_on_restore_purchases);
        }
    }

    // Settings
    {
        entity_t btn = find(e, H("sound"));
        if (btn.id) {
            add_node_event_listener(btn, BUTTON_EVENT_CLICK, appbox_on_sound);
            set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_SOUND);
        }
    }
    {
        entity_t btn = find(e, H("music"));
        if (btn.id) {
            add_node_event_listener(btn, BUTTON_EVENT_CLICK, appbox_on_music);
            set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_MUSIC);
        }
    }
    {
        entity_t btn = find(e, H("vibro"));
        if (btn.id) {
            add_node_event_listener(btn, BUTTON_EVENT_CLICK, appbox_on_vibro);
            set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_VIBRO);
        }
    }

    appbox_init_language_button(e);
}

void appbox_update_default_controls(entity_t e) {
    if (is_entity(e)) {
        {
            entity_t btn = find(e, H("remove_ads"));
            if (btn.id) {
                set_visible(btn, !g_ads.removed);
            }
        }
        {
            entity_t btn = find(e, H("sound"));
            if (btn.id) {
                set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_SOUND);
            }
        }
        {
            entity_t btn = find(e, H("music"));
            if (btn.id) {
                set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_MUSIC);
            }
        }
        {
            entity_t btn = find(e, H("vibro"));
            if (btn.id) {
                set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_VIBRO);
            }
        }
    }
}

void appbox_share_with_app_link(const char* text) {
    char buf[1024];
    ek_snprintf(buf, sizeof buf, "%s %s", text, g_appbox.config.app_link_url);
    ek_app_share(buf);
}

void appbox_rate_us(void) {
#ifdef __ANDROID__
    char buf[1024];
    ek_snprintf(buf, 1024, "market://details?id=%s", g_appbox.config.app_id);
    ek_app_open_url(buf);
#endif // __ANDROID__

#ifdef __APPLE__
    char buf[1024];
    ek_snprintf(buf, 1024, "itms-apps://itunes.apple.com/us/app/apple-store/id%s?mt=8&action=write-review",
                g_appbox.config.app_id);
    ek_app_open_url(buf);
#endif // __APPLE__
}

/// download app feature

//static void wrap_button(entity_t e, string_hash_t tag, const char* link) {
//    using namespace ek;
//    entity_t x = find(e, tag);
//    if (link && *link) {
//        add_button(x);
//        add_node_events(x)->signal.add(BUTTON_EVENT_CLICK, [link](const node_event_t& ) {
//            ek_app_open_url(link);
//        });
//    } else {
//        set_visible(e, false);
//    }
//}

void appbox_init_download_app_buttons(entity_t e) {
    UNUSED(e);
    //    auto banner = sg_create("gfx", "cross_banner");
    //    setName(banner, "banner");
    //    layout_wrapper{banner}.aligned(0.5f, 0.0f, 1.0f, 0.0f);
    //
    //    wrap_button(banner, "google_play", config.downloadApp.googlePlay);
    //    wrap_button(banner, "app_store", config.downloadApp.appStore);
    //
    //    append(e, banner);
}

static void appbox_on_language_button(const node_event_t* event) {
    UNUSED(event);
    uint32_t index = s_localization.lang_index;
    uint32_t num = s_localization.lang_num;
    // check if languages are available
    if (index < num && num != 0 &&
        set_language_index((index + 1) % num)) {
        const char* lang_name = s_localization.languages[s_localization.lang_index].name.str;
        ek_ls_set_s("selected_lang", lang_name);
    }
}

void appbox_init_language_button(entity_t e) {
    entity_t btn = find(e, H("language"));
    if (btn.id) {
        add_node_event_listener(btn, BUTTON_EVENT_CLICK, appbox_on_language_button);
    }
}
