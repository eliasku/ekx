import * as path from "path";
import * as fs from "fs";
import {execute2, isDir, isFile, replaceAll, replaceInFile,} from "../utils.js";
import {buildAssetPackAsync} from "../assets.js";
import {
    collectCppFlags,
    collectObjects,
    collectSourceFiles,
    collectSourceRootsAll,
    collectStrings
} from "../collectSources.js";
import {Project} from "../project.js";
import {CMakeGenerateProject, CMakeGenerateTarget, cmakeLists} from "../../cmake/generate.js";
import {logger} from "../logger.js";
import {AndroidProjGen, openAndroidStudioProject} from "../../android-proj/index.js";
import {buildAppIconAsync} from "../appicon/appicon.js";
import {
    callInDirSync,
    ensureDirSync,
    readJSONFileSync,
    rm,
    run,
    writeTextFileSync
} from "../../utils/utils.js";

const platforms = ["android"];

async function gradle(dir: string, ...args: string[]) {
    // const ANDROID_SDK_ROOT = getAndroidSdkRoot();
    //
    // let env = Object.create(process.env);
    // env = Object.assign(env, {
    //     ANDROID_SDK_ROOT,
    //     //JAVA_HOME
    // });

    await run({
        cmd: ["./gradlew", ...args],
        stdio: "inherit",
        cwd: dir
    });
}

const baseActivityClassName = "ek.EkActivity";
const activityClassName = "MainActivity";

function createMainClass(javaPackage: string, appModulePath: string) {
    const javaPackagePath = replaceAll(javaPackage, ".", "/");
    let file = `package ${javaPackage};

import android.os.Bundle;

public class ${activityClassName} extends ${baseActivityClassName} {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        ek.AppUtils.setDebugBuild(BuildConfig.DEBUG);
        super.onCreate(savedInstanceState);
    }
}`;

    const classFilePath = path.join(appModulePath, "src/main/java", javaPackagePath, `${activityClassName}.java`);
    ensureDirSync(path.dirname(classFilePath));
    writeTextFileSync(classFilePath, file);
}

function getAndroidScreenOrientation(orientation: string): string {
    if (orientation === "landscape") {
        return "sensorLandscape";
    } else if (orientation === "portrait") {
        return "sensorPortrait";
    } else {
        logger.warn("unknown orientation", orientation);
    }
    return "sensorPortrait";
}

function setupAndroidManifest(ctx: Project, proj: AndroidProjGen) {
    proj.androidManifest.activityName = ctx.android.package_id + "." + activityClassName;
    proj.androidManifest._root.push(...collectStrings(ctx, "android_manifest", ["android"], false));
    proj.androidManifest.permissions.push(...collectStrings(ctx, "android_permission", ["android"], false));
    proj.androidManifest._application.push(...collectStrings(ctx, "android_manifestApplication", ["android"], false));
    proj.androidManifest.screenOrientation = getAndroidScreenOrientation(ctx.orientation);
    proj.androidManifest.configChanges = [
        "keyboardHidden",
        "keyboard",
        "orientation",
        "screenSize",
        "layoutDirection",
        "locale",
        "uiMode",
        "screenLayout",
        "smallestScreenSize",
        "navigation"
    ];
}

function setupStringsXML(ctx: Project, proj: AndroidProjGen) {
    const android_strings = collectObjects(ctx, "android_strings", platforms);
    for (const strings of android_strings) {
        for (const key of Object.keys(strings)) {
            proj.strings[key] = strings[key];
        }
    }
    proj.strings.app_name = ctx.title;
    proj.strings.package_name = ctx.android.application_id;
}

function globSourceFiles(ctx: Project, cmakeDir: string) {
    const cppSources: string[] = [];
    const cppExtensions: string[] = ["hpp", "hxx", "h", "cpp", "cxx", "c"];
    callInDirSync(cmakeDir, () => {
        const cppRoots = collectSourceRootsAll(ctx, "cpp", platforms, ".");
        for (const cppRoot of cppRoots) {
            collectSourceFiles(cppRoot, cppExtensions, cppSources);
        }
    });
    return cppSources;
}

function createCMakeLists(dir: string, ctx: Project) {

    const cpp_sources = globSourceFiles(ctx, dir);
    const cpp_roots = collectSourceRootsAll(ctx, "cpp", platforms, dir);
    const cpp_include = collectSourceRootsAll(ctx, "cpp_include", platforms, dir);
    const cpp_define = collectStrings(ctx, "cpp_define", platforms, false);
    const cpp_lib = collectStrings(ctx, "cpp_lib", platforms, false);
    const cpp_flags = collectCppFlags(ctx, platforms);

    const cmakeName = "native-lib";
    const cmakeTarget: CMakeGenerateTarget = {
        type: "library",
        libraryType: "shared",
        name: cmakeName,
        sources: cpp_sources,
        includeDirectories: cpp_roots.concat(cpp_include),
        linkLibraries: cpp_lib,
        linkOptions: ["-g"],
        compileOptions: [
            "-g",
            "-ffunction-sections", //+
            "-fdata-sections", //+
            "-fvisibility=hidden",//+

            // "-fvisibility-inlines-hidden",
            // "-ftree-vectorize",
            // "-ffor-scope",
            // "-pipe",

            "-Oz",
            "-ffast-math",
            "-fno-exceptions",
            "-fno-rtti",

            "-Wall",
            "-Wextra",

            //"-Werror",
            "-Wnon-virtual-dtor",
            "-Wsign-promo",
            //"-Wstrict-null-sentinel"
        ],
        compileDefinitions: cpp_define,
        sourceFileCompileFlags: cpp_flags,
        // because Android's Oboe is still C++ 
        compileAsCpp: true,
    };

    // -fno-exceptions
    // -fno-rtti
    // -ffunction-sections
    // -fdata-sections
    // -fvisibility=hidden

    const cmakeProject: CMakeGenerateProject = {
        cmakeVersion: "3.19",
        project: cmakeName,
        targets: [cmakeTarget],
        compileOptions: [],
        compileDefinitions: []
    };

    cmakeTarget.linkOptions.push("-Wl,--build-id");
    cmakeTarget.linkOptions.push("-Wl,--gc-sections");
    cmakeTarget.linkOptions.push("-flto");
    cmakeTarget.compileOptions.push("-flto");

    // cmakeTarget.compileDefinitions.push("$<$<NOT:$<CONFIG:Debug>>:NDEBUG>");
    // cmakeTarget.compileOptions.push("$<$<NOT:$<CONFIG:Debug>>:-Oz>");
    // cmakeTarget.linkOptions.push("$<$<NOT:$<CONFIG:Debug>>:-Oz>");

    writeTextFileSync(path.join(dir, "CMakeLists.txt"), cmakeLists(cmakeProject));
}

/**
 * Check signing config file and warning if Legacy Names are found
 * @param config - signing config object to change
 */
function checkSigningConfigNames(config: any) {
    if (config.store_keystore != null || config.store_password != null || config.key_alias != null || config.key_password) {
        logger.warn("Android signing configs file has Legacy names, please check and update");
    }
}

export async function export_android(ctx: Project): Promise<void> {

    // generate project
    const defaultPackageName = ("ek." + ctx.name).replace(/-/g, "_");
    if (!ctx.android.application_id) {
        ctx.android.application_id = defaultPackageName;
        logger.warn("android.application_id property not found, using default:", ctx.android.application_id);
    }

    if (!ctx.android.package_id) {
        ctx.android.package_id = defaultPackageName;
        logger.warn("android.package_id property not found, using default:", ctx.android.package_id);
    }


    const platform_proj_name = ctx.name + "-" + ctx.current_target;
    const dest_dir = path.resolve(process.cwd(), "export");
    const projectPath = path.join(dest_dir, platform_proj_name);
    const appModulePath = path.join(projectPath, "app");
    const embeddedAssets = path.join(appModulePath, "src/main/assets");

    if (isDir(projectPath)) {
        logger.info("Remove old project", projectPath);
        await rm(projectPath);
    }

    // resolve absolute path to configs
    let signingConfigsPath = ctx.android.signingConfigPath;
    if (signingConfigsPath) {
        signingConfigsPath = path.resolve(ctx.projectPath, signingConfigsPath);
    }

    let serviceAccountKey = ctx.android.serviceAccountKey;
    if (serviceAccountKey) {
        serviceAccountKey = path.resolve(ctx.projectPath, serviceAccountKey);
    }

    ensureDirSync(projectPath);

    {
        const proj = new AndroidProjGen();
        await proj.prepare();
        proj.name = ctx.name;

        const assets = collectSourceRootsAll(ctx, "assets", platforms, appModulePath);
        const android_java = collectSourceRootsAll(ctx, "android_java", platforms, appModulePath);
        const android_aidl = collectSourceRootsAll(ctx, "android_aidl", platforms, appModulePath);
        const android_dependency = collectStrings(ctx, "android_dependency", platforms, false);
        const android_gradleApplyPlugin = collectStrings(ctx, "android_gradleApplyPlugin", platforms, false);
        const android_buildScriptDependency = collectStrings(ctx, "android_buildScriptDependency", platforms, false);
        const android_gradleConfigRelease = collectStrings(ctx, "android_gradleConfigRelease", platforms, false);

        android_java.push("src/main/java");
        assets.push(embeddedAssets);

        const appAndroid = proj.app.android!;
        appAndroid.sourceSets!.main.java.srcDirs = android_java;
        appAndroid.sourceSets!.main.aidl.srcDirs = android_aidl;
        appAndroid.sourceSets!.main.assets.srcDirs = assets;

        appAndroid.defaultConfig.versionCode = ctx.version.buildNumber();
        appAndroid.defaultConfig.versionName = ctx.version.name();
        appAndroid.defaultConfig.applicationId = ctx.android.application_id;
        proj.app.plugins!.push(...android_gradleApplyPlugin);

        proj.top.buildscript!.dependencies!.push(...android_buildScriptDependency);
        proj.app.dependencies!.push(...android_dependency);

        appAndroid.buildTypes.release._extraCode = android_gradleConfigRelease;
        appAndroid.namespace_ = ctx.android.package_id;

        if (signingConfigsPath) {
            const signingConfigs = readJSONFileSync(signingConfigsPath);
            for (const name of Object.keys(signingConfigs)) {
                const config = signingConfigs[name];
                if (config) {
                    checkSigningConfigNames(config);
                    // resolve relative path to store file
                    config.storeFile = path.relative(appModulePath, path.resolve(path.dirname(signingConfigsPath), config.storeFile));
                }
            }
            appAndroid.signingConfigs = signingConfigs;
            if (signingConfigs.release) {
                appAndroid.buildTypes.release.signingConfig = "release";
            }
        } else {
            logger.warn("Android signing configs file not found");
        }

        setupAndroidManifest(ctx, proj);
        setupStringsXML(ctx, proj);

        proj.save(projectPath);

        replaceInFile(path.resolve(projectPath, "fastlane/Appfile"), {
            "__PACKAGE_NAME__": ctx.android.application_id,
            "__SERVICE_ACCOUNT_KEY_PATH__": serviceAccountKey!,
        });
        createMainClass(ctx.android.package_id, appModulePath);
        createCMakeLists(appModulePath, ctx);
    }

    logger.info("Update native build info header");
    ctx.generateNativeBuildInfo();

    // Splash screen
    {
        ensureDirSync(path.join(appModulePath, "src/main/res/drawable"));
        fs.copyFileSync(path.resolve(ctx.sdk.templates, "android-splash/launch_splash.xml"), path.join(appModulePath, "src/main/res/drawable/launch_splash.xml"));
        fs.copyFileSync(path.resolve(ctx.sdk.templates, "android-splash/splash.png"), path.join(appModulePath, "src/main/res/drawable/splash.png"));
    }

    logger.info("Export assets");
    const embeddedAssetsPackName = "assets";
    const embeddedAssetsPackPath = path.join(embeddedAssets, embeddedAssetsPackName);

    await Promise.all([
        buildAssetPackAsync(ctx, embeddedAssetsPackPath),
        buildAppIconAsync({
            projectType: "android",
            output: path.join(appModulePath, "src/main/res"),
            iconPath: ctx.appIcon
        })
    ]);

    logger.info("Do project post-setup..");
    // evaluate post-scripts in with current working dir
    callInDirSync(projectPath, () => {
        for (const fn of ctx.onProjectGenerated) {
            fn();
        }
    });

    // TODO: `build` instead of bundle
    if (ctx.args.indexOf("bundle") >= 0) {
        await gradle(projectPath, 'bundleRelease');
        const aabPath = path.join(projectPath, 'app/build/outputs/bundle/release/app-release.aab');
        if (isFile(aabPath)) {
            fs.copyFileSync(path.join(projectPath, 'app/build/outputs/bundle/release/app-release.aab'),
                path.join(dest_dir, `${ctx.name}_${ctx.version.toString()}.aab`));
        }
    }

    if (ctx.options?.openProject) {
        logger.info("Open Android Studio project at", projectPath);
        openAndroidStudioProject(projectPath);
    }

    if (ctx.options.deploy != null) {
        await execute2("fastlane", [ctx.options.deploy], projectPath);
    }
}
