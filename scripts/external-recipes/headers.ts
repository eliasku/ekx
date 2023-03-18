import {join} from "node:path";
import {rm} from "../../lib/utils/utils.js";
import {downloadFiles} from "../../lib/utils/download.js";
import {resolveEkxPath} from "../../lib/utils/dirs.js";

const destPath = resolveEkxPath("external/headers");

async function clean() {
    await rm(join(destPath, "include"));
}

function fetch() {
    return Promise.all([
        download_stb(),
        download_dr(),
        download_pocketmod(),
        download_fast_obj(),
        download_sokol(),
    ]).then(_ => undefined);
}

async function download_stb() {
    const branch = "master";
    const repoUrl = `https://github.com/nothings/stb/raw/${branch}`;
    await downloadFiles({
        srcBaseUrl: repoUrl,
        destPath: join(destPath, "include/stb"),
        fileList: [
            "stb_image.h",
            "stb_image_write.h",
            "stb_sprintf.h",
            "stb_truetype.h",
            "stb_vorbis.c",
            "stb_rect_pack.h",
        ]
    });
}

async function download_dr() {
    const branch = "master";
    const repoUrl = `https://github.com/mackron/dr_libs/raw/${branch}`;
    await downloadFiles({
        srcBaseUrl: repoUrl,
        destPath: join(destPath, "include/dr"),
        fileList: [
            "dr_mp3.h",
            "dr_wav.h",
        ]
    });
}

async function download_pocketmod() {
    const branch = "master";
    const repoUrl = `https://github.com/rombankzero/pocketmod/raw/${branch}`;
    await downloadFiles({
        srcBaseUrl: repoUrl,
        destPath: join(destPath, "include/pocketmod"),
        fileList: [
            "pocketmod.h"
        ]
    });
}

async function download_fast_obj() {
    const branch = "master";
    const repoUrl = `https://github.com/thisistherk/fast_obj/raw/${branch}`;
    await downloadFiles({
        srcBaseUrl: repoUrl,
        destPath: join(destPath, "include/fast_obj"),
        fileList: [
            "fast_obj.h"
        ]
    });
}

async function download_sokol() {
    const branch = "master";
    const repoUrl = `https://github.com/floooh/sokol/raw/${branch}`;
    const destDir = join(destPath, "include/sokol");
    await downloadFiles({
        srcBaseUrl: repoUrl,
        destPath: destDir,
        fileList: [
            "sokol_gfx.h",
            "sokol_time.h",
            "util/sokol_imgui.h",
            "util/sokol_gfx_imgui.h",
        ],
    });
}

export default {clean, fetch};
