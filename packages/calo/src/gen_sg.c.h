#include <gen_sg.h>
#include <ek/buf.h>



typedef sg_scene_info_t* sg_scene_info_array_t;

typedef string_hash_t* stringhash32_array_t;

typedef vec2_t* vec2_array_t;

typedef sg_easing_t* sg_easing_array_t;

typedef sg_movie_frame_t* sg_movie_frame_array_t;

typedef sg_movie_layer_t* sg_movie_layer_array_t;

typedef sg_movie_t* sg_movie_opt_t;

typedef sg_text_layer_t* sg_text_layer_array_t;

typedef sg_dynamic_text_t* sg_dynamic_text_opt_t;

typedef struct sg_node_data_* sg_node_data_struct_array_t;

typedef uint16_t* u16_array_t;

typedef model3d_vertex_t* model3d_vertex_array_t;

typedef sprite_info_t* sprite_info_array_t;

typedef atlas_page_info_t* atlas_page_info_array_t;

typedef bmfont_glyph_t* bmfont_glyph_array_t;

typedef bmfont_entry_t* bmfont_entry_array_t;


sg_scene_info_array_t read_stream_sg_scene_info_array(calo_reader_t* r);

void write_stream_sg_scene_info_array(calo_writer_t* w, sg_scene_info_array_t v);

stringhash32_array_t read_stream_stringhash32_array(calo_reader_t* r);

void write_stream_stringhash32_array(calo_writer_t* w, stringhash32_array_t v);

vec2_array_t read_stream_vec2_array(calo_reader_t* r);

void write_stream_vec2_array(calo_writer_t* w, vec2_array_t v);

sg_easing_array_t read_stream_sg_easing_array(calo_reader_t* r);

void write_stream_sg_easing_array(calo_writer_t* w, sg_easing_array_t v);

sg_movie_frame_array_t read_stream_sg_movie_frame_array(calo_reader_t* r);

void write_stream_sg_movie_frame_array(calo_writer_t* w, sg_movie_frame_array_t v);

sg_movie_layer_array_t read_stream_sg_movie_layer_array(calo_reader_t* r);

void write_stream_sg_movie_layer_array(calo_writer_t* w, sg_movie_layer_array_t v);

sg_movie_opt_t read_stream_sg_movie_opt(calo_reader_t* r);

void write_stream_sg_movie_opt(calo_writer_t* w, sg_movie_opt_t v);

sg_text_layer_array_t read_stream_sg_text_layer_array(calo_reader_t* r);

void write_stream_sg_text_layer_array(calo_writer_t* w, sg_text_layer_array_t v);

sg_dynamic_text_opt_t read_stream_sg_dynamic_text_opt(calo_reader_t* r);

void write_stream_sg_dynamic_text_opt(calo_writer_t* w, sg_dynamic_text_opt_t v);

sg_node_data_struct_array_t read_stream_sg_node_data_struct_array(calo_reader_t* r);

void write_stream_sg_node_data_struct_array(calo_writer_t* w, sg_node_data_struct_array_t v);

u16_array_t read_stream_u16_array(calo_reader_t* r);

void write_stream_u16_array(calo_writer_t* w, u16_array_t v);

model3d_vertex_array_t read_stream_model3d_vertex_array(calo_reader_t* r);

void write_stream_model3d_vertex_array(calo_writer_t* w, model3d_vertex_array_t v);

sprite_info_array_t read_stream_sprite_info_array(calo_reader_t* r);

void write_stream_sprite_info_array(calo_writer_t* w, sprite_info_array_t v);

atlas_page_info_array_t read_stream_atlas_page_info_array(calo_reader_t* r);

void write_stream_atlas_page_info_array(calo_writer_t* w, atlas_page_info_array_t v);

bmfont_glyph_array_t read_stream_bmfont_glyph_array(calo_reader_t* r);

void write_stream_bmfont_glyph_array(calo_writer_t* w, bmfont_glyph_array_t v);

bmfont_entry_array_t read_stream_bmfont_entry_array(calo_reader_t* r);

void write_stream_bmfont_entry_array(calo_writer_t* w, bmfont_entry_array_t v);

sg_frame_script_t read_stream_sg_frame_script(calo_reader_t* r) {
    sg_frame_script_t val;
	val.frame = read_i32(r);
	val.code = read_stream_string(r);
    return val;
}

void write_stream_sg_frame_script(calo_writer_t* w, sg_frame_script_t v) {
	write_i32(w, v.frame);
	write_stream_string(w, v.code);
}

sg_frame_label_t read_stream_sg_frame_label(calo_reader_t* r) {
    sg_frame_label_t val;
	val.frame = read_i32(r);
	val.name = read_stream_string(r);
    return val;
}

void write_stream_sg_frame_label(calo_writer_t* w, sg_frame_label_t v) {
	write_i32(w, v.frame);
	write_stream_string(w, v.name);
}

sg_scene_info_t read_stream_sg_scene_info(calo_reader_t* r) {
    sg_scene_info_t val;
	val.name = read_u32(r);
	val.linkage = read_u32(r);
    return val;
}

void write_stream_sg_scene_info(calo_writer_t* w, sg_scene_info_t v) {
	write_u32(w, v.name);
	write_u32(w, v.linkage);
}

sg_scene_info_array_t read_stream_sg_scene_info_array(calo_reader_t* r) {
    sg_scene_info_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_stream_sg_scene_info(r);
        }
    }
    return val;
}

void write_stream_sg_scene_info_array(calo_writer_t* w, sg_scene_info_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_stream_sg_scene_info(w, v[i]);
        }
    }
}

stringhash32_array_t read_stream_stringhash32_array(calo_reader_t* r) {
    stringhash32_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_u32(r);
        }
    }
    return val;
}

void write_stream_stringhash32_array(calo_writer_t* w, stringhash32_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_u32(w, v[i]);
        }
    }
}

sg_file_t read_stream_sg_file(calo_reader_t* r) {
    sg_file_t val;
	val.scenes = read_stream_stringhash32_array(r);
	val.linkages = read_stream_sg_scene_info_array(r);
	val.library = read_stream_sg_node_data_struct_array(r);
    return val;
}

void write_stream_sg_file(calo_writer_t* w, sg_file_t v) {
	write_stream_stringhash32_array(w, v.scenes);
	write_stream_sg_scene_info_array(w, v.linkages);
	write_stream_sg_node_data_struct_array(w, v.library);
}

sg_keyframe_transform_t read_stream_sg_keyframe_transform(calo_reader_t* r) {
    sg_keyframe_transform_t val;
	val.position = read_stream_vec2(r);
	val.scale = read_stream_vec2(r);
	val.skew = read_stream_vec2(r);
	val.pivot = read_stream_vec2(r);
	val.color = read_stream_color2f(r);
    return val;
}

void write_stream_sg_keyframe_transform(calo_writer_t* w, sg_keyframe_transform_t v) {
	write_stream_vec2(w, v.position);
	write_stream_vec2(w, v.scale);
	write_stream_vec2(w, v.skew);
	write_stream_vec2(w, v.pivot);
	write_stream_color2f(w, v.color);
}

vec2_array_t read_stream_vec2_array(calo_reader_t* r) {
    vec2_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_stream_vec2(r);
        }
    }
    return val;
}

void write_stream_vec2_array(calo_writer_t* w, vec2_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_stream_vec2(w, v[i]);
        }
    }
}

sg_easing_t read_stream_sg_easing(calo_reader_t* r) {
    sg_easing_t val;
	val.ease = read_f32(r);
	val.curve = read_stream_vec2_array(r);
	val.attribute = read_u8(r);
    return val;
}

void write_stream_sg_easing(calo_writer_t* w, sg_easing_t v) {
	write_f32(w, v.ease);
	write_stream_vec2_array(w, v.curve);
	write_u8(w, v.attribute);
}

sg_easing_array_t read_stream_sg_easing_array(calo_reader_t* r) {
    sg_easing_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_stream_sg_easing(r);
        }
    }
    return val;
}

void write_stream_sg_easing_array(calo_writer_t* w, sg_easing_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_stream_sg_easing(w, v[i]);
        }
    }
}

sg_movie_frame_t read_stream_sg_movie_frame(calo_reader_t* r) {
    sg_movie_frame_t val;
	val.index = read_i32(r);
	val.duration = read_i32(r);
	val.motion_type = read_i32(r);
	val.easing = read_stream_sg_easing_array(r);
	val.transform = read_stream_sg_keyframe_transform(r);
	val.visible = read_u8(r);
	val.loop_mode = read_i32(r);
	val.first_frame = read_i32(r);
	val.rotate = read_i32(r);
	val.rotate_times = read_i32(r);
    return val;
}

void write_stream_sg_movie_frame(calo_writer_t* w, sg_movie_frame_t v) {
	write_i32(w, v.index);
	write_i32(w, v.duration);
	write_i32(w, v.motion_type);
	write_stream_sg_easing_array(w, v.easing);
	write_stream_sg_keyframe_transform(w, v.transform);
	write_u8(w, v.visible);
	write_i32(w, v.loop_mode);
	write_i32(w, v.first_frame);
	write_i32(w, v.rotate);
	write_i32(w, v.rotate_times);
}

sg_movie_frame_array_t read_stream_sg_movie_frame_array(calo_reader_t* r) {
    sg_movie_frame_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_stream_sg_movie_frame(r);
        }
    }
    return val;
}

void write_stream_sg_movie_frame_array(calo_writer_t* w, sg_movie_frame_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_stream_sg_movie_frame(w, v[i]);
        }
    }
}

sg_movie_layer_t read_stream_sg_movie_layer(calo_reader_t* r) {
    sg_movie_layer_t val;
	val.frames = read_stream_sg_movie_frame_array(r);
    return val;
}

void write_stream_sg_movie_layer(calo_writer_t* w, sg_movie_layer_t v) {
	write_stream_sg_movie_frame_array(w, v.frames);
}

sg_movie_layer_array_t read_stream_sg_movie_layer_array(calo_reader_t* r) {
    sg_movie_layer_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_stream_sg_movie_layer(r);
        }
    }
    return val;
}

void write_stream_sg_movie_layer_array(calo_writer_t* w, sg_movie_layer_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_stream_sg_movie_layer(w, v[i]);
        }
    }
}

sg_movie_t read_stream_sg_movie(calo_reader_t* r) {
    sg_movie_t val;
	val.frames = read_i32(r);
	val.fps = read_f32(r);
	val.layers = read_stream_sg_movie_layer_array(r);
    return val;
}

void write_stream_sg_movie(calo_writer_t* w, sg_movie_t v) {
	write_i32(w, v.frames);
	write_f32(w, v.fps);
	write_stream_sg_movie_layer_array(w, v.layers);
}

sg_movie_opt_t read_stream_sg_movie_opt(calo_reader_t* r) {
    sg_movie_opt_t val;
    {
        bool has = read_u8(r) != 0;
        val = 0;
        if (has) {
            val = (sg_movie_opt_t)malloc(sizeof *val);
            *val = read_stream_sg_movie(r);
        }
    }
    return val;
}

void write_stream_sg_movie_opt(calo_writer_t* w, sg_movie_opt_t v) {
    {
        write_u8(w, v ? 1 : 0);
        if (v) {
            write_stream_sg_movie(w, *v);
        }
    }
}

sg_text_layer_t read_stream_sg_text_layer(calo_reader_t* r) {
    sg_text_layer_t val;
	val.color = read_stream_color(r);
	val.offset = read_stream_vec2(r);
	val.blur_radius = read_f32(r);
	val.blur_iterations = read_u32(r);
	val.strength = read_u32(r);
    return val;
}

void write_stream_sg_text_layer(calo_writer_t* w, sg_text_layer_t v) {
	write_stream_color(w, v.color);
	write_stream_vec2(w, v.offset);
	write_f32(w, v.blur_radius);
	write_u32(w, v.blur_iterations);
	write_u32(w, v.strength);
}

sg_text_layer_array_t read_stream_sg_text_layer_array(calo_reader_t* r) {
    sg_text_layer_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_stream_sg_text_layer(r);
        }
    }
    return val;
}

void write_stream_sg_text_layer_array(calo_writer_t* w, sg_text_layer_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_stream_sg_text_layer(w, v[i]);
        }
    }
}

sg_dynamic_text_t read_stream_sg_dynamic_text(calo_reader_t* r) {
    sg_dynamic_text_t val;
	val.text = read_stream_string(r);
	val.font = read_u32(r);
	val.size = read_f32(r);
	val.alignment = read_stream_vec2(r);
	val.rect = read_stream_rect(r);
	val.line_spacing = read_f32(r);
	val.line_height = read_f32(r);
	val.layers = read_stream_sg_text_layer_array(r);
	val.word_wrap = read_u8(r);
    return val;
}

void write_stream_sg_dynamic_text(calo_writer_t* w, sg_dynamic_text_t v) {
	write_stream_string(w, v.text);
	write_u32(w, v.font);
	write_f32(w, v.size);
	write_stream_vec2(w, v.alignment);
	write_stream_rect(w, v.rect);
	write_f32(w, v.line_spacing);
	write_f32(w, v.line_height);
	write_stream_sg_text_layer_array(w, v.layers);
	write_u8(w, v.word_wrap);
}

sg_dynamic_text_opt_t read_stream_sg_dynamic_text_opt(calo_reader_t* r) {
    sg_dynamic_text_opt_t val;
    {
        bool has = read_u8(r) != 0;
        val = 0;
        if (has) {
            val = (sg_dynamic_text_opt_t)malloc(sizeof *val);
            *val = read_stream_sg_dynamic_text(r);
        }
    }
    return val;
}

void write_stream_sg_dynamic_text_opt(calo_writer_t* w, sg_dynamic_text_opt_t v) {
    {
        write_u8(w, v ? 1 : 0);
        if (v) {
            write_stream_sg_dynamic_text(w, *v);
        }
    }
}

sg_node_data_struct_array_t read_stream_sg_node_data_struct_array(calo_reader_t* r) {
    sg_node_data_struct_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_stream_sg_node_data(r);
        }
    }
    return val;
}

void write_stream_sg_node_data_struct_array(calo_writer_t* w, sg_node_data_struct_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_stream_sg_node_data(w, v[i]);
        }
    }
}

vec4_t read_stream_vec4(calo_reader_t* r) {
    vec4_t val;
	val.x = read_f32(r);
	val.y = read_f32(r);
	val.z = read_f32(r);
	val.w = read_f32(r);
    return val;
}

void write_stream_vec4(calo_writer_t* w, vec4_t v) {
	write_f32(w, v.x);
	write_f32(w, v.y);
	write_f32(w, v.z);
	write_f32(w, v.w);
}

color2f_t read_stream_color2f(calo_reader_t* r) {
    color2f_t val;
	val.scale = read_stream_vec4(r);
	val.offset = read_stream_vec4(r);
    return val;
}

void write_stream_color2f(calo_writer_t* w, color2f_t v) {
	write_stream_vec4(w, v.scale);
	write_stream_vec4(w, v.offset);
}

mat2_t read_stream_mat2(calo_reader_t* r) {
    mat2_t val;
	val.a = read_f32(r);
	val.b = read_f32(r);
	val.c = read_f32(r);
	val.d = read_f32(r);
    return val;
}

void write_stream_mat2(calo_writer_t* w, mat2_t v) {
	write_f32(w, v.a);
	write_f32(w, v.b);
	write_f32(w, v.c);
	write_f32(w, v.d);
}

mat3x2_t read_stream_mat3x2(calo_reader_t* r) {
    mat3x2_t val;
	val.rot = read_stream_mat2(r);
	val.pos = read_stream_vec2(r);
    return val;
}

void write_stream_mat3x2(calo_writer_t* w, mat3x2_t v) {
	write_stream_mat2(w, v.rot);
	write_stream_vec2(w, v.pos);
}

sg_node_data_t read_stream_sg_node_data(calo_reader_t* r) {
    sg_node_data_t val;
	val.matrix = read_stream_mat3x2(r);
	val.color = read_stream_color2f(r);
	val.name = read_u32(r);
	val.library_name = read_u32(r);
	val.sprite = read_u32(r);
	val.flags = (sg_node_flags_t)read_u32(r);
	val.bounding_rect = read_stream_rect(r);
	val.scale_grid = read_stream_rect(r);
	val.children = read_stream_sg_node_data_struct_array(r);
	val.dynamic_text = read_stream_sg_dynamic_text_opt(r);
	val.movie = read_stream_sg_movie_opt(r);
	val.movie_target_id = read_i32(r);
    return val;
}

void write_stream_sg_node_data(calo_writer_t* w, sg_node_data_t v) {
	write_stream_mat3x2(w, v.matrix);
	write_stream_color2f(w, v.color);
	write_u32(w, v.name);
	write_u32(w, v.library_name);
	write_u32(w, v.sprite);
	write_u32(w, v.flags);
	write_stream_rect(w, v.bounding_rect);
	write_stream_rect(w, v.scale_grid);
	write_stream_sg_node_data_struct_array(w, v.children);
	write_stream_sg_dynamic_text_opt(w, v.dynamic_text);
	write_stream_sg_movie_opt(w, v.movie);
	write_i32(w, v.movie_target_id);
}

sg_filter_t read_stream_sg_filter(calo_reader_t* r) {
    sg_filter_t val;
	val.type = (sg_filter_type_t)read_u32(r);
	val.quality = read_u32(r);
	val.color = read_stream_color(r);
	val.blur = read_stream_vec2(r);
	val.offset = read_stream_vec2(r);
    return val;
}

void write_stream_sg_filter(calo_writer_t* w, sg_filter_t v) {
	write_u32(w, v.type);
	write_u32(w, v.quality);
	write_stream_color(w, v.color);
	write_stream_vec2(w, v.blur);
	write_stream_vec2(w, v.offset);
}

u16_array_t read_stream_u16_array(calo_reader_t* r) {
    u16_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_u16(r);
        }
    }
    return val;
}

void write_stream_u16_array(calo_writer_t* w, u16_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_u16(w, v[i]);
        }
    }
}

color_t read_stream_color(calo_reader_t* r) {
    color_t val;
	val.value = read_u32(r);
    return val;
}

void write_stream_color(calo_writer_t* w, color_t v) {
	write_u32(w, v.value);
}

vec2_t read_stream_vec2(calo_reader_t* r) {
    vec2_t val;
	val.x = read_f32(r);
	val.y = read_f32(r);
    return val;
}

void write_stream_vec2(calo_writer_t* w, vec2_t v) {
	write_f32(w, v.x);
	write_f32(w, v.y);
}

vec3_t read_stream_vec3(calo_reader_t* r) {
    vec3_t val;
	val.x = read_f32(r);
	val.y = read_f32(r);
	val.z = read_f32(r);
    return val;
}

void write_stream_vec3(calo_writer_t* w, vec3_t v) {
	write_f32(w, v.x);
	write_f32(w, v.y);
	write_f32(w, v.z);
}

model3d_vertex_t read_stream_model3d_vertex(calo_reader_t* r) {
    model3d_vertex_t val;
	val.position = read_stream_vec3(r);
	val.normal = read_stream_vec3(r);
	val.uv = read_stream_vec2(r);
	val.color = read_stream_color(r);
	val.color2 = read_stream_color(r);
    return val;
}

void write_stream_model3d_vertex(calo_writer_t* w, model3d_vertex_t v) {
	write_stream_vec3(w, v.position);
	write_stream_vec3(w, v.normal);
	write_stream_vec2(w, v.uv);
	write_stream_color(w, v.color);
	write_stream_color(w, v.color2);
}

model3d_vertex_array_t read_stream_model3d_vertex_array(calo_reader_t* r) {
    model3d_vertex_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_stream_model3d_vertex(r);
        }
    }
    return val;
}

void write_stream_model3d_vertex_array(calo_writer_t* w, model3d_vertex_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_stream_model3d_vertex(w, v[i]);
        }
    }
}

model3d_t read_stream_model3d(calo_reader_t* r) {
    model3d_t val;
	val.vertices = read_stream_model3d_vertex_array(r);
	val.indices = read_stream_u16_array(r);
    return val;
}

void write_stream_model3d(calo_writer_t* w, model3d_t v) {
	write_stream_model3d_vertex_array(w, v.vertices);
	write_stream_u16_array(w, v.indices);
}

sprite_info_t read_stream_sprite_info(calo_reader_t* r) {
    sprite_info_t val;
	val.name = read_u32(r);
	val.flags = read_u32(r);
	val.rc = read_stream_rect(r);
	val.uv = read_stream_rect(r);
    return val;
}

void write_stream_sprite_info(calo_writer_t* w, sprite_info_t v) {
	write_u32(w, v.name);
	write_u32(w, v.flags);
	write_stream_rect(w, v.rc);
	write_stream_rect(w, v.uv);
}

sprite_info_array_t read_stream_sprite_info_array(calo_reader_t* r) {
    sprite_info_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_stream_sprite_info(r);
        }
    }
    return val;
}

void write_stream_sprite_info_array(calo_writer_t* w, sprite_info_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_stream_sprite_info(w, v[i]);
        }
    }
}

atlas_page_info_t read_stream_atlas_page_info(calo_reader_t* r) {
    atlas_page_info_t val;
	val.width = read_u16(r);
	val.height = read_u16(r);
	val.image_path = read_stream_string(r);
	val.sprites = read_stream_sprite_info_array(r);
    return val;
}

void write_stream_atlas_page_info(calo_writer_t* w, atlas_page_info_t v) {
	write_u16(w, v.width);
	write_u16(w, v.height);
	write_stream_string(w, v.image_path);
	write_stream_sprite_info_array(w, v.sprites);
}

atlas_page_info_array_t read_stream_atlas_page_info_array(calo_reader_t* r) {
    atlas_page_info_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_stream_atlas_page_info(r);
        }
    }
    return val;
}

void write_stream_atlas_page_info_array(calo_writer_t* w, atlas_page_info_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_stream_atlas_page_info(w, v[i]);
        }
    }
}

atlas_info_t read_stream_atlas_info(calo_reader_t* r) {
    atlas_info_t val;
	val.pages = read_stream_atlas_page_info_array(r);
    return val;
}

void write_stream_atlas_info(calo_writer_t* w, atlas_info_t v) {
	write_stream_atlas_page_info_array(w, v.pages);
}

rect_t read_stream_rect(calo_reader_t* r) {
    rect_t val;
	val.x = read_f32(r);
	val.y = read_f32(r);
	val.w = read_f32(r);
	val.h = read_f32(r);
    return val;
}

void write_stream_rect(calo_writer_t* w, rect_t v) {
	write_f32(w, v.x);
	write_f32(w, v.y);
	write_f32(w, v.w);
	write_f32(w, v.h);
}

bmfont_glyph_t read_stream_bmfont_glyph(calo_reader_t* r) {
    bmfont_glyph_t val;
	val.box = read_stream_rect(r);
	val.advance_x = read_f32(r);
	val.sprite = read_u32(r);
    return val;
}

void write_stream_bmfont_glyph(calo_writer_t* w, bmfont_glyph_t v) {
	write_stream_rect(w, v.box);
	write_f32(w, v.advance_x);
	write_u32(w, v.sprite);
}

bmfont_glyph_array_t read_stream_bmfont_glyph_array(calo_reader_t* r) {
    bmfont_glyph_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_stream_bmfont_glyph(r);
        }
    }
    return val;
}

void write_stream_bmfont_glyph_array(calo_writer_t* w, bmfont_glyph_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_stream_bmfont_glyph(w, v[i]);
        }
    }
}

bmfont_entry_t read_stream_bmfont_entry(calo_reader_t* r) {
    bmfont_entry_t val;
	val.codepoint = read_u32(r);
	val.glyph_index = read_u32(r);
    return val;
}

void write_stream_bmfont_entry(calo_writer_t* w, bmfont_entry_t v) {
	write_u32(w, v.codepoint);
	write_u32(w, v.glyph_index);
}

bmfont_entry_array_t read_stream_bmfont_entry_array(calo_reader_t* r) {
    bmfont_entry_array_t val;
    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = read_stream_bmfont_entry(r);
        }
    }
    return val;
}

void write_stream_bmfont_entry_array(calo_writer_t* w, bmfont_entry_array_t v) {
    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            write_stream_bmfont_entry(w, v[i]);
        }
    }
}

bmfont_header_t read_stream_bmfont_header(calo_reader_t* r) {
    bmfont_header_t val;
	val.base_font_size = read_f32(r);
	val.line_height_multiplier = read_f32(r);
	val.ascender = read_f32(r);
	val.descender = read_f32(r);
    return val;
}

void write_stream_bmfont_header(calo_writer_t* w, bmfont_header_t v) {
	write_f32(w, v.base_font_size);
	write_f32(w, v.line_height_multiplier);
	write_f32(w, v.ascender);
	write_f32(w, v.descender);
}

bmfont_t read_stream_bmfont(calo_reader_t* r) {
    bmfont_t val;
	val.header = read_stream_bmfont_header(r);
	val.dict = read_stream_bmfont_entry_array(r);
	val.glyphs = read_stream_bmfont_glyph_array(r);
    return val;
}

void write_stream_bmfont(calo_writer_t* w, bmfont_t v) {
	write_stream_bmfont_header(w, v.header);
	write_stream_bmfont_entry_array(w, v.dict);
	write_stream_bmfont_glyph_array(w, v.glyphs);
}

image_path_t read_stream_image_path(calo_reader_t* r) {
    image_path_t val;
	read_span(r, val.str, sizeof val.str);
    return val;
}

void write_stream_image_path(calo_writer_t* w, image_path_t v) {
	write_span(w, v.str, sizeof v.str);
}

image_data_t read_stream_image_data(calo_reader_t* r) {
    image_data_t val;
	val.type = (image_data_type_t)read_u32(r);
	val.format_mask = read_u32(r);
	val.images_num = read_u32(r);
	read_span(r, val.images, sizeof val.images);
    return val;
}

void write_stream_image_data(calo_writer_t* w, image_data_t v) {
	write_u32(w, v.type);
	write_u32(w, v.format_mask);
	write_u32(w, v.images_num);
	write_span(w, v.images, sizeof v.images);
}
