import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "graphics",
        cpp: "src",
        apple: {
            cpp_flags: {
                files: [
                    "src/ek/ek_gfx.c"
                ],
                flags: "-x objective-c"
            },
        }
    });
    await project.import("../app/ek.ts");
}
