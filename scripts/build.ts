import {existsSync} from "fs";
import {UtilityConfig} from "../lib/cli/utils.js";
import {logger} from "../lib/cli/logger.js";

// run build files
import "../packages/app/web/build.js";
import "../packages/auph/web/build.js";
import "../packages/graphics/build.js";
import "../plugins/firebase/build.js";

UtilityConfig.verbose = true;

// check files are really exists
const filesToVerify = [
    "packages/app/js/lib/app.js",
    "packages/app/js/pre/app.js",
    "packages/auph/js/pre/auph.js",
    "packages/auph/js/pre/auph.js.map",
];

let missFiles = 0;
for (const file of filesToVerify) {
    if (existsSync(file)) {
        logger.info("✅ file is in place", file)
    } else {
        logger.warn("🤷 file is missing", file);
        ++missFiles;
    }
}

process.exit(missFiles > 0 ? 1 : 0);
