import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "ecs",
        cpp: "src"
    });
    await project.import("../core/ek.ts");
}
