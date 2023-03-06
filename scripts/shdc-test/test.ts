import {join} from "path";
import {existsSync} from "fs";
import {shdc} from "../../lib/sokol-shdc.js";
import {getModuleDir, rm} from "../../lib/utils/utils.js";
import {UtilityConfig} from "../../lib/cli/utils.js";

UtilityConfig.verbose = true;

const __dirname = getModuleDir(import.meta);

await shdc({
    input: "simple2d.glsl",
    output: "simple2d_shader.h",
    cwd: __dirname
});

const hdr = join(__dirname, "simple2d_shader.h");
if (!existsSync(hdr)) {
    throw new Error("shader header not found");
}

await rm(hdr);
