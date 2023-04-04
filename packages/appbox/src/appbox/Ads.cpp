#include "Ads.hpp"

#include <ek/time.h>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/local_storage.h>
#include <ek/admob.h>
#include <ek/admob_wrapper.h>
#include <utility>

#if EK_DEV_TOOLS
const bool useAdMobSimulator = true;
#else
const bool useAdMobSimulator = false;
#endif

static void ads_on_purchase_changed(const purchase_data_t* purchase) {
    if (!g_ads->removed && purchase->state == 0 &&
        strcmp(purchase->product_id, g_ads->config.sku_remove_ads) == 0) {
        g_ads->onRemoveAdsPurchased();
        if (purchase->token && purchase->token[0]) {
            // non-consumable
        }
    }
}

static void ads_on_product_details(const product_details_t* details) {
    if (strcmp(details->product_id, g_ads->config.sku_remove_ads) == 0) {
        g_ads->price = details->price;
        g_ads->on_product_loaded();
    }
}

namespace ek {

Ads::Ads(ads_premium_config config_) :
        config{config_} {
    admob_wrapper_setup(useAdMobSimulator);
    g_billing.on_purchase_changed = &ads_on_purchase_changed;
    g_billing.on_product_details = &ads_on_product_details;

#ifndef NDEBUG
    setRemoveAdsPurchaseCache(false);
#endif

    removed = checkRemoveAdsPurchase();
    if (!removed) {
        g_game_app->dispatcher.listeners.push_back(this);
    }
}

void Ads::onStart() {
    // just wait billing service a little
    // TODO: billing initialized promise
    ek_timer_callback cb;
    cb.action = [](void* userdata) {
        billing_get_purchases();
        const char* product_ids[1] = {(const char*) userdata};
        billing_get_details(product_ids, 1);
    };
    cb.cleanup = nullptr;
    cb.userdata = (void*) config.sku_remove_ads;
    ek_set_timeout(cb, 3);
}

void Ads::purchaseRemoveAds() const {
    billing_purchase(config.sku_remove_ads, "");
}

void Ads::setRemoveAdsPurchaseCache(bool adsRemoved) const {
    ek_ls_set_i(config.key0, adsRemoved ? config.val0 : 0);
    ek_ls_set_i(config.key1, adsRemoved ? config.val1 : 1);
}

bool Ads::checkRemoveAdsPurchase() const {
    return ek_ls_get_i(config.key0, 0) == config.val0 &&
           ek_ls_get_i(config.key1, 0) == config.val1;
}

void Ads::onRemoveAdsPurchased() {
    // disable current ads
    removed = true;

    // save
    setRemoveAdsPurchaseCache(true);

    // dispatch event that ads is removed
    on_removed();
}

static void on_rewarded_ad_end(bool rewarded) {
    if (g_ads->on_rewarded) {
        g_ads->on_rewarded(rewarded);
        g_ads->on_rewarded = nullptr;
    }
}

static void on_interstitial_ad_end(void) {
    if (g_ads->on_game_over_passed) {
        g_ads->on_game_over_passed();
        g_ads->on_game_over_passed = nullptr;
    }
}

void Ads::gameOver(std::function<void()> callback) {
    if (removed) {
        if (callback) {
            callback();
        }
    } else {
        on_game_over_passed = std::move(callback);
        admob_wrapper_show_interstitial(on_interstitial_ad_end);
    }
}

void Ads::showRewardVideo(std::function<void(bool)> callback) {
    on_rewarded = std::move(callback);
    admob_wrapper_show_rewarded_ad(on_rewarded_ad_end);
}

void Ads::cheat_RemoveAds() {
    onRemoveAdsPurchased();
}

bool Ads::hasVideoRewardSupport() const {
    return ek_admob_supported() && ek_admob.config.video;
}

Ads::~Ads() = default;

}

ek::Ads* g_ads = nullptr;

void ads_init(ads_premium_config config) {
    EK_ASSERT(g_ads == nullptr);
    g_ads = new ek::Ads(config);
}
