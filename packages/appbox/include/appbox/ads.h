#ifndef APPBOX_ADS_H
#define APPBOX_ADS_H

#include <billing.h>
#include <ek/core/target.h>
#include <ek/sigslot.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* sku_remove_ads;
    const char* key0;
    int val0;
    const char* key1;
    int val1;
} ads_premium_config;

typedef struct {
    ads_premium_config config;
    signal_t on_removed;
    signal_t on_product_loaded;

    void (* on_game_over_passed)(void);

    void (* on_rewarded)(bool);

    void* price;
    bool removed;
} ads_t;

void ads_purchase_remove_ads(void);

// TODO: rename, try start commercial break
void ads_game_over(void(* callback)(void));

void ads_show_reward_video(void(* callback)(bool));

void ads_cheat_remove_ads(void);

bool ads_has_video_reward_support(void);

void ads_on_remove_ads_purchased(void);

void ads_set_remove_ads_purchase_cache(bool ads_removed);

bool ads_check_remove_ads_purchase(void);

extern ads_t g_ads;
void ads_init(ads_premium_config config);
// call when app is loaded
void ads_on_game_start(void);

#ifdef __cplusplus
}
#endif

#endif // APPBOX_ADS_H
