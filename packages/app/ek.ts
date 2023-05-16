import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "app",
        cpp: "src",
        cpp_include: "include",
        android: {
            cpp_lib: ["log", "android", "GLESv3"],
            android_java: "java",
            android_dependency: [
                `implementation 'androidx.appcompat:appcompat:1.6.1'`
            ]
        },
        macos: {
            xcode_framework: [
                "Cocoa",
                "Metal",
                "MetalKit",
                "QuartzCore",
            ]
        },
        ios: {
            xcode_framework: [
                "Foundation",
                "UIKit",
                "Metal",
                "MetalKit",
                "QuartzCore",
            ]
        },
        web: {
            js: "js/lib",
            js_pre: "js/pre",
            cpp_lib: ["GL"]
        },
    });
    await project.import(
        "../std/ek.ts",
    );
}
