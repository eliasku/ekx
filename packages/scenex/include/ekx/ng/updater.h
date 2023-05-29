#ifndef EKX_NG_UPDATER_H
#define EKX_NG_UPDATER_H

#include <sce/util/time_layers.h>
#include <ecx/ecx.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* updater_callback_t)(entity_t e, time_layer_t time_layer);

typedef struct {
    time_layer_t time_layer;
    updater_callback_t callback;
} updater_t;

void setup_updater(void);

void process_updaters(void);

void set_updater(entity_t e, updater_callback_t callback);

#define del_updater(e) remove_component(&ECX_ID(updater_t),e);

#ifdef __cplusplus
}
#endif

#endif // EKX_NG_UPDATER_H
