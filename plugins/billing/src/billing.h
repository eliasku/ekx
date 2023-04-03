#ifndef EK_BILLING_H
#define EK_BILLING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* product_id;
    const char* token;
    const char* payload;
    const char* signature;
    int32_t state;
    int32_t error_code;
    // ios
    const char* receipt;
} purchase_data_t;

typedef struct {
    const char* product_id;
    const char* price;
    const char* currency_code;
} product_details_t;

struct billing_callbacks {
    void (* on_purchase_changed)(const purchase_data_t* purchase);

    void (* on_product_details)(const product_details_t* product_details);
};

extern struct billing_callbacks g_billing;

void billing_setup(const char* developer_key);

void billing_shutdown(void);

void billing_get_purchases(void);

void billing_get_details(const char** product_ids, uint32_t count);

void billing_purchase(const char* sku, const char* payload);

void billing_consume(const char* token);

#ifdef __cplusplus
};
#endif

#endif // EK_BILLING_H
