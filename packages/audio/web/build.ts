import {getModuleDir} from "../../../lib/utils/utils.js";
import * as path from "path";
import * as esbuild from "esbuild";
import {logger} from "../../../lib/cli/logger.js";

const __dirname = getModuleDir(import.meta);

await esbuild.build({
    entryPoints: [path.resolve(__dirname, "src/webaudio/index.ts")],
    globalName: "auph",
    target: "es2020",
    bundle: true,
    sourcemap: true,
    outfile: path.join(__dirname, "../js/pre/audio.js"),
    define: {
        "process.env.NODE_ENV":'"production"'
    }
}).catch(err => logger.error(err));

logger.info("auph build completed");
