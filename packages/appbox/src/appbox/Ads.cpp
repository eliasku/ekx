#include "Ads.hpp"

#include <ek/time.h>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/local_storage.h>
#include <ek/admob_wrapper.hpp>
#include <utility>

namespace ek {

#if EK_DEV_TOOLS
const bool useAdMobSimulator = true;
#else
const bool useAdMobSimulator = false;
#endif

void ads_on_purchase_changed(const purchase_data_t* purchase) {
    if (!g_ads->removed && purchase->state == 0 &&
        strcmp(purchase->product_id, g_ads->config.sku_remove_ads) == 0) {
        g_ads->onRemoveAdsPurchased();
        if (purchase->token && purchase->token[0]) {
            // non-consumable
        }
    }
}

void ads_on_product_details(const product_details_t* details) {
    if (strcmp(details->product_id, g_ads->config.sku_remove_ads) == 0) {
        g_ads->price = details->price;
        g_ads->onProductLoaded();
    }
}

Ads::Ads(ads_premium_config config_) :
        config{config_},
        wrapper{AdMobWrapper::create(useAdMobSimulator)} {
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
    cb.action = [](void* sku_) {
        billing_get_purchases();
        const char* sku_list[1] = {(const char*)sku_};
        billing_get_details(sku_list, 1);
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
    onRemoved();
}

void Ads::gameOver(std::function<void()> callback) {
    if (removed) {
        if (callback) {
            callback();
        }
    } else {
        wrapper->showInterstitial(std::move(callback));
    }
}

void Ads::showRewardVideo(std::function<void(bool)> callback) {
    wrapper->showRewardedAd(std::move(callback));
}

void Ads::cheat_RemoveAds() {
    onRemoveAdsPurchased();
}

bool Ads::hasVideoRewardSupport() const {
    return ek_admob_supported() && ek_admob.config.video;
}

bool Ads::isSupported() const {
    return ek_admob_supported();
}

Ads::~Ads() {
    delete wrapper;
}

}

ek::Ads* g_ads = nullptr;
void ads_init(ads_premium_config config) {
    EK_ASSERT(g_ads == nullptr);
    g_ads = new ek::Ads(config);
}
