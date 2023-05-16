import {getModuleDir} from "../../lib/utils/utils.js";
import {shdc} from "../../lib/sokol-shdc.js";
import {logger} from "../../lib/cli/logger.js";

await shdc({
    input: "src/ek/scenex/3d/render3d.glsl",
    output: "src/ek/scenex/3d/render3d_shader.h",
    cwd: getModuleDir(import.meta)
});

logger.info("scenex render3d shader build completed");