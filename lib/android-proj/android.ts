import * as path from "path"
import * as os from "os"
import {logger} from "../cli/logger.js";
import {run} from "../utils/utils.js";

export function getAndroidSdkRoot(): string | null {
    return process.env.ANDROID_SDK_ROOT ?? path.join(process.env.HOME ?? "~", 'Library/Android/sdk');
}

export async function getJavaHome(version: string | number): Promise<string> {
    let home = "";
    const plat = os.platform();
    if (plat === "darwin") {
        const result = await run({cmd: ["/usr/libexec/java_home", "-v", version.toString()], stdio: "pipe", io: true});
        home = result.data ?? "";
    } else if (plat === "linux") {
        // TODO: don't respect java version
        const result = await run({cmd: ["java", "-XshowSettings:properties", "-version"], stdio: "pipe", io: true});
        const output = result.data || result.error;
        if (output) {
            home = output.match(/java\.home\s*=\s*(.*)/i)?.[1] ?? "";
        }
    }
    if (home) {
        logger.debug(`java.home found: ${home}`);
    } else {
        logger.error("java.home not found");
    }
    return home;
}

function getAndroidStudioPath(): string {
    switch (os.platform()) {
        case "darwin":
            return "/Applications/Android Studio.app";
    }
    throw new Error("Not supported");
}

export function openAndroidStudioProject(projectPath: string): void {
    switch (os.platform()) {
        case "darwin":
            run({cmd: ["open", "-a", getAndroidStudioPath(), projectPath]}).catch((err) => logger.error(err));
            break;
        default:
            logger.error("Not supported");
            break;
    }
}