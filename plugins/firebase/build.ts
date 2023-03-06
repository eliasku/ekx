import {getModuleDir} from "../../lib/utils/utils.js";
import * as path from "path";
import * as esbuild from "esbuild";
import {logger} from "../../lib/cli/logger.js";

const __dirname = getModuleDir(import.meta);
await esbuild.build({
    entryPoints: [path.resolve(__dirname, "web/firebase.js")],
    globalName: "firebase_js",
    target: "es2020",
    bundle: true,
    sourcemap: true,
    outfile: path.join(__dirname, "js/script/firebase.js")
}).catch(err => logger.error(err));

await esbuild.build({
    entryPoints: [path.resolve(__dirname, "web/firebase_c.js")],
    bundle: true,
    format: "esm",
    sourcemap: true,
    target: "node14",
    outfile: path.join(__dirname, "js/lib/firebase.js")
}).catch(err => logger.error(err));

logger.info("firebase build completed");
