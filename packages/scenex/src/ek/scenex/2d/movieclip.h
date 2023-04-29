#ifndef SCENEX_MOVIECLIP_H
#define SCENEX_MOVIECLIP_H

#include <ecx/ecx.h>
#include <gen_sg.h>
#include <ekx/app/time_layers.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const sg_movie_t* data;
    float time;
    float fps;
    time_layer_t timer;
    bool playing;
} movieclip_t;

typedef struct {
    int32_t key;
} movieclip_target_t;

#define MovieClip ECX_ID(movieclip_t)
#define get_movieclip(e) ECX_GET(movieclip_t,e)
#define add_movieclip(e) ECX_ADD(movieclip_t,e)
#define set_movieclip_target(e, index_key) (ECX_ADD(movieclip_target_t,e)->key = (index_key))
#define get_movieclip_target(e) ECX_GET(movieclip_target_t,e)
extern ECX_DEFINE_TYPE(movieclip_t);
extern ECX_DEFINE_TYPE(movieclip_target_t);

void setup_movieclip(void);

void update_movieclips(void);

void goto_and_stop(entity_t e, float frame);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_MOVIECLIP_H
