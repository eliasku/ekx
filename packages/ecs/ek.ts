import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "ecs",
        cpp: "src",
        cpp_include: "include",
    });
    await project.import("../std/ek.ts");
}
