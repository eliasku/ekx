#pragma once

#include <ecx/ecx.hpp>
#include <ek/admob.h>
#include "Ads.hpp"

namespace ek {

struct AppBoxConfig {
    const char* version_name = "1.0.0";
    const char* version_code = "";
    const char* privacy_policy_url = "https://eliasku-games.web.app/privacy-policy";
    ads_premium_config ads{};
    ek_admob_config admob{nullptr, nullptr, nullptr, EK_ADMOB_CHILD_DIRECTED_UNSPECIFIED};
    const char* billing_key = "";

    // used for sharing results or app for link in the end of the message
    // TODO: could be better to share link object with text description
    const char* app_link_url = "";

    // currently only for manual "rate us" feature
    const char* app_id = "";

    struct {
        const char* google_play = nullptr;
        const char* app_store = nullptr;
    } download_app;
};

class AppBox {
public:
    explicit AppBox(AppBoxConfig info);

    void initDefaultControls(entity_t e);

    void shareWithAppLink(const char* text);

    void rateUs() const;

    void initDownloadAppButtons(entity_t e);

    void initLanguageButton(entity_t e);

public:
    AppBoxConfig config;

private:
};

}

extern ek::AppBox* g_app_box;
void init_app_box(ek::AppBoxConfig config);

