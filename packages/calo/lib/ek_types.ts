import {f32, type, u32} from "./common.js";

export const StringHash32 = type("StringHash32", {}, {
    primitiveBytes: 4,
    extern: true,
    readAsType: u32,
    target: {
        c: {
            typeName: "string_hash_t",
        }
    }
});

export const RectF = type("Rect", {
    x: f32,
    y: f32,
    w: f32,
    h: f32,
}, {
    extern: true,
});

export const Vec2 = type("Vec2", {
    x: f32,
    y: f32,
}, {
    extern: true,
});

export const Vec3 = type("Vec3", {
    x: f32,
    y: f32,
    z: f32,
}, {
    extern: true,
});

export const Vec4 = type("Vec4", {
    x: f32,
    y: f32,
    z: f32,
    w: f32,
}, {
    extern: true,
});

export const Color = type("Color", {
    value: u32,
}, {
    extern: true,
});

export const Color2F = type("Color2F", {
    scale: Vec4,
    offset: Vec4,
}, {
    extern: true,
});

export const Mat2x2 = type("Mat2", {
    a: f32,
    b: f32,
    c: f32,
    d: f32,
}, {
    extern: true,
});

export const Mat3x2 = type("Mat3x2", {
    rot: Mat2x2,
    pos: Vec2,
}, {
    extern: true,
});

export const Mat3 = type("Mat3", {
    m00: f32, m01: f32, m02: f32,
    m10: f32, m11: f32, m12: f32,
    m20: f32, m21: f32, m22: f32,
}, {
    extern: true,
});

export const Mat4 = type("Mat4", {
    m00: f32, m01: f32, m02: f32, m03: f32,
    m10: f32, m11: f32, m12: f32, m13: f32,
    m20: f32, m21: f32, m22: f32, m23: f32,
    m30: f32, m31: f32, m32: f32, m33: f32,
}, {
    extern: true,
});


