import {Project} from "../../lib/cli/project.js";

export function setup(project:Project) {
    project.addModule({
        name: "chipmunk2d",
        cpp: "src",
        cpp_include: "include"
    });
}
