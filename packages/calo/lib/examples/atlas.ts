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
