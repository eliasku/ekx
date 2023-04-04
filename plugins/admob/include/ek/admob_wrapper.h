#ifndef ADMOB_WRAPPER_H
#define ADMOB_WRAPPER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct admob_wrapper {
    void (* on_interstitial_completed)(void);

    void (* on_rewarded_ad_completed)(bool);

    bool simulated;
    bool supported;
    bool interstitial_active;
    bool rewarded_ad_active;
    bool user_rewarded;
};

extern struct admob_wrapper g_admob_wrapper;

void admob_wrapper_setup(bool simulation);

void admob_wrapper_show_interstitial(void(* callback)(void));

void admob_wrapper_show_rewarded_ad(void(* callback)(bool));

#ifdef __cplusplus
};
#endif

#endif // ADMOB_WRAPPER_H
