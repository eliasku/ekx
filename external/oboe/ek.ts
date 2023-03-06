import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "oboe",
        android: {
            cpp_include: [
                "src",
                "include"
            ],
            cpp_lib: ["android", "log", "OpenSLES"],
        },
    });
}
