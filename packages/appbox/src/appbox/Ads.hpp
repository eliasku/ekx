#pragma once

#include <billing.h>
#include <ek/core/target.h>
#include <ek/util/Signal.hpp>
#include <ek/ds/String.hpp>
#include <functional>

typedef struct {
    const char* sku_remove_ads;
    const char* key0;
    int val0;
    const char* key1;
    int val1;
} ads_premium_config;

namespace ek {

struct Ads {
    explicit Ads(ads_premium_config config_);

    ~Ads();

    void purchaseRemoveAds() const;

    // TODO: rename, try start commercial break
    void gameOver(std::function<void()> callback);

    void showRewardVideo(std::function<void(bool)> callback);

    Signal<> on_removed{};
    Signal<> on_product_loaded{};
    std::function<void()> on_game_over_passed{};
    std::function<void(bool)> on_rewarded{};

    void cheat_RemoveAds();

    [[nodiscard]] bool hasVideoRewardSupport() const;

    String price{};
    bool removed = false;
    ads_premium_config config;

    void onRemoveAdsPurchased();
private:
    void setRemoveAdsPurchaseCache(bool adsRemoved) const;

    [[nodiscard]] bool checkRemoveAdsPurchase() const;
};

}

extern ek::Ads* g_ads;
void ads_init(ads_premium_config config);
// call when app is loaded
void ads_on_game_start(void);

