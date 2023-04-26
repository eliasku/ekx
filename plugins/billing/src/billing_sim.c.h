#include <ek/time.h>
#include <ek/log.h>

void billing_setup(const char* developerKey) {
    (void) developerKey;
    log_debug("billing setup");
}

void billing_get_purchases(void) {

}

static void billing_get_details_callback(void* userdata) {
    product_details_t details = {(const char*) userdata, "$1.99", "USD"};
    g_billing.on_product_details(&details);
}

static void billing_purchase_callback(void* userdata) {
    g_billing.on_purchase_changed((purchase_data_t*) userdata);
    free(userdata);
}

void billing_get_details(const char** product_ids, uint32_t count) {
    double time = 0.5;
    for (uint32_t i = 0; i < count; ++i) {
        const ek_timer_callback callback = {billing_get_details_callback, NULL, (void*) product_ids[i]};
        ek_set_timeout(callback, time);
        time += 0.5;
    }
}

void billing_purchase(const char* product_id, const char* payload) {
    purchase_data_t* data = (purchase_data_t*) calloc(sizeof(purchase_data_t), 1);
    data->product_id = product_id;
    data->payload = payload;
    data->state = 0;
    const ek_timer_callback callback = {billing_purchase_callback, NULL, (void*) data};
    ek_set_timeout(callback, 2);
}

void billing_consume(const char* token) {
    (void) token;
}
