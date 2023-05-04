import * as path from "path";
import {copyFileSync} from "fs";
import {Project} from "../../lib/cli/project.js";
import {logger} from "../../lib/cli/logger.js";

export async function setup(project: Project) {
    project.onProjectGenerated.push(() => {
        if (project.current_target === "android") {
            if (!project.android.googleServicesConfigDir) {
                throw new Error("please set `android.googleServicesConfigDir` !!!");
            }
            const configFile = "google-services.json";
            const configPath = path.join(path.resolve(project.projectPath, project.android.googleServicesConfigDir), configFile);
            try {
                // CWD is project generated path here
                copyFileSync(configPath, path.join("app", configFile))
            } catch (err) {
                logger.error("missing google-service config", configPath, err);
            }
        } else if (project.current_target === "ios") {
            if (!project.ios.googleServicesConfigDir) {
                throw new Error("please set `ios.googleServicesConfigDir` !!!");
            }
            const configFile = "GoogleService-Info.plist";
            const configPath = path.join(path.resolve(project.projectPath, project.ios.googleServicesConfigDir), configFile);
            try {
                // CWD is project generated path here
                copyFileSync(configPath, configFile);
            } catch (err) {
                logger.error("missing google-service config", configPath, err);
            }
        } else if (project.current_target === "web") {
            if (!project.web.firebaseConfig) {
                logger.error("please set `web.firebaseConfig` !!!");
            }
//             project.web.headCode.push(`<script type="module">
//   import { initializeApp } from "https://www.gstatic.com/firebasejs/9.1.2/firebase-app.js";
//   import { getAnalytics } from "https://www.gstatic.com/firebasejs/9.1.2/firebase-analytics.js";
//   var firebaseConfig = ${JSON.stringify(project.web.firebaseConfig)};
//   var app = initializeApp(firebaseConfig);
//   var analytics = getAnalytics(app);
// </script>`);
            project.web.headCode.push(`<script>
  window.firebaseConfig = ${JSON.stringify(project.web.firebaseConfig)};
</script>`);

            // auth ui
            project.web.headCode.push(`
<script src="https://www.gstatic.com/firebasejs/9.6.10/firebase-app-compat.js"></script>
<script src="https://www.gstatic.com/firebasejs/9.6.10/firebase-analytics-compat.js"></script>
`);
        }
    });

    project.addModule({
        name: "plugin-firebase",
        cpp: "src",
        cpp_include: "include",
        android: {
            // TODO: fastlane
            // gradle(task: "uploadCrashlyticsSymbolFile", build_type: "Release")

            android_java: "java",
            // Import the BoM for the Firebase platform
            // Check Release Notes for updates: https://firebase.google.com/support/release-notes/android
            android_buildScriptDependency: [
                `classpath 'com.google.gms:google-services:4.3.15'`,
                `classpath 'com.google.firebase:firebase-crashlytics-gradle:2.9.5'`
            ],
            android_gradleApplyPlugin: ['com.google.gms.google-services', 'com.google.firebase.crashlytics'],
            android_gradleConfigRelease: `firebaseCrashlytics {
                nativeSymbolUploadEnabled true
            }`,
            android_dependency: [
                `implementation platform('com.google.firebase:firebase-bom:31.5.0')`,
                `implementation 'com.google.firebase:firebase-crashlytics-ndk'`,
                `implementation 'com.google.firebase:firebase-analytics'`
            ]
        },
        ios: {
            podfile_pod: [
                "Firebase/Crashlytics",
                "Firebase/Analytics"
            ],
            podfile_code: [
                `script_phase name: 'Run Firebase Crashlytics',
        shell_path: '/bin/sh',
        script: '"\${PODS_ROOT}/FirebaseCrashlytics/run"',
        execution_position: :after_compile,
        input_files: ['$(SRCROOT)/$(BUILT_PRODUCTS_DIR)/$(INFOPLIST_PATH)','\${DWARF_DSYM_FOLDER_PATH}/\${DWARF_DSYM_FILE_NAME}/Contents/Resources/DWARF/\${TARGET_NAME}']`
            ],
            cpp_flags: {
                files: [
                    "src/firebase.c"
                ],
                flags: "-x objective-c"
            },
            xcode_file: [
                "GoogleService-Info.plist"
            ],
        },
        web: {
            js: "js/lib",
            js_script: "js/script/firebase.js"
        }
    });

    await project.import("../../packages/app/ek.ts");
}
