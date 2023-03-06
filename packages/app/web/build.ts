import {getModuleDir} from "../../../lib/utils/utils.js";
import * as path from "path";
import * as esbuild from "esbuild";
import {logger} from "../../../lib/cli/logger.js";

const __dirname = getModuleDir(import.meta);

await esbuild.build({
    entryPoints: [path.resolve(__dirname, "src/pre/index.ts")],
    globalName: "App",
    format: "iife",
    target: "es2020",
    bundle: true,
    sourcemap: true,
    minify: true,
    outfile: path.join(__dirname, "../js/pre/app.js")
}).catch(err=> logger.error(err));

await esbuild.build({
    entryPoints: [path.resolve(__dirname, "src/lib/index.ts")],
    bundle: true,
    format: "esm",
    sourcemap: true,
    target: "node14",
    outfile: path.join(__dirname, "../js/lib/app.js")
}).catch(err=> logger.error(err));

logger.info("app.js build completed");
