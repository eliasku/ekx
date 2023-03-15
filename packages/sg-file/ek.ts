import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "sg-file",
        cpp: "src",
        cpp_include: "../../../calo/src",
    });
    await project.importModule("../std/ek.ts");
}
