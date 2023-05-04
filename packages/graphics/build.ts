import {getModuleDir} from "../../lib/utils/utils.js";
import {shdc} from "../../lib/sokol-shdc.js";
import {logger} from "../../lib/cli/logger.js";

await shdc({
    input: "src/canvas.glsl",
    output: "src/canvas_shader.h",
    cwd: getModuleDir(import.meta)
});

logger.info("graphics build completed");