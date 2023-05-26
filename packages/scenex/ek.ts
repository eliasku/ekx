import {Project} from "../../lib/cli/project.js";

export async function setup(project: Project) {
    project.addModule({
        name: "scenex",
        cpp: "src",
        cpp_include: "include",
        android: {
            android_permission: "android.permission.INTERNET"
        },
    });
    await project.import(
        "../sce/ek.ts",
    );
}
