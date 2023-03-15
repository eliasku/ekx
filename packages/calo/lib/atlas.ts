import {array, CString, type, u16, u32} from "./common.js";
import {RectF, StringHash32} from "./ek_types.js";

const sprite_info = type("sprite_info", {
    name: StringHash32,
    // flags in atlas image
    flags: u32,

    // physical rect
    rc: RectF,

    // coords in atlas image
    uv: RectF,
});

const atlas_page_info = type("atlas_page_info", {
    width: u16,
    height: u16,
    image_path: CString,
    sprites: array(sprite_info),
});

export const atlas_info = type("atlas_info", {
    pages: array(atlas_page_info),
});