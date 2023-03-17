import {
	read_stream_u8,
	read_stream_u16,
	read_stream_u32,
	read_stream_i8,
	read_stream_i16,
	read_stream_i32,
	read_stream_f32,
	read_stream_f64,
	read_stream_string,
	read_stream_static_string,
	read_stream_boolean,
	write_stream_u8,
	write_stream_u16,
	write_stream_u32,
	write_stream_i8,
	write_stream_i16,
	write_stream_i32,
	write_stream_f32,
	write_stream_f64,
	write_stream_string,
	write_stream_static_string,
	write_stream_boolean,
} from "./calo.js";

import type {
	char,
	u8,
	u16,
	u32,
	i8,
	i16,
	i32,
	f32,
	f64,
	StringHash32,
	Vec2,
	Vec3,
	Vec4,
	Rect,
	Color,
	Color2F,
	Mat2,
	Mat3x2,
	StaticString,
	StaticArray,
	Writer,
	Reader,
} from "./calo.js";

export interface sg_frame_script {
	frame: i32;
	code: string;
}

export const read_stream_sg_frame_script = (r: Reader): sg_frame_script => {
    const val = {} as sg_frame_script;
	val.frame = read_stream_i32(r);
	val.code = read_stream_string(r);
    return val;
}

export const write_stream_sg_frame_script = (w: Writer, v: sg_frame_script): void => {
	write_stream_i32(w, v.frame);
	write_stream_string(w, v.code);
}

export interface sg_frame_label {
	frame: i32;
	name: string;
}

export const read_stream_sg_frame_label = (r: Reader): sg_frame_label => {
    const val = {} as sg_frame_label;
	val.frame = read_stream_i32(r);
	val.name = read_stream_string(r);
    return val;
}

export const write_stream_sg_frame_label = (w: Writer, v: sg_frame_label): void => {
	write_stream_i32(w, v.frame);
	write_stream_string(w, v.name);
}

export interface sg_scene_info {
	name: StringHash32;
	linkage: StringHash32;
}

export const read_stream_sg_scene_info = (r: Reader): sg_scene_info => {
    const val = {} as sg_scene_info;
	val.name = read_stream_u32(r);
	val.linkage = read_stream_u32(r);
    return val;
}

export const write_stream_sg_scene_info = (w: Writer, v: sg_scene_info): void => {
	write_stream_u32(w, v.name);
	write_stream_u32(w, v.linkage);
}

export interface sg_file {
	scenes: StringHash32[];
	linkages: sg_scene_info[];
	library: sg_node_data[];
}

export const read_stream_sg_file = (r: Reader): sg_file => {
    const val = {} as sg_file;
	val.scenes = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.scenes[i] = read_stream_u32(r);
	val.linkages = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.linkages[i] = read_stream_sg_scene_info(r);
	val.library = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.library[i] = read_stream_sg_node_data(r);
    return val;
}

export const write_stream_sg_file = (w: Writer, v: sg_file): void => {
	write_stream_u32(w, v.scenes.length);
	for(let i = 0; i < v.scenes.length; ++i) write_stream_u32(w, v.scenes[i]);
	write_stream_u32(w, v.linkages.length);
	for(let i = 0; i < v.linkages.length; ++i) write_stream_sg_scene_info(w, v.linkages[i]);
	write_stream_u32(w, v.library.length);
	for(let i = 0; i < v.library.length; ++i) write_stream_sg_node_data(w, v.library[i]);
}

export interface sg_keyframe_transform {
	position: Vec2;
	scale: Vec2;
	skew: Vec2;
	pivot: Vec2;
	color: Color2F;
}

export const read_stream_sg_keyframe_transform = (r: Reader): sg_keyframe_transform => {
    const val = {} as sg_keyframe_transform;
	val.position = read_stream_Vec2(r);
	val.scale = read_stream_Vec2(r);
	val.skew = read_stream_Vec2(r);
	val.pivot = read_stream_Vec2(r);
	val.color = read_stream_Color2F(r);
    return val;
}

export const write_stream_sg_keyframe_transform = (w: Writer, v: sg_keyframe_transform): void => {
	write_stream_Vec2(w, v.position);
	write_stream_Vec2(w, v.scale);
	write_stream_Vec2(w, v.skew);
	write_stream_Vec2(w, v.pivot);
	write_stream_Color2F(w, v.color);
}

export interface sg_easing {
	ease: f32;
	curve: Vec2[];
	attribute: u8;
}

export const read_stream_sg_easing = (r: Reader): sg_easing => {
    const val = {} as sg_easing;
	val.ease = read_stream_f32(r);
	val.curve = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.curve[i] = read_stream_Vec2(r);
	val.attribute = read_stream_u8(r);
    return val;
}

export const write_stream_sg_easing = (w: Writer, v: sg_easing): void => {
	write_stream_f32(w, v.ease);
	write_stream_u32(w, v.curve.length);
	for(let i = 0; i < v.curve.length; ++i) write_stream_Vec2(w, v.curve[i]);
	write_stream_u8(w, v.attribute);
}

export interface sg_movie_frame {
	index: i32;
	duration: i32;
	motion_type: i32;
	easing: sg_easing[];
	transform: sg_keyframe_transform;
	visible: boolean;
	loop_mode: i32;
	first_frame: i32;
	rotate: i32;
	rotate_times: i32;
}

export const read_stream_sg_movie_frame = (r: Reader): sg_movie_frame => {
    const val = {} as sg_movie_frame;
	val.index = read_stream_i32(r);
	val.duration = read_stream_i32(r);
	val.motion_type = read_stream_i32(r);
	val.easing = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.easing[i] = read_stream_sg_easing(r);
	val.transform = read_stream_sg_keyframe_transform(r);
	val.visible = read_stream_boolean(r);
	val.loop_mode = read_stream_i32(r);
	val.first_frame = read_stream_i32(r);
	val.rotate = read_stream_i32(r);
	val.rotate_times = read_stream_i32(r);
    return val;
}

export const write_stream_sg_movie_frame = (w: Writer, v: sg_movie_frame): void => {
	write_stream_i32(w, v.index);
	write_stream_i32(w, v.duration);
	write_stream_i32(w, v.motion_type);
	write_stream_u32(w, v.easing.length);
	for(let i = 0; i < v.easing.length; ++i) write_stream_sg_easing(w, v.easing[i]);
	write_stream_sg_keyframe_transform(w, v.transform);
	write_stream_boolean(w, v.visible);
	write_stream_i32(w, v.loop_mode);
	write_stream_i32(w, v.first_frame);
	write_stream_i32(w, v.rotate);
	write_stream_i32(w, v.rotate_times);
}

export interface sg_movie_layer {
	frames: sg_movie_frame[];
	targets: any;
}

export const read_stream_sg_movie_layer = (r: Reader): sg_movie_layer => {
    const val = {} as sg_movie_layer;
	val.frames = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.frames[i] = read_stream_sg_movie_frame(r);
    return val;
}

export const write_stream_sg_movie_layer = (w: Writer, v: sg_movie_layer): void => {
	write_stream_u32(w, v.frames.length);
	for(let i = 0; i < v.frames.length; ++i) write_stream_sg_movie_frame(w, v.frames[i]);
}

export interface sg_movie {
	frames: i32;
	fps: f32;
	layers: sg_movie_layer[];
}

export const read_stream_sg_movie = (r: Reader): sg_movie => {
    const val = {} as sg_movie;
	val.frames = read_stream_i32(r);
	val.fps = read_stream_f32(r);
	val.layers = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.layers[i] = read_stream_sg_movie_layer(r);
    return val;
}

export const write_stream_sg_movie = (w: Writer, v: sg_movie): void => {
	write_stream_i32(w, v.frames);
	write_stream_f32(w, v.fps);
	write_stream_u32(w, v.layers.length);
	for(let i = 0; i < v.layers.length; ++i) write_stream_sg_movie_layer(w, v.layers[i]);
}

export interface sg_text_layer {
	color: Color;
	offset: Vec2;
	blur_radius: f32;
	blur_iterations: u32;
	strength: u32;
}

export const read_stream_sg_text_layer = (r: Reader): sg_text_layer => {
    const val = {} as sg_text_layer;
	val.color = read_stream_Color(r);
	val.offset = read_stream_Vec2(r);
	val.blur_radius = read_stream_f32(r);
	val.blur_iterations = read_stream_u32(r);
	val.strength = read_stream_u32(r);
    return val;
}

export const write_stream_sg_text_layer = (w: Writer, v: sg_text_layer): void => {
	write_stream_Color(w, v.color);
	write_stream_Vec2(w, v.offset);
	write_stream_f32(w, v.blur_radius);
	write_stream_u32(w, v.blur_iterations);
	write_stream_u32(w, v.strength);
}

export interface sg_dynamic_text {
	text: string;
	font: StringHash32;
	size: f32;
	alignment: Vec2;
	rect: Rect;
	line_spacing: f32;
	line_height: f32;
	layers: sg_text_layer[];
	word_wrap: boolean;
}

export const read_stream_sg_dynamic_text = (r: Reader): sg_dynamic_text => {
    const val = {} as sg_dynamic_text;
	val.text = read_stream_string(r);
	val.font = read_stream_u32(r);
	val.size = read_stream_f32(r);
	val.alignment = read_stream_Vec2(r);
	val.rect = read_stream_Rect(r);
	val.line_spacing = read_stream_f32(r);
	val.line_height = read_stream_f32(r);
	val.layers = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.layers[i] = read_stream_sg_text_layer(r);
	val.word_wrap = read_stream_boolean(r);
    return val;
}

export const write_stream_sg_dynamic_text = (w: Writer, v: sg_dynamic_text): void => {
	write_stream_string(w, v.text);
	write_stream_u32(w, v.font);
	write_stream_f32(w, v.size);
	write_stream_Vec2(w, v.alignment);
	write_stream_Rect(w, v.rect);
	write_stream_f32(w, v.line_spacing);
	write_stream_f32(w, v.line_height);
	write_stream_u32(w, v.layers.length);
	for(let i = 0; i < v.layers.length; ++i) write_stream_sg_text_layer(w, v.layers[i]);
	write_stream_boolean(w, v.word_wrap);
}

export const enum sg_node_flags {
   SG_NODE_BUTTON = 1,
   SG_NODE_TOUCHABLE = 2,
   SG_NODE_VISIBLE = 4,
   SG_NODE_SCISSORS_ENABLED = 8,
   SG_NODE_HIT_AREA_ENABLED = 16,
   SG_NODE_BOUNDS_ENABLED = 32,
}

export const read_stream_Vec4 = (r: Reader): Vec4 => {
    const val = {} as Vec4;
	val.x = read_stream_f32(r);
	val.y = read_stream_f32(r);
	val.z = read_stream_f32(r);
	val.w = read_stream_f32(r);
    return val;
}

export const write_stream_Vec4 = (w: Writer, v: Vec4): void => {
	write_stream_f32(w, v.x);
	write_stream_f32(w, v.y);
	write_stream_f32(w, v.z);
	write_stream_f32(w, v.w);
}

export const read_stream_Color2F = (r: Reader): Color2F => {
    const val = {} as Color2F;
	val.scale = read_stream_Vec4(r);
	val.offset = read_stream_Vec4(r);
    return val;
}

export const write_stream_Color2F = (w: Writer, v: Color2F): void => {
	write_stream_Vec4(w, v.scale);
	write_stream_Vec4(w, v.offset);
}

export const read_stream_Mat2 = (r: Reader): Mat2 => {
    const val = {} as Mat2;
	val.a = read_stream_f32(r);
	val.b = read_stream_f32(r);
	val.c = read_stream_f32(r);
	val.d = read_stream_f32(r);
    return val;
}

export const write_stream_Mat2 = (w: Writer, v: Mat2): void => {
	write_stream_f32(w, v.a);
	write_stream_f32(w, v.b);
	write_stream_f32(w, v.c);
	write_stream_f32(w, v.d);
}

export const read_stream_Mat3x2 = (r: Reader): Mat3x2 => {
    const val = {} as Mat3x2;
	val.rot = read_stream_Mat2(r);
	val.pos = read_stream_Vec2(r);
    return val;
}

export const write_stream_Mat3x2 = (w: Writer, v: Mat3x2): void => {
	write_stream_Mat2(w, v.rot);
	write_stream_Vec2(w, v.pos);
}

export interface sg_node_data {
	matrix: Mat3x2;
	color: Color2F;
	name: StringHash32;
	library_name: StringHash32;
	sprite: StringHash32;
	flags: sg_node_flags;
	bounding_rect: Rect;
	scale_grid: Rect;
	children: sg_node_data[];
	dynamic_text?: sg_dynamic_text;
	movie?: sg_movie;
	labels: sg_frame_label[];
	scripts: sg_frame_script[];
	movie_target_id: i32;
}

export const read_stream_sg_node_data = (r: Reader): sg_node_data => {
    const val = {} as sg_node_data;
	val.matrix = read_stream_Mat3x2(r);
	val.color = read_stream_Color2F(r);
	val.name = read_stream_u32(r);
	val.library_name = read_stream_u32(r);
	val.sprite = read_stream_u32(r);
	val.flags = read_stream_u32(r) as sg_node_flags;
	val.bounding_rect = read_stream_Rect(r);
	val.scale_grid = read_stream_Rect(r);
	val.children = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.children[i] = read_stream_sg_node_data(r);
	{ const has = read_stream_u8(r); if (has) val.dynamic_text = read_stream_sg_dynamic_text(r); }
	{ const has = read_stream_u8(r); if (has) val.movie = read_stream_sg_movie(r); }
	val.movie_target_id = read_stream_i32(r);
    return val;
}

export const write_stream_sg_node_data = (w: Writer, v: sg_node_data): void => {
	write_stream_Mat3x2(w, v.matrix);
	write_stream_Color2F(w, v.color);
	write_stream_u32(w, v.name);
	write_stream_u32(w, v.library_name);
	write_stream_u32(w, v.sprite);
	write_stream_u32(w, v.flags);
	write_stream_Rect(w, v.bounding_rect);
	write_stream_Rect(w, v.scale_grid);
	write_stream_u32(w, v.children.length);
	for(let i = 0; i < v.children.length; ++i) write_stream_sg_node_data(w, v.children[i]);
	write_stream_u8(w, v.dynamic_text != null ? 1 : 0);
	if(v.dynamic_text != null) write_stream_sg_dynamic_text(w, v.dynamic_text);
	write_stream_u8(w, v.movie != null ? 1 : 0);
	if(v.movie != null) write_stream_sg_movie(w, v.movie);
	write_stream_i32(w, v.movie_target_id);
}

export const enum sg_filter_type {
   SG_FILTER_NONE = 0,
   SG_FILTER_SHADOW = 1,
   SG_FILTER_GLOW = 2,
}

export interface sg_filter {
	type: sg_filter_type;
	quality: u32;
	color: Color;
	blur: Vec2;
	offset: Vec2;
}

export const read_stream_sg_filter = (r: Reader): sg_filter => {
    const val = {} as sg_filter;
	val.type = read_stream_u32(r) as sg_filter_type;
	val.quality = read_stream_u32(r);
	val.color = read_stream_Color(r);
	val.blur = read_stream_Vec2(r);
	val.offset = read_stream_Vec2(r);
    return val;
}

export const write_stream_sg_filter = (w: Writer, v: sg_filter): void => {
	write_stream_u32(w, v.type);
	write_stream_u32(w, v.quality);
	write_stream_Color(w, v.color);
	write_stream_Vec2(w, v.blur);
	write_stream_Vec2(w, v.offset);
}

export const read_stream_Color = (r: Reader): Color => {
    const val = {} as Color;
	val.value = read_stream_u32(r);
    return val;
}

export const write_stream_Color = (w: Writer, v: Color): void => {
	write_stream_u32(w, v.value);
}

export const read_stream_Vec2 = (r: Reader): Vec2 => {
    const val = {} as Vec2;
	val.x = read_stream_f32(r);
	val.y = read_stream_f32(r);
    return val;
}

export const write_stream_Vec2 = (w: Writer, v: Vec2): void => {
	write_stream_f32(w, v.x);
	write_stream_f32(w, v.y);
}

export const read_stream_Vec3 = (r: Reader): Vec3 => {
    const val = {} as Vec3;
	val.x = read_stream_f32(r);
	val.y = read_stream_f32(r);
	val.z = read_stream_f32(r);
    return val;
}

export const write_stream_Vec3 = (w: Writer, v: Vec3): void => {
	write_stream_f32(w, v.x);
	write_stream_f32(w, v.y);
	write_stream_f32(w, v.z);
}

export interface model3d_vertex {
	position: Vec3;
	normal: Vec3;
	uv: Vec2;
	color: Color;
	color2: Color;
}

export const read_stream_model3d_vertex = (r: Reader): model3d_vertex => {
    const val = {} as model3d_vertex;
	val.position = read_stream_Vec3(r);
	val.normal = read_stream_Vec3(r);
	val.uv = read_stream_Vec2(r);
	val.color = read_stream_Color(r);
	val.color2 = read_stream_Color(r);
    return val;
}

export const write_stream_model3d_vertex = (w: Writer, v: model3d_vertex): void => {
	write_stream_Vec3(w, v.position);
	write_stream_Vec3(w, v.normal);
	write_stream_Vec2(w, v.uv);
	write_stream_Color(w, v.color);
	write_stream_Color(w, v.color2);
}

export interface model3d {
	vertices: model3d_vertex[];
	indices: u16[];
}

export const read_stream_model3d = (r: Reader): model3d => {
    const val = {} as model3d;
	val.vertices = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.vertices[i] = read_stream_model3d_vertex(r);
	val.indices = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.indices[i] = read_stream_u16(r);
    return val;
}

export const write_stream_model3d = (w: Writer, v: model3d): void => {
	write_stream_u32(w, v.vertices.length);
	for(let i = 0; i < v.vertices.length; ++i) write_stream_model3d_vertex(w, v.vertices[i]);
	write_stream_u32(w, v.indices.length);
	for(let i = 0; i < v.indices.length; ++i) write_stream_u16(w, v.indices[i]);
}

export interface sprite_info {
	name: StringHash32;
	flags: u32;
	rc: Rect;
	uv: Rect;
}

export const read_stream_sprite_info = (r: Reader): sprite_info => {
    const val = {} as sprite_info;
	val.name = read_stream_u32(r);
	val.flags = read_stream_u32(r);
	val.rc = read_stream_Rect(r);
	val.uv = read_stream_Rect(r);
    return val;
}

export const write_stream_sprite_info = (w: Writer, v: sprite_info): void => {
	write_stream_u32(w, v.name);
	write_stream_u32(w, v.flags);
	write_stream_Rect(w, v.rc);
	write_stream_Rect(w, v.uv);
}

export interface atlas_page_info {
	width: u16;
	height: u16;
	image_path: string;
	sprites: sprite_info[];
}

export const read_stream_atlas_page_info = (r: Reader): atlas_page_info => {
    const val = {} as atlas_page_info;
	val.width = read_stream_u16(r);
	val.height = read_stream_u16(r);
	val.image_path = read_stream_string(r);
	val.sprites = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.sprites[i] = read_stream_sprite_info(r);
    return val;
}

export const write_stream_atlas_page_info = (w: Writer, v: atlas_page_info): void => {
	write_stream_u16(w, v.width);
	write_stream_u16(w, v.height);
	write_stream_string(w, v.image_path);
	write_stream_u32(w, v.sprites.length);
	for(let i = 0; i < v.sprites.length; ++i) write_stream_sprite_info(w, v.sprites[i]);
}

export interface atlas_info {
	pages: atlas_page_info[];
}

export const read_stream_atlas_info = (r: Reader): atlas_info => {
    const val = {} as atlas_info;
	val.pages = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.pages[i] = read_stream_atlas_page_info(r);
    return val;
}

export const write_stream_atlas_info = (w: Writer, v: atlas_info): void => {
	write_stream_u32(w, v.pages.length);
	for(let i = 0; i < v.pages.length; ++i) write_stream_atlas_page_info(w, v.pages[i]);
}

export const read_stream_Rect = (r: Reader): Rect => {
    const val = {} as Rect;
	val.x = read_stream_f32(r);
	val.y = read_stream_f32(r);
	val.w = read_stream_f32(r);
	val.h = read_stream_f32(r);
    return val;
}

export const write_stream_Rect = (w: Writer, v: Rect): void => {
	write_stream_f32(w, v.x);
	write_stream_f32(w, v.y);
	write_stream_f32(w, v.w);
	write_stream_f32(w, v.h);
}

export interface bmfont_glyph {
	box: Rect;
	advance_x: f32;
	sprite: StringHash32;
}

export const read_stream_bmfont_glyph = (r: Reader): bmfont_glyph => {
    const val = {} as bmfont_glyph;
	val.box = read_stream_Rect(r);
	val.advance_x = read_stream_f32(r);
	val.sprite = read_stream_u32(r);
    return val;
}

export const write_stream_bmfont_glyph = (w: Writer, v: bmfont_glyph): void => {
	write_stream_Rect(w, v.box);
	write_stream_f32(w, v.advance_x);
	write_stream_u32(w, v.sprite);
}

export interface bmfont_entry {
	codepoint: u32;
	glyph_index: u32;
}

export const read_stream_bmfont_entry = (r: Reader): bmfont_entry => {
    const val = {} as bmfont_entry;
	val.codepoint = read_stream_u32(r);
	val.glyph_index = read_stream_u32(r);
    return val;
}

export const write_stream_bmfont_entry = (w: Writer, v: bmfont_entry): void => {
	write_stream_u32(w, v.codepoint);
	write_stream_u32(w, v.glyph_index);
}

export interface bmfont_header {
	base_font_size: f32;
	line_height_multiplier: f32;
	ascender: f32;
	descender: f32;
}

export const read_stream_bmfont_header = (r: Reader): bmfont_header => {
    const val = {} as bmfont_header;
	val.base_font_size = read_stream_f32(r);
	val.line_height_multiplier = read_stream_f32(r);
	val.ascender = read_stream_f32(r);
	val.descender = read_stream_f32(r);
    return val;
}

export const write_stream_bmfont_header = (w: Writer, v: bmfont_header): void => {
	write_stream_f32(w, v.base_font_size);
	write_stream_f32(w, v.line_height_multiplier);
	write_stream_f32(w, v.ascender);
	write_stream_f32(w, v.descender);
}

export interface bmfont {
	header: bmfont_header;
	dict: bmfont_entry[];
	glyphs: bmfont_glyph[];
}

export const read_stream_bmfont = (r: Reader): bmfont => {
    const val = {} as bmfont;
	val.header = read_stream_bmfont_header(r);
	val.dict = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.dict[i] = read_stream_bmfont_entry(r);
	val.glyphs = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.glyphs[i] = read_stream_bmfont_glyph(r);
    return val;
}

export const write_stream_bmfont = (w: Writer, v: bmfont): void => {
	write_stream_bmfont_header(w, v.header);
	write_stream_u32(w, v.dict.length);
	for(let i = 0; i < v.dict.length; ++i) write_stream_bmfont_entry(w, v.dict[i]);
	write_stream_u32(w, v.glyphs.length);
	for(let i = 0; i < v.glyphs.length; ++i) write_stream_bmfont_glyph(w, v.glyphs[i]);
}

export const enum image_data_type {
   IMAGE_DATA_NORMAL = 0,
   IMAGE_DATA_CUBE_MAP = 1,
}

export interface image_data {
	type: image_data_type;
	format_mask: u32;
	images: string[];
}

export const read_stream_image_data = (r: Reader): image_data => {
    const val = {} as image_data;
	val.type = read_stream_u32(r) as image_data_type;
	val.format_mask = read_stream_u32(r);
	val.images = [];
	for(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.images[i] = read_stream_string(r);
    return val;
}

export const write_stream_image_data = (w: Writer, v: image_data): void => {
	write_stream_u32(w, v.type);
	write_stream_u32(w, v.format_mask);
	write_stream_u32(w, v.images.length);
	for(let i = 0; i < v.images.length; ++i) write_stream_string(w, v.images[i]);
}