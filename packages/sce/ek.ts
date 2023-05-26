import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "sce",
        cpp: "src",
        cpp_include: "include",
    });
    await project.import(
        "../calo/ek.ts",
        "../graphics/ek.ts",
        "../audio/ek.ts",
        "../app/ek.ts",
        "../ecs/ek.ts",
    );
}
