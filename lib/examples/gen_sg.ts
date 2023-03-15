interface sg_frame_script {
	frame: number;
	code: string;
}


interface sg_frame_label {
	frame: number;
	name: string;
}


interface sg_scene_info {
	name: string_hash;
	linkage: string_hash;
}


interface sg_file {
	scenes: string_hash[];
	linkages: sg_scene_info[];
	library: sg_node_data_struct[];
}


interface sg_keyframe_transform {
	position: Vec2;
	scale: Vec2;
	skew: Vec2;
	pivot: Vec2;
	color: Color2F;
}


interface sg_easing {
	ease: number;
	curve: Vec2[];
	attribute: number;
}


interface sg_movie_frame {
	index: number;
	duration: number;
	motion_type: number;
	easing: sg_easing[];
	transform: sg_keyframe_transform;
	visible: boolean;
	loop_mode: number;
	first_frame: number;
	rotate: number;
	rotate_times: number;
}


interface sg_movie_layer {
	frames: sg_movie_frame[];
	targets: sg_node_data**;
}


interface sg_movie {
	frames: number;
	fps: number;
	layers: sg_movie_layer[];
}


interface Optional {
}


interface sg_text_layer {
	color: Color;
	offset: Vec2;
	blur_radius: number;
	blur_iterations: number;
	strength: number;
}


interface sg_dynamic_text {
	text: string;
	font: string_hash;
	size: number;
	alignment: Vec2;
	rect: Rect;
	line_spacing: number;
	line_height: number;
	layers: sg_text_layer[];
	word_wrap: boolean;
}


interface Optional {
}


interface sg_node_flags {
}


interface sg_node_data {
	matrix: Mat3x2;
	color: Color2F;
	name: string_hash;
	library_name: string_hash;
	sprite: string_hash;
	flags: sg_node_flags;
	bounding_rect: Rect;
	scale_grid: Rect;
	children: sg_node_data_struct[];
	dynamic_text: Optional;
	movie: Optional;
	labels: sg_frame_label[];
	scripts: sg_frame_script[];
	movie_target_id: number;
}


interface sg_filter_type {
}


interface sg_filter {
	type: sg_filter_type;
	quality: number;
	color: Color;
	blur: Vec2;
	offset: Vec2;
}


interface model3d_vertex {
	position: Vec3;
	normal: Vec3;
	uv: Vec2;
	color: Color;
	color2: Color;
}


interface model3d {
	vertices: model3d_vertex[];
	indices: number[];
}


interface sprite_info {
	name: string_hash;
	flags: number;
	rc: Rect;
	uv: Rect;
}


interface atlas_page_info {
	width: number;
	height: number;
	image_path: string;
	sprites: sprite_info[];
}


interface atlas_info {
	pages: atlas_page_info[];
}


interface bmfont_glyph {
	box: Rect;
	advance_x: number;
	sprite: string_hash;
}


interface bmfont_entry {
	codepoint: number;
	glyph_index: number;
}


interface bmfont_header {
	base_font_size: number;
	line_height_multiplier: number;
	ascender: number;
	descender: number;
}


interface bmfont {
	header: bmfont_header;
	dict: bmfont_entry[];
	glyphs: bmfont_glyph[];
}
