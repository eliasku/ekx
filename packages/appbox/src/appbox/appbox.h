#ifndef APPBOX_H
#define APPBOX_H

#include <ecx/ecx.h>
#include <ek/admob.h>
#include "Ads.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* version_name;
    const char* version_code;
    const char* privacy_policy_url;
    ads_premium_config ads;
    ek_admob_config admob;
    const char* billing_key;

    // used for sharing results or app for link in the end of the message
    // TODO: could be better to share link object with text description
    const char* app_link_url;

    // currently only for manual "rate us" feature
    const char* app_id;

    struct {
        const char* google_play;
        const char* app_store;
    } download_app;
} appbox_config_t;

appbox_config_t appbox_config_default(void);

typedef struct {
    appbox_config_t config;
} appbox_context_t;

void appbox_init_default_controls(entity_t e);

void appbox_share_with_app_link(const char* text);

void appbox_rate_us(void);

void appbox_init_download_app_buttons(entity_t e);

void appbox_init_language_button(entity_t e);

void appbox_setup(appbox_config_t config);

extern appbox_context_t g_appbox;

#ifdef __cplusplus
};
#endif

#endif // APPBOX_H
