import {copyFolderRecursiveSync, execute2, isDir, replaceInFile} from "../utils.js";
import plist from "plist";
import * as path from "path";
import * as fs from "fs";
import {copyFileSync, existsSync} from "fs";
import {buildAssetPackAsync} from "../assets.js";
import {Project} from "../project.js";
import {collectCppFlags, collectObjects, collectStrings} from "../collectSources.js";
import {logger} from "../logger.js";
import {buildAppIconAsync} from "../appicon/appicon.js";
import {
    callInDir,
    ensureDirSync,
    readJSONFileSync,
    readTextFileSync,
    rm,
    writeJSONFileSync,
    writeTextFileSync
} from "../../utils/utils.js";
import {resolveCachePath, resolveEkxPath} from "../../utils/dirs.js";

const iosPlatforms = ["apple", "ios"];

interface AppStoreCredentials {
    team_id?: string;
    apple_id?: string;
    itc_team_id?: string;
    application_specific_password?: string;
}

function mod_plist(ctx: Project, filepath: string) {
    const dict: any = plist.parse(readTextFileSync(filepath));
    dict.CFBundleDisplayName = ctx.title;
    dict.CFBundleShortVersionString = ctx.version.name();
    dict.CFBundleVersion = ctx.version.buildNumber().toString();
    dict.UIRequiresFullScreen = true;
    dict.UIStatusBarHidden = true;
    dict.UIStatusBarStyle = "UIStatusBarStyleDefault";
    dict.UIViewControllerBasedStatusBarAppearance = false;

    if (ctx.orientation === "portrait") {
        dict.UISupportedInterfaceOrientations = [
            "UIInterfaceOrientationPortrait"
        ];
        dict["UISupportedInterfaceOrientations~ipad"] = [
            "UIInterfaceOrientationPortrait",
            "UIInterfaceOrientationPortraitUpsideDown"
        ];
    } else {
        dict.UISupportedInterfaceOrientations = [
            "UIInterfaceOrientationLandscapeLeft",
            "UIInterfaceOrientationLandscapeRight"
        ];
        dict["UISupportedInterfaceOrientations~ipad"] = [
            "UIInterfaceOrientationLandscapeLeft",
            "UIInterfaceOrientationLandscapeRight"
        ];
    }

    const extra_data = collectObjects(ctx, "xcode_plist", iosPlatforms);
    for (const extra of extra_data) {
        for (const [k, v] of Object.entries(extra)) {
            dict[k] = v;
        }
    }
    writeTextFileSync(filepath, plist.build(dict));
}

export async function export_ios(ctx: Project): Promise<void> {

    ctx.generateNativeBuildInfo();

    // setup automation
    let credentials: AppStoreCredentials = {};
    try {
        credentials = readJSONFileSync(ctx.ios.appStoreCredentials!);
    } catch {
        // ignore
    }

    const iconsContents = readJSONFileSync(path.join(ctx.sdk.templates, "xcode-template/src/Assets.xcassets/AppIcon.appiconset/Contents.json"));
    await Promise.all([
        buildAssetPackAsync(ctx),
        buildAppIconAsync({
            projectType: "ios",
            iosAppIconContents: iconsContents,
            output: "export/ios",
            iconPath: ctx.appIcon
        })
    ]);

    const platform_target = ctx.current_target; // "ios"
    const platform_proj_name = ctx.name + "-" + platform_target;
    const dest_dir = path.resolve(process.cwd(), "export");
    const dest_path = path.join(dest_dir, platform_proj_name);

    if (isDir(dest_path)) {
        logger.info("Remove XCode project", dest_path);
        await rm(dest_path);
        logger.assert(!isDir(dest_path));
    }

    copyFolderRecursiveSync(path.join(ctx.sdk.templates, "xcode-template"), dest_path);

    const base_path = "../..";
    await callInDir(dest_path, async () => {
        const embeddedAssetsDir = "assets";
        copyFolderRecursiveSync(path.join(base_path, ctx.getAssetsOutput()), embeddedAssetsDir);
        copyFolderRecursiveSync(path.join(base_path, "export/ios/AppIcon.appiconset"),
            "src/Assets.xcassets/AppIcon.appiconset");

        const src_launch_logo_path = path.join(base_path, "export/ios/AppIcon.appiconset");
        const dest_launch_logo_path = "src/Assets.xcassets/LaunchLogo.imageset";
        // launch logo
        fs.copyFileSync(path.join(src_launch_logo_path, "iphone_40.png"),
            path.join(dest_launch_logo_path, "iphone_40.png"));
        fs.copyFileSync(path.join(src_launch_logo_path, "iphone_80.png"),
            path.join(dest_launch_logo_path, "iphone_80.png"));
        fs.copyFileSync(path.join(src_launch_logo_path, "iphone_120.png"),
            path.join(dest_launch_logo_path, "iphone_120.png"));

        mod_plist(ctx, "src/Info.plist");

        for (const fn of ctx.onProjectGenerated) {
            fn();
        }

        //const xcode_projectPythonPostScript = collectStrings(ctx, "xcode_projectPythonPostScript", iosPlatforms, false);
        const declaration: any = {modules: []};
        for (const module of ctx.modules) {
            declaration.modules.push({
                name: module.name ?? (module.path ? path.basename(module.path) : "global"),

                assets: collectStrings(module, "assets", iosPlatforms, true),

                cpp: collectStrings(module, "cpp", iosPlatforms, true),
                cpp_include: collectStrings(module, "cpp_include", iosPlatforms, true),
                cpp_lib: collectStrings(module, "cpp_lib", iosPlatforms, false),
                cpp_define: collectStrings(module, "cpp_define", iosPlatforms, false),
                cpp_flags: collectCppFlags(module, iosPlatforms),

                xcode_framework: collectStrings(module, "xcode_framework", iosPlatforms, false),
                xcode_capability: collectStrings(module, "xcode_capability", iosPlatforms, false),
                xcode_plist: collectObjects(module, "xcode_plist", iosPlatforms),
                xcode_file: collectStrings(module, "xcode_file", iosPlatforms, false),
            });
        }
        declaration.modules.push({name: "embedded", assets: [embeddedAssetsDir]});

        let projectDeclaration = {
            name: "app-ios",
            options: {
                deploymentTarget: {
                    iOS: "12.0"
                },
                usesTabs: false,
                tabWidth: 2,
                indentWidth: 2,
                defaultConfig: "Release"
            },
            settings: {
                DEVELOPMENT_TEAM: "696L2V4X5J"
            },
            targets: {
                "app-ios": {
                    type: "application",
                    platform: "iOS",
                    sources: [
                        {name: "App", path: "src", excludes: []},
                        {name: "Assets", path: embeddedAssetsDir, excludes: [".inputs_checksum"], type: "folder"},
                    ] as any[],
                    dependencies: [] as any,
                    settings: {
                        base: {
                            PRODUCT_BUNDLE_IDENTIFIER: ctx.ios.application_id!,
                            INFOPLIST_FILE: "src/Info.plist",
                            HEADER_SEARCH_PATHS: ["$(inherited)"],
                            GCC_ENABLE_CPP_EXCEPTIONS: "NO",
                            GCC_ENABLE_CPP_RTTI: "NO",
                            GCC_ENABLE_OBJC_EXCEPTIONS: "NO",
                            GCC_C_LANGUAGE_STANDARD: "c11",
                            CLANG_CXX_LANGUAGE_STANDARD: "c++17",
                            CLANG_CXX_LIBRARY: "libc++",
                            OTHER_CFLAGS: ["$(inherited)"],
                            LD_RUNPATH_SEARCH_PATHS: [
                                "$(inherited)",
                                "@executable_path/Frameworks",
                                "/usr/lib/swift",
                            ],
                        },
                        configs: {
                            Debug: {
                                ENABLE_BITCODE: false
                            },
                            Release: {
                                GCC_PREPROCESSOR_DEFINITIONS: [
                                    "$(inherited)",
                                    "NDEBUG"
                                ],
                                GCC_OPTIMIZATION_LEVEL: "z",
                                LLVM_LTO: "YES",
                            }
                        }
                    },
                    attributes: {
                        SystemCapabilities: {} as any,
                    },
                },
            }
        };

        const target = projectDeclaration.targets["app-ios"];
        const defines: string[] = [];
        for (const module of ctx.modules) {
            for (const cpp_flags of collectCppFlags(module, iosPlatforms)) {
                for (const cpp_file of cpp_flags.files) {
                    target.sources.push({path: cpp_file, compilerFlags: cpp_flags.flags});
                }
            }
            for (const cpp of collectStrings(module, "cpp", iosPlatforms, true)) {
                target.sources.push({
                    name: module.name,
                    path: cpp,
                    excludes: [
                        "**/build",
                        "**/CMakeLists.txt",
                        "**/.DS_Store",
                        "**/*.md",
                        "**/*.js",
                        "**/*.ts",
                        "**/*.glsl",
                        "**/*.sh",
                    ],
                });
                target.settings.base.HEADER_SEARCH_PATHS.push(cpp);
            }
            for (const xcode_file of collectStrings(module, "xcode_file", iosPlatforms, false)) {
                target.sources.push({path: xcode_file});
            }
            for (const cpp_include of collectStrings(module, "cpp_include", iosPlatforms, true)) {
                target.settings.base.HEADER_SEARCH_PATHS.push(cpp_include);
            }
            for (const capability of collectStrings(module, "xcode_capability", iosPlatforms, false)) {
                target.attributes.SystemCapabilities[capability] = {enabled: 1};
            }
            for (const framework of collectStrings(module, "xcode_framework", iosPlatforms, false)) {
                if (!(target.dependencies as any[]).find(d => d.framework === framework)) {
                    target.dependencies.push({
                        framework: framework,
                        embed: false,
                        link: false,
                    });
                }
            }
            for (const def of collectStrings(module, "cpp_define", iosPlatforms, false)) {
                defines.push(def);
            }
            for (const lib of collectStrings(module, "cpp_lib", iosPlatforms, false)) {
                //target.attributes.SystemCapabilities[capability] = {enabled: 1};
            }

            collectStrings(module, "cpp_lib", iosPlatforms, false)
        }

        target.settings.base.OTHER_CFLAGS.push(...defines.map(d => `-D${d.replaceAll(`"`, `\\"`)}`));

        writeJSONFileSync("project.json", projectDeclaration);

        {
            const xcgDir = resolveCachePath("bin/xcg");
            const xcgPackage = path.join(xcgDir, "Package.swift");
            if (!existsSync(xcgPackage)) {
                ensureDirSync(xcgDir);
                copyFileSync(resolveEkxPath("lib/cli/_templates/xcode-proj-gen/Package.swift"), xcgPackage);
            }
            await execute2("swift", ["run", "--package-path", xcgDir, "xcodegen", "generate", "--spec", "project.json"]);
        }

        logger.info("Prepare PodFile");
        let pods = collectStrings(ctx, "podfile_pod", iosPlatforms, false)
            .map((v) => `pod '${v}'`).join("\n  ");
        pods += "\n\n" + collectStrings(ctx, "podfile_code", iosPlatforms, false).join("\n  ")
        replaceInFile("Podfile", {
            "# TEMPLATE DEPENDENCIES": pods
        });

        replaceInFile("fastlane/Appfile", {
            "[[APP_IDENTIFIER]]": ctx.ios.application_id!,
            "[[APPLE_ID]]": credentials.apple_id ?? "",
            "[[TEAM_ID]]": credentials.team_id ?? "",
            "[[ITC_TEAM_ID]]": credentials.itc_team_id ?? "",
        });

        logger.info("Install Pods");
        if (ctx.args.indexOf("clean") < 0 || 0 !== await execute2("pod", ["install", "--repo-update"])) {
            // maybe no internet connection, so we can't update pods repo
            await execute2("pod", ["install"]);
        }
    });

    if (ctx.options.openProject) {
        const workspace_path = path.join(dest_path, "app-ios.xcworkspace");
        await execute2("open", [workspace_path]);
        // execute("xcodebuild", [
        //     "-workspace", workspace_path,
        //     "-scheme", platform_proj_name,
        //     "-configuration", "Release"
        // ]);
    }

    if (ctx.options.deploy != null) {
        await execute2("fastlane", [ctx.options.deploy], dest_path, {
            FASTLANE_APPLE_APPLICATION_SPECIFIC_PASSWORD: credentials.application_specific_password!
        });
    }
}