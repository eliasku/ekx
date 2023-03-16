import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "audio",
        cpp: "src",
        apple: {
            cpp_flags: {
                files: [
                    "src/ek_audio_auph.c"
                ],
                flags: "-x objective-c"
            },
        },
        android: {
            cpp_flags: {
                files: [
                    "src/ek_audio_auph.c"
                ],
                flags: "-xc++ -std=c++17"
            },
        }
    });
    await project.import(
        "../auph/ek.ts",
        "../app/ek.ts",
    );
}
