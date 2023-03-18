import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "std",
        cpp: "src",
        cpp_include: "include",
    });
    await project.import(
        "../../external/headers/ek.ts"
    );
}
