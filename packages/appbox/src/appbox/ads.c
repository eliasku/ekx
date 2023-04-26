#include "ads.h"

#include <ek/time.h>
#include <ek/local_storage.h>
#include <ek/admob.h>
#include <ek/admob_wrapper.h>

#if EK_DEV_TOOLS
const bool use_admob_simulator = true;
#else
const bool use_admob_simulator = false;
#endif

ads_t g_ads;

static void ads_on_purchase_changed(const purchase_data_t* purchase) {
    if (!g_ads.removed && purchase->state == 0 &&
        strcmp(purchase->product_id, g_ads.config.sku_remove_ads) == 0) {
        ads_on_remove_ads_purchased();
        if (purchase->token && purchase->token[0]) {
            // non-consumable
        }
    }
}

static void ads_on_product_details(const product_details_t* details) {
    if (strcmp(details->product_id, g_ads.config.sku_remove_ads) == 0) {
        str_init_c_str(&g_ads.price, details->price);
        sig_emit(&g_ads.on_product_loaded, 0, 0);
    }
}

static void on_game_start_ads_post_init(void* userdata) {
    UNUSED(userdata);
    billing_get_purchases();
    const char* product_ids[1] = {g_ads.config.sku_remove_ads};
    billing_get_details(product_ids, 1);
}

void ads_on_game_start(void) {
    if (!g_ads.removed) {
        // just wait billing service a little
        // TODO: billing initialized promise
        ek_timer_callback cb = INIT_ZERO;
        cb.action = on_game_start_ads_post_init;
        ek_set_timeout(cb, 3);
    }
}

void ads_purchase_remove_ads(void) {
    billing_purchase(g_ads.config.sku_remove_ads, "");
}

void set_remove_ads_purchase_cache(bool ads_removed) {
    ek_ls_set_i(g_ads.config.key0, ads_removed ? g_ads.config.val0 : 0);
    ek_ls_set_i(g_ads.config.key1, ads_removed ? g_ads.config.val1 : 1);
}

bool ads_check_remove_ads_purchase(void) {
    return ek_ls_get_i(g_ads.config.key0, 0) == g_ads.config.val0 &&
           ek_ls_get_i(g_ads.config.key1, 0) == g_ads.config.val1;
}

void ads_on_remove_ads_purchased(void) {
    // disable current ads
    g_ads.removed = true;

    // save
    set_remove_ads_purchase_cache(true);

    // dispatch event that ads is removed
    sig_emit(&g_ads.on_removed, 0, 0);
}

static void on_rewarded_ad_end(bool rewarded) {
    if (g_ads.on_rewarded) {
        g_ads.on_rewarded(rewarded);
        g_ads.on_rewarded = NULL;
    }
}

static void on_interstitial_ad_end(void) {
    if (g_ads.on_game_over_passed) {
        g_ads.on_game_over_passed();
        g_ads.on_game_over_passed = NULL;
    }
}

void ads_game_over(void(* callback)(void)) {
    if (g_ads.removed) {
        if (callback) {
            callback();
        }
    } else {
        g_ads.on_game_over_passed = callback;
        admob_wrapper_show_interstitial(on_interstitial_ad_end);
    }
}

void ads_show_reward_video(void(* callback)(bool)) {
    g_ads.on_rewarded = callback;
    admob_wrapper_show_rewarded_ad(on_rewarded_ad_end);
}

void ads_cheat_remove_ads() {
    ads_on_remove_ads_purchased();
}

bool ads_has_video_reward_support(void) {
    return ek_admob_supported() && ek_admob.config.video;
}

void ads_destroy(void) {
    arr_reset(g_ads.price);
    sig_close(&g_ads.on_removed);
    sig_close(&g_ads.on_product_loaded);
}

void ads_init(ads_premium_config config) {
    g_ads.config = config;
    admob_wrapper_setup(use_admob_simulator);
    g_billing.on_purchase_changed = &ads_on_purchase_changed;
    g_billing.on_product_details = &ads_on_product_details;

#ifndef NDEBUG
    set_remove_ads_purchase_cache(false);
#endif

    g_ads.removed = ads_check_remove_ads_purchase();
}

