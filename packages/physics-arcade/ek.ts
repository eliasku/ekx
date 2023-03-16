import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "physics-arcade",
        cpp: "src"
    });
    await project.import("../ecs/ek.ts");
}
