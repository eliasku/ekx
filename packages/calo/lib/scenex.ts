import {_enum, array, bool, CString, f32, i32, optional, type, u32, u8} from "./common.js";
import {Color, Color2F, Mat3x2, RectF, StringHash32, Vec2} from "./ek_types.js";

const sg_filter_type = _enum("sg_filter_type", {
    SG_FILTER_NONE: 0,
    SG_FILTER_SHADOW: 1,
    SG_FILTER_GLOW: 2,
});

export const sg_filter = type("sg_filter", {
    type: sg_filter_type,
    quality: u32,
    color: Color,
    blur: Vec2,
    offset: Vec2,
});

const sg_text_layer = type("sg_text_layer", {
    color: Color,
    offset: Vec2,
    blur_radius: f32,
    blur_iterations: u32,
    strength: u32
});

const sg_keyframe_transform = type("sg_keyframe_transform", {
    position: Vec2,
    scale: Vec2,
    skew: Vec2,
    pivot: Vec2,
    color: Color2F,
});

export const sg_frame_label = type("sg_frame_label", {
    frame: i32,
    name: CString,
});

export const sg_frame_script = type("sg_frame_script", {
    frame: i32,
    code: CString,
});

const sg_scene_info = type("sg_scene_info", {
    // public export linkage name
    name: StringHash32,
    // internal symbol name
    linkage: StringHash32,
});

const sg_dynamic_text = type("sg_dynamic_text", {
    text: CString,
    font: StringHash32,
    size: f32,
    alignment: Vec2,
    rect: RectF,
    line_spacing: f32,
    line_height: f32,
    layers: array(sg_text_layer),
    word_wrap: bool,
});

// TODO: it should be optimized with easing table store :)
const sg_easing = type("sg_easing", {
    ease: f32,
    curve: array(Vec2),
    attribute: u8,
});

// TODO: VALIDATION!
// #ifndef NDEBUG
// uint32_t mask = 0;
// for(auto& ease : easing) {
//     EK_ASSERT((mask & ease.attribute) == 0);
//     mask |= 1 << ease.attribute;
// }
// #endif
const sg_movie_frame = type("sg_movie_frame", {
    index: i32,
    duration: i32,
    motion_type: i32,

    easing: array(sg_easing),

    transform: sg_keyframe_transform,

    visible: bool,

    // graphic frame control
    loop_mode: i32, // loop_mode
    first_frame: i32,

    // rotation postprocessing
    rotate: i32, // rotate_direction
    rotate_times: i32,
});

const sg_node_data_struct = type("sg_node_data_struct", {}, {
    apiStreamRead: true,
    apiStreamWrite: true,
    extern: true,
    target: {
        c: {
            typeName: "struct sg_node_data_",
            reader: "read_stream_sg_node_data",
            writer: "write_stream_sg_node_data",
        },
        ts: {
            typeName: "sg_node_data",
            reader: "read_stream_sg_node_data",
            writer: "write_stream_sg_node_data",
        },
    },
});

const sg_node_data_array = array(sg_node_data_struct);

const sg_movie_layer = type("sg_movie_layer", {
    frames: array(sg_movie_frame),

    // temp for restoring target ID
    //@TRANSIENT
    targets: type("sg_node_data**", {}, {
        target: {
            c: {
                typeName: "struct sg_node_data_ **",
                reader: "_THIS_IS_TRANSIENT_FIELD_",
            },
            ts: {
                typeName: "any",
                reader: "_THIS_IS_TRANSIENT_FIELD_",
            }
        },
        apiStreamRead: true,
        extern: true,
    }),
}, {
    transientFields: {
        targets: true,
    },
});

const sg_movie = type("sg_movie", {
    frames: i32, // = 1
    fps: f32, // = 24
    layers: array(sg_movie_layer),
    // TODO: labels and scripts
});

const sg_node_flags = _enum("sg_node_flags", {
    SG_NODE_BUTTON: 1 << 0,
    SG_NODE_TOUCHABLE: 1 << 1,
    SG_NODE_VISIBLE: 1 << 2,
    SG_NODE_SCISSORS_ENABLED: 1 << 3,
    SG_NODE_HIT_AREA_ENABLED: 1 << 4,
    SG_NODE_BOUNDS_ENABLED: 1 << 5,
});

export const sg_node_data = type("sg_node_data", {
    matrix: Mat3x2, // = mat3x2_identity();
    color: Color2F, //=  color2f();

    // instance name
    name: StringHash32,

    // name in library
    library_name: StringHash32,

    // sprite id
    sprite: StringHash32,

    flags: sg_node_flags,
    bounding_rect: RectF,
    scale_grid: RectF,
    children: sg_node_data_array,
    dynamic_text: optional(sg_dynamic_text),
    movie: optional(sg_movie),

    // SKIP
    labels: array(sg_frame_label),
    // SKIP
    scripts: array(sg_frame_script),
    movie_target_id: i32, // = -1;
}, {
    transientFields: {
        labels: true,
        scripts: true,
    },
});

export const sg_file = type("sg_file", {
    scenes: array(StringHash32),
    linkages: array(sg_scene_info),
    library: sg_node_data_array,
});