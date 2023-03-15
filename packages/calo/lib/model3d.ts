import {array, type, u16} from "./common.js";
import {Color, Vec2, Vec3} from "./ek_types.js";

const model3d_vertex = type("model3d_vertex", {
    position: Vec3,
    normal: Vec3,
    uv: Vec2,
    color: Color,
    color2: Color,
});

export const model3d = type("model3d", {
    vertices: array(model3d_vertex),
    indices: array(u16),
});
