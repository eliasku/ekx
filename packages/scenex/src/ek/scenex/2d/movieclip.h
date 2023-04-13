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
    TimeLayer timer;
    bool playing;
}  movieclip_t;

typedef struct {
    int32_t key;
} movieclip_target_index_t;

extern ecx_component_type MovieClip;
extern ecx_component_type MovieClipTarget;
void MovieClip_setup(void);
void MovieClip_update(void);
#define get_movieclip(e) ((movieclip_t*)get_component(&MovieClip, e))
#define add_movieclip(e) ((movieclip_t*)add_component(&MovieClip, e))
#define set_movieclip_target(e, index_key) (((movieclip_target_index_t*)add_component(&MovieClipTarget, (e)))->key = (index_key))
#define get_movieclip_target(e) ((movieclip_target_index_t*)get_component(&MovieClipTarget, e))

void goto_and_stop(entity_t e, float frame);

#ifdef __cplusplus
}
#endif

#endif // SCENEX_MOVIECLIP_H
