import {_enum, array, CString, static_array, static_string, type, u32} from "./common.js";

const image_data_type = _enum("image_data_type", {
    IMAGE_DATA_NORMAL: 0,
    IMAGE_DATA_CUBE_MAP: 1,
});

export const image_data = type("image_data", {
    type: image_data_type,
    format_mask: u32,
    images: array(CString),
});
