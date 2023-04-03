import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "audio",
        cpp_include: "include",
        cpp: "src",
        android: {
            cpp_flags: {
                files: [
                    "src/ek_audio.c"
                ],
                flags: "-xc++ -std=c++17"
            },
            android_java: "android/java",
            android_permission: "android.permission.VIBRATE",
            // from oboe dep
            //cpp_lib: ["android", "log", "OpenSLES"],
        },
        apple: {
            cpp_flags: {
                files: [
                    "src/ek_audio.c"
                ],
                flags: "-x objective-c"
            },
            xcode_framework: ["Foundation", "AudioToolbox"],
        },
        macos: {
            xcode_framework: "CoreAudio"
        },
        ios: {
            xcode_framework: "AVFoundation"
        },
        web: {
            js_pre: "js/pre"
        },
        windows: {},
        linux: {}
    });
    await project.import(
        "../app/ek.ts",
        "../../external/oboe/ek.ts",
    );
}
