#pragma once

#include <billing.hpp>
#include <ek/core/target.h>
#include <ek/scenex/app/GameAppListener.hpp>
#include <functional>

namespace ek {

class AdMobWrapper;

class Ads : public GameAppListener {
public:

    struct Config {
        String skuRemoveAds = "remove_ads";
        String key0 = "ads_key_0";
        int val0 = 1111;
        String key1 = "ads_key_1";
        int val1 = 2222;
    };

    void onStart() override;

    explicit Ads(Config config);
    ~Ads() override;

    void onPurchaseChanged(const billing::PurchaseData& purchase);

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
    Config config_;
    AdMobWrapper* wrapper = nullptr;
private:
    void setRemoveAdsPurchaseCache(bool adsRemoved) const;

    [[nodiscard]] bool checkRemoveAdsPurchase() const;

    void onRemoveAdsPurchased();
};

}

extern ek::Ads* g_ads;
void ads_init(ek::Ads::Config config);