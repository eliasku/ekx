import {readFileSync} from "fs";
import {
    newReader,
    read_stream_f32,
    read_stream_string,
    read_stream_u32,
    read_table
} from "../../../packages/calo/lib/generated/calo.js";
import {read_stream_image_data, read_stream_sg_file} from "../../../packages/calo/lib/generated/gen_sg.js";
import {H} from "../utility/hash.js";
import {logger} from "../logger.js";

export const listAssets = () => {
    const buf = readFileSync("build/assets/pack.bin");
    const r = newReader(buf);
    read_table(r);

    while(r._pos < r._u8.length) {
    const type = read_stream_u32(r);
    if (type == H("audio")) {
        const name = read_stream_u32(r);
        const flags = read_stream_u32(r);
        const path = read_stream_string(r);
        logger.info("sound: " + path);
    } else if (type == H("scene")) {
        const name = read_stream_u32(r);
        const path = read_stream_string(r);
        logger.info("scene: " + path);
    } else if (type == H("bmfont")) {
        const name = read_stream_u32(r);
        const path = read_stream_string(r);
        logger.info("bmfont: " + path);
    } else if (type == H("ttf")) {
        const name = read_stream_u32(r);
        const path = read_stream_string(r);
        const glyphCache = read_stream_u32(r);
        const baseFontSize = read_stream_f32(r);
        logger.info("ttf: " + path);
    } else if (type == H("atlas")) {
        const name = read_stream_string(r);
        const formatMask = read_stream_u32(r);
        logger.info("atlas: " + name);
    } else if (type == H("dynamic_atlas")) {
        const name = read_stream_u32(r);
        const flags = read_stream_u32(r);
        logger.info("dynamic_atlas: " + name);
    } else if (type == H("model")) {
        const name = read_stream_string(r);
        logger.info("model: " + name);
    } else if (type == H("texture")) {
        const name = read_stream_u32(r);
        const texData = read_stream_image_data(r);
        logger.info("texture: " + name);
    } else if (type == H("strings")) {
        const name = read_stream_string(r);
        const langs_num = read_stream_u32(r);
        r._pos += 16 * 8;
        logger.info("strings: " + name);
    } else if (type == H("pack")) {
        const name = read_stream_string(r);
        logger.info("pack: " + name);
    } else {
        logger.error("asset: unknown `type` name hash: " + type);
    }
    }
};