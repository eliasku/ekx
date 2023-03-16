import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "imgui",
        cpp: "src"
    });
    await project.import(
        "../headers/ek.ts",
        "../freetype/ek.ts",
    );
}
