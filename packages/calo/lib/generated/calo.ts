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
}

export interface Reader {
    _u8: Uint8Array;
    _pos: number;
}

export const newWriter = (cap: number): Writer => ({
    _u8: new Uint8Array(cap),
    _pos: 0,
});

export const newReader = (buffer: ArrayBuffer): Reader => ({
    _u8: new Uint8Array(buffer),
    _pos: 0,
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
    const dataLength = read_stream_u32(r);
    const len = dataLength - 1;
    const buf = r._u8.slice(r._pos, r._pos + len);
    r._pos += dataLength;
    return new TextDecoder().decode(buf);
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

export const write_stream_string = (w: Writer, v: string): void => {
    const buf = new TextEncoder().encode(v);
    ensure(w, buf.length + 4 + 1);
    write_stream_u32(w, buf.length + 1);
    w._u8.set(buf, w._pos);
    w._pos += buf.length;
    w._u8[w._pos++] = 0;
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