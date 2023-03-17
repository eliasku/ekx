import {logger} from "../../../../lib/cli/logger.js";
import {len} from "../../../auph/web/src/webaudio/common.js";
import {write_stream_sg_file} from "./gen_sg.js";
import {writeFileSync} from "fs";

export type char = number;
export type u8 = number;
export type u16 = number;
export type u32 = number;
export type i8 = number;
export type i16 = number;
export type i32 = number;
export type f32 = number;
export type f64 = number;
export type StringHash32 = number;
export type Vec2 = { x: f32, y: f32 };
export type Vec3 = { x: f32, y: f32, z: f32 };
export type Vec4 = { x: f32, y: f32, z: f32, w: f32 };
export type Rect = { x: f32, y: f32, w: f32, h: f32 };
export type Color = { value: u32 };
export type Color2F = { scale: Vec4, offset: Vec4 };
export type Mat2 = { a: f32, b: f32, c: f32, d: f32 };
export type Mat3x2 = { rot: Mat2, pos: Vec2 };
export type StaticString<N extends number> = string;
export type StaticArray<T, N extends number> = [T, ...T[]] & { length: N };

const conversionBuffer = new ArrayBuffer(8);
const conversionU8 = new Uint8Array(conversionBuffer);
const conversionU32 = new Uint32Array(conversionBuffer);
const conversionI32 = new Int32Array(conversionBuffer);
const conversionF32 = new Float32Array(conversionBuffer);
const conversionF64 = new Float64Array(conversionBuffer);

export interface Writer {
    _u8: Uint8Array;
    _pos: number;
    _strings: VLATable;
}

export interface Reader {
    _u8: Uint8Array;
    _pos: number;
    _strings: VLATable;
}

interface VLATable {
    strings: string[];
}

const createVLATable = () => {
    return {
        strings: [],
    };
};

const add_string = (table: VLATable, str: string): u32 => {
    const max_length = 1024;
    if (str.length + 1 >= max_length || (new TextEncoder().encode(str)).length + 1 >= max_length) {
        logger.error("can't add string, size is more than 1024");
        return 0;
    }

    let handle = table.strings.indexOf(str);
    if (handle < 0) {
        handle = table.strings.length;
        table.strings.push(str);
    }
    return handle;
};

export const write_raw_bytes = (w: Writer, bytes: Uint8Array) => {
    const n = bytes.length;
    ensure(w, n);
    w._u8.set(bytes, w._pos);
    w._pos += n;
}

export const get_writer_bytes = (w: Writer) => {
    return w._u8.slice(0, w._pos);
};

export const get_writer_bytes_copy = (w: Writer) => {
    return new Uint8Array(w._u8.buffer.slice(0, w._pos));
};

export const save_writer = (w: Writer, filepath: string) => {
    const writer = newWriter(w._pos);
    logger.info("P0: ", writer._pos, writer._u8.length);
    write_table(writer, w._strings);
    logger.info("P1: ", writer._pos, writer._u8.length);
    logger.info("P1: ", writer._u8[0], writer._u8[1], writer._u8[2], writer._u8[3]);
    write_raw_bytes(writer, get_writer_bytes(w));
    logger.info("P2: ", writer._pos, writer._u8.length);
    logger.info("P2: ", writer._u8[0], writer._u8[1], writer._u8[2], writer._u8[3]);
    writeFileSync(filepath, get_writer_bytes_copy(writer));
};

export const write_table = (w: Writer, t: VLATable) => {
    const count = t.strings.length;
    const dataWriter = newWriter(t.strings.length * 16);
    const offsets: u32[] = [];
    const lengths: u32[] = [];

    for (let i = 0; i < count; ++i) {
        const offset = dataWriter._pos;
        write_utf8_string_data(dataWriter, t.strings[i]);
        const length = dataWriter._pos - offset;
        offsets.push(offset);
        lengths.push(length);
        console.info("write string: ", offset, length);
    }
    write_stream_u32(w, count);
    const size = dataWriter._pos;
    write_stream_u32(w, size);

    for (let i = 0; i < count; ++i) {
        write_stream_u32(w, offsets[i]);
    }
    for (let i = 0; i < count; ++i) {
        write_stream_u32(w, lengths[i]);
    }
    write_raw_bytes(w, get_writer_bytes(dataWriter));
};

export const read_table = (r: Reader) => {
    const count = read_stream_u32(r);
    const size = read_stream_u32(r);
    const offsets: u32[] = [];
    const lengths: u32[] = [];
    for (let i = 0; i < count; ++i) {
        offsets[i] = read_stream_u32(r);
    }
    for (let i = 0; i < count; ++i) {
        lengths[i] = read_stream_u32(r);
    }
    for (let i = 0; i < count; ++i) {
        const begin = r._pos + offsets[i];
        const end = begin + lengths[i];
        console.info("parse string: ", begin, end);
        if (end - begin - 1 > 0) {
            const buf = r._u8.slice(begin, end - 1);
            const str = new TextDecoder().decode(buf);
            r._strings.strings.push(str);
        }
        r._pos = end;
    }
};

//uint32_t add_data(vla_table_t* table, const void* data, uint32_t length);

//const char* get_data(const vla_table_t* table, uint32_t index);

export const newWriter = (cap: number): Writer => ({
    _u8: new Uint8Array(cap),
    _pos: 0,
    _strings: createVLATable(),
});

export const newReader = (buffer: ArrayBuffer): Reader => ({
    _u8: new Uint8Array(buffer),
    _pos: 0,
    _strings: createVLATable(),
});

export const read_stream_u8 = (r: Reader): u8 => r._u8[r._pos++];
export const read_stream_u16 = (r: Reader): u16 => (r._u8[r._pos++] | r._u8[r._pos++] << 8);
export const read_stream_u32 = (r: Reader): u32 => (r._u8[r._pos++] | r._u8[r._pos++] << 8 | r._u8[r._pos++] << 16 | r._u8[r._pos++] << 24) >>> 0;
export const read_stream_i8 = (r: Reader): i8 => (r._u8[r._pos++] << 24) >> 24;
export const read_stream_i16 = (r: Reader): i16 => (r._u8[r._pos++] | r._u8[r._pos++] << 8) << 16 >> 16;
export const read_stream_i32 = (r: Reader): i32 => r._u8[r._pos++] | r._u8[r._pos++] << 8 | r._u8[r._pos++] << 16 | r._u8[r._pos++] << 24;
export const read_stream_f32 = (r: Reader): f32 => {
    conversionU8[0] = r._u8[r._pos++];
    conversionU8[1] = r._u8[r._pos++];
    conversionU8[2] = r._u8[r._pos++];
    conversionU8[3] = r._u8[r._pos++];
    return conversionF32[0];
};
export const read_stream_f64 = (r: Reader): f64 => {
    conversionU8[0] = r._u8[r._pos++];
    conversionU8[1] = r._u8[r._pos++];
    conversionU8[2] = r._u8[r._pos++];
    conversionU8[3] = r._u8[r._pos++];
    conversionU8[4] = r._u8[r._pos++];
    conversionU8[5] = r._u8[r._pos++];
    conversionU8[6] = r._u8[r._pos++];
    conversionU8[7] = r._u8[r._pos++];
    return conversionF64[0];
};

export const read_stream_string = (r: Reader): string => {
    const handle = read_stream_u32(r);
    return r._strings.strings[handle];
};

export const read_stream_static_string = (r: Reader, n: number): string => {
    let len = 0;
    for (; len < n; ++len) {
        if (!r._u8[r._pos + len]) {
            break;
        }
    }
    r._pos += n;
    const buf = r._u8.slice(r._pos, r._pos + len);
    return new TextDecoder().decode(buf);
};

export const read_stream_boolean = (r: Reader): boolean => !!r._u8[r._pos++];
export const write_stream_u8 = (w: Writer, v: u8): void => {
    ensure(w, 1);
    w._u8[w._pos++] = v;
};
export const write_stream_u16 = (w: Writer, v: u16): void => {
    ensure(w, 2);
    w._u8[w._pos++] = v;
    w._u8[w._pos++] = v >> 8;
};
export const write_stream_u32 = (w: Writer, v: u32): void => {
    ensure(w, 4);
    w._u8[w._pos++] = v;
    w._u8[w._pos++] = v >> 8;
    w._u8[w._pos++] = v >> 16;
    w._u8[w._pos++] = v >> 24;
};
export const write_stream_i8 = (w: Writer, v: i8): void => {
    ensure(w, 1);
    w._u8[w._pos++] = v;
};
export const write_stream_i16 = (w: Writer, v: i16): void => {
    ensure(w, 2);
    w._u8[w._pos++] = v;
    w._u8[w._pos++] = v >> 8;
};
export const write_stream_i32 = (w: Writer, v: i32): void => {
    ensure(w, 4);
    w._u8[w._pos++] = v;
    w._u8[w._pos++] = v >> 8;
    w._u8[w._pos++] = v >> 16;
    w._u8[w._pos++] = v >> 24;
};
export const write_stream_f32 = (w: Writer, v: f32): void => {
    ensure(w, 4);
    conversionF32[0] = v;
    w._u8[w._pos++] = conversionU8[0];
    w._u8[w._pos++] = conversionU8[1];
    w._u8[w._pos++] = conversionU8[2];
    w._u8[w._pos++] = conversionU8[3];
};
export const write_stream_f64 = (w: Writer, v: f64): void => {
    ensure(w, 8);
    conversionF64[0] = v;
    w._u8[w._pos++] = conversionU8[0];
    w._u8[w._pos++] = conversionU8[1];
    w._u8[w._pos++] = conversionU8[2];
    w._u8[w._pos++] = conversionU8[3];
    w._u8[w._pos++] = conversionU8[4];
    w._u8[w._pos++] = conversionU8[5];
    w._u8[w._pos++] = conversionU8[6];
    w._u8[w._pos++] = conversionU8[7];
};

export const write_utf8_string_data = (w: Writer, v: string): void => {
    console.info("encode: ", v);
    const buf = new TextEncoder().encode(v);
    ensure(w, buf.length + 1);
    w._u8.set(buf, w._pos);
    w._pos += buf.length;
    w._u8[w._pos++] = 0;
};

export const write_stream_string = (w: Writer, v: string): void => {
    console.info("add string: ", v);
    const handle = add_string(w._strings, v);
    write_stream_u32(w, handle);
};

export const write_stream_static_string = (w: Writer, v: string, n: number): void => {
    ensure(w, n);
    const buf = new TextEncoder().encode(v);
    let bytesLen = buf.length + 1;
    if (bytesLen > n) {
        bytesLen = n;
    }
    w._u8.set(buf.slice(0, bytesLen - 1), w._pos);
    w._u8[w._pos + bytesLen - 1] = 0;
    w._pos += n;
};

export const write_stream_boolean = (w: Writer, v: boolean): void => {
    ensure(w, 1);
    w._u8[w._pos++] = v ? 1 : 0;
};

const ensure = (w: Writer, bytesToAdd: number) => {
    const pos2 = w._pos + bytesToAdd;
    let cap = w._u8.length;
    if (pos2 > cap) {
        while (pos2 > cap) {
            cap <<= 1;
        }
        const data = new Uint8Array(cap);
        data.set(w._u8, 0);
        w._u8 = data;
    }
}