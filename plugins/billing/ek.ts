import {Project} from "../../lib/cli/project.js";

export function setup(project: Project) {
    project.addModule({
        name: "plugin-billing",
        cpp: "src",
        cpp_include: "include",
        android: {
            android_java: "java",
            android_dependency: `implementation 'com.android.billingclient:billing:5.2.0'`
        },
        ios: {
            xcode_capability: "com.apple.InAppPurchase",
            xcode_framework: "StoreKit",
            cpp_flags: {
                files: [
                    "src/billing.c"
                ],
                flags: "-x objective-c++"
            }
        }
    });
}
