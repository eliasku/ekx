import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "std",
        cpp: "src",
        cpp_include: "include",
        web: {
            //cpp: "src-wasm",
            //cpp_lib: "--import-memory"
        }
    });
    await project.import(
        "../../external/headers/ek.ts",
        "../../external/sokol/ek.ts",
    );
}
