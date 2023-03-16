import {
    c_include,
    CGenOptions,
    generateType as gen_C,
    save_c
} from "./c/index.js";
import {generateType as gen_TS, save_ts} from "./ts/index.js";
import {collectTypes, Type} from "./common.js";
import {atlas_info} from "./atlas.js";
import {join} from "node:path";
import {mkdirSync} from "node:fs";
import {toCamelCase} from "./c/naming.js";
import {model3d} from "./model3d.js";
import {sg_file, sg_filter, sg_frame_label, sg_frame_script, sg_node_data} from "./scenex.js";
import {bmfont} from "./bmfont.js";
import {getModuleDir} from "../../../lib/utils/utils.js";
import * as path from "path";
import {image_data} from "./images.js";

c_include("ek/math.h", true, false);
c_include("ek/hash.h", true, false);
c_include("ek/buf.h", false, false);

interface GenerateOptions {
    c?: CGenOptions;
    outputDirC_src?: string;
    outputDirC_include?: string;
    outputDirTs?: string;
    name: string;
    types: Type[];
}

const generate = (options: GenerateOptions) => {
    const types = new Set<Type>();
    collectTypes(types, ...options.types);
    const rtypes = [...types.values()].reverse();
    for (const type of rtypes) {
        gen_C(type);
        gen_TS(type);
    }

    const file_prefix = "gen_";
    if (options.c) {
        save_c(options.c);
    }

    if (options.outputDirTs) {
        try {
            mkdirSync(options.outputDirTs, {recursive: true});
        } catch {
        }
        save_ts(options.name, join(options.outputDirTs, file_prefix + toCamelCase(options.name) + ".ts"));
    }
};

const caloDir = path.resolve(getModuleDir(import.meta), "..");

generate({
    c: {
        includeDir: path.join(caloDir, "include"),
        sourceDir: path.join(caloDir, "src"),
        name: "gen_sg",
    },
    outputDirTs: path.join(caloDir, "lib/generated"),
    name: "sg",
    types: [
        // dedicated images/textures
        image_data,
        // bitmap font resource
        bmfont,
        // sprite atlas resource
        atlas_info,
        // 3d model resource
        model3d,
        // scene model
        sg_filter,
        sg_node_data,
        sg_file,
        sg_frame_label,
        sg_frame_script,
    ],
});