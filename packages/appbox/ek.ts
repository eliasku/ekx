import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "appbox",
        cpp: "src",
        cpp_include: "include",
    });

    await project.import(
        "../scenex/ek.ts",
        "../../plugins/firebase/ek.ts",
        "../../plugins/admob/ek.ts",
        "../../plugins/billing/ek.ts",
        "../../plugins/game-services/ek.ts",
    );
}
