#ifndef CALO_GEN_GEN_SG_H
#define CALO_GEN_GEN_SG_H

#include <stdint.h>
#include <stdbool.h>

#include <calo.h>
#include <ek/math.h>
#include <ek/hash.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*** types ***/

struct sg_frame_script_;
struct sg_frame_label_;
struct sg_scene_info_;
struct sg_file_;
struct sg_keyframe_transform_;
struct sg_easing_;
struct sg_movie_frame_;
struct sg_movie_layer_;
struct sg_movie_;
struct sg_text_layer_;
struct sg_dynamic_text_;
struct sg_node_data_;
struct sg_filter_;
struct model3d_vertex_;
struct model3d_;
struct sprite_info_;
struct atlas_page_info_;
struct atlas_info_;
struct bmfont_glyph_;
struct bmfont_entry_;
struct bmfont_header_;
struct bmfont_;
struct image_path_;
struct image_data_;
typedef struct sg_frame_script_ {
	int32_t frame;
	const char* code;
} sg_frame_script_t;

typedef struct sg_frame_label_ {
	int32_t frame;
	const char* name;
} sg_frame_label_t;

typedef struct sg_scene_info_ {
	string_hash_t name;
	string_hash_t linkage;
} sg_scene_info_t;

typedef struct sg_file_ {
	string_hash_t* scenes;
	struct sg_scene_info_* linkages;
	struct sg_node_data_* library;
} sg_file_t;

typedef struct sg_keyframe_transform_ {
	vec2_t position;
	vec2_t scale;
	vec2_t skew;
	vec2_t pivot;
	color2f_t color;
} sg_keyframe_transform_t;

typedef struct sg_easing_ {
	float ease;
	vec2_t* curve;
	uint8_t attribute;
} sg_easing_t;

typedef struct sg_movie_frame_ {
	int32_t index;
	int32_t duration;
	int32_t motion_type;
	struct sg_easing_* easing;
	sg_keyframe_transform_t transform;
	bool visible;
	int32_t loop_mode;
	int32_t first_frame;
	int32_t rotate;
	int32_t rotate_times;
} sg_movie_frame_t;

typedef struct sg_movie_layer_ {
	struct sg_movie_frame_* frames;
	struct sg_node_data_ ** targets;
} sg_movie_layer_t;

typedef struct sg_movie_ {
	int32_t frames;
	float fps;
	struct sg_movie_layer_* layers;
} sg_movie_t;

typedef struct sg_text_layer_ {
	color_t color;
	vec2_t offset;
	float blur_radius;
	uint32_t blur_iterations;
	uint32_t strength;
} sg_text_layer_t;

typedef struct sg_dynamic_text_ {
	const char* text;
	string_hash_t font;
	float size;
	vec2_t alignment;
	rect_t rect;
	float line_spacing;
	float line_height;
	struct sg_text_layer_* layers;
	bool word_wrap;
} sg_dynamic_text_t;

typedef enum {
   SG_NODE_BUTTON = 1,
   SG_NODE_TOUCHABLE = 2,
   SG_NODE_VISIBLE = 4,
   SG_NODE_SCISSORS_ENABLED = 8,
   SG_NODE_HIT_AREA_ENABLED = 16,
   SG_NODE_BOUNDS_ENABLED = 32
} sg_node_flags_t;

typedef struct sg_node_data_ {
	mat3x2_t matrix;
	color2f_t color;
	string_hash_t name;
	string_hash_t library_name;
	string_hash_t sprite;
	sg_node_flags_t flags;
	rect_t bounding_rect;
	rect_t scale_grid;
	struct sg_node_data_* children;
	struct sg_dynamic_text_* dynamic_text;
	struct sg_movie_* movie;
	struct sg_frame_label_* labels;
	struct sg_frame_script_* scripts;
	int32_t movie_target_id;
} sg_node_data_t;

typedef enum {
   SG_FILTER_NONE = 0,
   SG_FILTER_SHADOW = 1,
   SG_FILTER_GLOW = 2
} sg_filter_type_t;

typedef struct sg_filter_ {
	sg_filter_type_t type;
	uint32_t quality;
	color_t color;
	vec2_t blur;
	vec2_t offset;
} sg_filter_t;

typedef struct model3d_vertex_ {
	vec3_t position;
	vec3_t normal;
	vec2_t uv;
	color_t color;
	color_t color2;
} model3d_vertex_t;

typedef struct model3d_ {
	struct model3d_vertex_* vertices;
	uint16_t* indices;
} model3d_t;

typedef struct sprite_info_ {
	string_hash_t name;
	uint32_t flags;
	rect_t rc;
	rect_t uv;
} sprite_info_t;

typedef struct atlas_page_info_ {
	uint16_t width;
	uint16_t height;
	const char* image_path;
	struct sprite_info_* sprites;
} atlas_page_info_t;

typedef struct atlas_info_ {
	struct atlas_page_info_* pages;
} atlas_info_t;

typedef struct bmfont_glyph_ {
	rect_t box;
	float advance_x;
	string_hash_t sprite;
} bmfont_glyph_t;

typedef struct bmfont_entry_ {
	uint32_t codepoint;
	uint32_t glyph_index;
} bmfont_entry_t;

typedef struct bmfont_header_ {
	float base_font_size;
	float line_height_multiplier;
	float ascender;
	float descender;
} bmfont_header_t;

typedef struct bmfont_ {
	bmfont_header_t header;
	struct bmfont_entry_* dict;
	struct bmfont_glyph_* glyphs;
} bmfont_t;

typedef struct image_path_ {
	char str[128];
} image_path_t;

typedef enum {
   IMAGE_DATA_NORMAL = 0,
   IMAGE_DATA_CUBE_MAP = 1
} image_data_type_t;

typedef struct image_data_ {
	image_data_type_t type;
	uint32_t format_mask;
	uint32_t images_num;
	struct image_path_ images[6];
} image_data_t;


/*** functions ***/

sg_frame_script_t read_stream_sg_frame_script(calo_reader_t* r);

void write_stream_sg_frame_script(calo_writer_t* w, sg_frame_script_t v);

sg_frame_label_t read_stream_sg_frame_label(calo_reader_t* r);

void write_stream_sg_frame_label(calo_writer_t* w, sg_frame_label_t v);

sg_scene_info_t read_stream_sg_scene_info(calo_reader_t* r);

void write_stream_sg_scene_info(calo_writer_t* w, sg_scene_info_t v);

sg_file_t read_stream_sg_file(calo_reader_t* r);

void write_stream_sg_file(calo_writer_t* w, sg_file_t v);

sg_keyframe_transform_t read_stream_sg_keyframe_transform(calo_reader_t* r);

void write_stream_sg_keyframe_transform(calo_writer_t* w, sg_keyframe_transform_t v);

sg_easing_t read_stream_sg_easing(calo_reader_t* r);

void write_stream_sg_easing(calo_writer_t* w, sg_easing_t v);

sg_movie_frame_t read_stream_sg_movie_frame(calo_reader_t* r);

void write_stream_sg_movie_frame(calo_writer_t* w, sg_movie_frame_t v);

sg_movie_layer_t read_stream_sg_movie_layer(calo_reader_t* r);

void write_stream_sg_movie_layer(calo_writer_t* w, sg_movie_layer_t v);

sg_movie_t read_stream_sg_movie(calo_reader_t* r);

void write_stream_sg_movie(calo_writer_t* w, sg_movie_t v);

sg_text_layer_t read_stream_sg_text_layer(calo_reader_t* r);

void write_stream_sg_text_layer(calo_writer_t* w, sg_text_layer_t v);

sg_dynamic_text_t read_stream_sg_dynamic_text(calo_reader_t* r);

void write_stream_sg_dynamic_text(calo_writer_t* w, sg_dynamic_text_t v);

vec4_t read_stream_vec4(calo_reader_t* r);

void write_stream_vec4(calo_writer_t* w, vec4_t v);

color2f_t read_stream_color2f(calo_reader_t* r);

void write_stream_color2f(calo_writer_t* w, color2f_t v);

mat2_t read_stream_mat2(calo_reader_t* r);

void write_stream_mat2(calo_writer_t* w, mat2_t v);

mat3x2_t read_stream_mat3x2(calo_reader_t* r);

void write_stream_mat3x2(calo_writer_t* w, mat3x2_t v);

sg_node_data_t read_stream_sg_node_data(calo_reader_t* r);

void write_stream_sg_node_data(calo_writer_t* w, sg_node_data_t v);

sg_filter_t read_stream_sg_filter(calo_reader_t* r);

void write_stream_sg_filter(calo_writer_t* w, sg_filter_t v);

color_t read_stream_color(calo_reader_t* r);

void write_stream_color(calo_writer_t* w, color_t v);

vec2_t read_stream_vec2(calo_reader_t* r);

void write_stream_vec2(calo_writer_t* w, vec2_t v);

vec3_t read_stream_vec3(calo_reader_t* r);

void write_stream_vec3(calo_writer_t* w, vec3_t v);

model3d_vertex_t read_stream_model3d_vertex(calo_reader_t* r);

void write_stream_model3d_vertex(calo_writer_t* w, model3d_vertex_t v);

model3d_t read_stream_model3d(calo_reader_t* r);

void write_stream_model3d(calo_writer_t* w, model3d_t v);

sprite_info_t read_stream_sprite_info(calo_reader_t* r);

void write_stream_sprite_info(calo_writer_t* w, sprite_info_t v);

atlas_page_info_t read_stream_atlas_page_info(calo_reader_t* r);

void write_stream_atlas_page_info(calo_writer_t* w, atlas_page_info_t v);

atlas_info_t read_stream_atlas_info(calo_reader_t* r);

void write_stream_atlas_info(calo_writer_t* w, atlas_info_t v);

rect_t read_stream_rect(calo_reader_t* r);

void write_stream_rect(calo_writer_t* w, rect_t v);

bmfont_glyph_t read_stream_bmfont_glyph(calo_reader_t* r);

void write_stream_bmfont_glyph(calo_writer_t* w, bmfont_glyph_t v);

bmfont_entry_t read_stream_bmfont_entry(calo_reader_t* r);

void write_stream_bmfont_entry(calo_writer_t* w, bmfont_entry_t v);

bmfont_header_t read_stream_bmfont_header(calo_reader_t* r);

void write_stream_bmfont_header(calo_writer_t* w, bmfont_header_t v);

bmfont_t read_stream_bmfont(calo_reader_t* r);

void write_stream_bmfont(calo_writer_t* w, bmfont_t v);

image_path_t read_stream_image_path(calo_reader_t* r);

void write_stream_image_path(calo_writer_t* w, image_path_t v);

image_data_t read_stream_image_data(calo_reader_t* r);

void write_stream_image_data(calo_writer_t* w, image_data_t v);

// functions

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CALO_GEN_GEN_SG_H
