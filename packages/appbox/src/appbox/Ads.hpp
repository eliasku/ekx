#pragma once

#include <billing.h>
#include <ek/core/target.h>
#include <ek/util/Signal.hpp>
#include <ek/ds/String.hpp>
#include <ek/scenex/app/GameAppListener.hpp>
#include <functional>

typedef struct ads_premium_config {
    const char* sku_remove_ads = "remove_ads";
    const char* key0 = "ads_key_0";
    int val0 = 1111;
    const char* key1 = "ads_key_1";
    int val1 = 2222;
} ads_premium_config;

namespace ek {

class AdMobWrapper;

class Ads : public GameAppListener {
public:
    void onStart() override;

    explicit Ads(ads_premium_config config_);
    ~Ads() override;

    void purchaseRemoveAds() const;

    // TODO: rename, try start commercial break
    void gameOver(std::function<void()> callback);

    void showRewardVideo(std::function<void(bool)> callback);

    Signal<> onRemoved{};
    Signal<> onProductLoaded{};

    void cheat_RemoveAds();

    [[nodiscard]] bool hasVideoRewardSupport() const;

    [[nodiscard]] bool isSupported() const;

    String price{};
    bool removed = false;
    ads_premium_config config;
    AdMobWrapper* wrapper = nullptr;

    void onRemoveAdsPurchased();
private:
    void setRemoveAdsPurchaseCache(bool adsRemoved) const;

    [[nodiscard]] bool checkRemoveAdsPurchase() const;


};

}

extern ek::Ads* g_ads;
void ads_init(ads_premium_config config);