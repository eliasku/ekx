import * as path from "path";
import {rm} from "../../lib/utils/utils.js";
import {downloadFiles} from "../../lib/utils/download.js";
import {resolveEkxPath} from "../../lib/utils/dirs.js";

const destPath = resolveEkxPath("external/pugixml");

async function clean() {
    await rm(path.join(destPath, "src"));
}

async function fetch() {
    await downloadFiles({
        srcBaseUrl: "https://github.com/zeux/pugixml/raw/master",
        destPath,
        fileList: [
            "src/pugiconfig.hpp",
            "src/pugixml.hpp",
            "src/pugixml.cpp",
        ]
    });
}

export default {clean, fetch};
