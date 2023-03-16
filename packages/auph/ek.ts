import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "auph",
        cpp_include: "include",
        android: {
            // from oboe dep
            //cpp_lib: ["android", "log", "OpenSLES"],
            android_java: "android/java",
            android_permission: "android.permission.VIBRATE"
        },
        apple: {
            xcode_framework: ["Foundation", "AudioToolbox"]
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
        "../../external/headers/ek.ts",
        "../../external/oboe/ek.ts",
    );
}
