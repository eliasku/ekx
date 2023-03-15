import {array, f32, type, u32} from "./common.js";
import {RectF, StringHash32} from "./ek_types.js";

const bmfont_glyph = type("bmfont_glyph", {
    box: RectF,
    advance_x: f32,
    sprite: StringHash32,
});

const bmfont_header = type("bmfont_header", {
    // all metrics are in EM (units / units_per_EM)
    // base font size used to generate bitmaps
    base_font_size: f32,
    line_height_multiplier: f32,
    ascender: f32,
    descender: f32,
});

const bmfont_entry = type("bmfont_entry", {
    codepoint: u32,
    glyph_index: u32,
});

export const bmfont = type("bmfont", {
    header: bmfont_header,
    dict: array(bmfont_entry),
    glyphs: array(bmfont_glyph),
});