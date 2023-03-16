import {_enum, static_array, static_string, type, u32} from "./common.js";

const image_data_type = _enum("image_data_type", {
    IMAGE_DATA_NORMAL: 0,
    IMAGE_DATA_CUBE_MAP: 1,
});

const image_path = type("image_path", {
    str: static_string(128),
});

export const image_data = type("image_data", {
    type: image_data_type,
    format_mask: u32,
    images_num: u32,
    images: static_array(image_path, 6),
});
