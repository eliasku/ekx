import {collectCppFlags, collectSourceFiles, collectSourceRootsAll, collectStrings} from "../collectSources.js";
import {build as buildCMake} from "../../cmake/mod.js";
import {CMakeGenerateProject, CMakeGenerateTarget, cmakeLists} from "../../cmake/generate.js";
import {isDir} from "../utils.js";
import {Project} from "../project.js";
import {callInDirSync, ensureDirSync, writeTextFileSync} from "../../utils/utils.js";
import * as path from "path";
import * as fs from "fs";

function renderCMakeFile(ctx: Project, buildType: string): string {
    const platforms = ["web"];
    const cppSourceFiles:string[] = [];
    const cppExtensions = ["hpp", "hxx", "h", "hh", "cpp", "cxx", "c", "cc"];
    const cppSourceRoots = collectSourceRootsAll(ctx, "cpp", platforms, ".");
    for (const cppSourceRoot of cppSourceRoots) {
        collectSourceFiles(cppSourceRoot, cppExtensions, cppSourceFiles);
    }
    const cpp_include = collectSourceRootsAll(ctx, "cpp_include", platforms, ".");

    const jsExtensions = ["js"];

    const jsLibraryFiles:string[] = [];
    const js = collectSourceRootsAll(ctx, "js", platforms, ".");
    for (const jsLibraryRoot of js) {
        collectSourceFiles(jsLibraryRoot, jsExtensions, jsLibraryFiles);
    }

    const jsPreFiles:string[] = [];
    const js_pre = collectSourceRootsAll(ctx, "js_pre", platforms, ".");
    for (const jsPreRoot of js_pre) {
        collectSourceFiles(jsPreRoot, jsExtensions, jsPreFiles);
    }

    const cpp_define = collectStrings(ctx, "cpp_define", platforms, false);
    const cpp_lib = collectStrings(ctx, "cpp_lib", platforms, false);
    const cpp_flags = collectCppFlags(ctx, platforms);

    const cmakeTarget: CMakeGenerateTarget = {
        type: "executable",
        libraryType: "static",
        name: ctx.name,
        sources: cppSourceFiles,
        includeDirectories: cppSourceRoots.concat(cpp_include),
        linkLibraries: cpp_lib,
        linkOptions: [
            "-ffast-math",
            "--no_exceptions"
        ],
        compileOptions: [
            "-ffast-math",
            "-fno-exceptions",
            "-fno-rtti",
            "-Wall",
            "-Wextra",
        ],
        compileDefinitions: cpp_define,
        sourceFileCompileFlags: cpp_flags
    };

    const cmakeProject: CMakeGenerateProject = {
        cmakeVersion: "3.19",
        project: ctx.name,
        targets: [cmakeTarget],
        compileOptions: [],
        compileDefinitions: []
    };

    if (buildType === "Release") {
        const releaseExpFlags = [
            // "-fno-stack-protector",
            // "-fno-align-functions",
            "-fno-strict-overflow",
            // "-fno-strict-aliasing"
        ];
        // const releaseExpFlags = ["-fno-vectorize", "-fno-slp-vectorize", "-fno-tree-vectorize", "-fno-unroll-loops"];
        // const releaseExpFlags = [];
        cmakeTarget.linkOptions.push("-Oz", "-flto", ...releaseExpFlags);
        cmakeTarget.compileOptions.push("-Oz", "-flto", ...releaseExpFlags);
        cmakeTarget.linkOptions.push("-g0");
        cmakeTarget.compileOptions.push("-g0");
        if (1) {
            cmakeTarget.linkOptions.push("--profiling-funcs");
            cmakeTarget.compileOptions.push("--profiling-funcs");
        }
        cmakeTarget.compileDefinitions.push("NDEBUG");
    }

    if (buildType === "Debug") {
        cmakeTarget.linkOptions.push("-Oz", "-g");
        cmakeTarget.compileOptions.push("-Oz", "-g");

        //cmakeTarget.linkOptions.push("-Oz", "-gsource-map");
    }

    for (const jsLibraryFile of jsLibraryFiles) {
        cmakeTarget.linkOptions.push(`"SHELL:--js-library \${CMAKE_CURRENT_SOURCE_DIR}/${jsLibraryFile}"`);
    }
    for (const jsPreFile of jsPreFiles) {
        cmakeTarget.linkOptions.push(`"SHELL:--pre-js \${CMAKE_CURRENT_SOURCE_DIR}/${jsPreFile}"`);
    }

    // cmakeTarget.linkOptions.push("-Wl,--import-memory");

    const emOptions: any = {
        ASSERTIONS: buildType === "Debug" ? 2 : 0,
        // DEMANGLE_SUPPORT: buildType === "Debug" ? 1 : 0,
        SAFE_HEAP: buildType === "Debug" ? 1 : 0,
        // SAFE_HEAP_LOG: buildType === "Debug" ? 1 : 0,

        // TODO: strange runtime DOM exception error with Release
        STRICT: 1,

        //MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION: 1,
        // { try ES6
        MINIMAL_RUNTIME: 1,
        MODULARIZE: 1,
        EXPORT_ES6: 1,
        STACK_SIZE: "5MB",
        //EXPORT_NAME: "createModule",
        // }

        SUPPORT_ERRNO: 0,

        // STACK_OVERFLOW_CHECK: 2,
        // ALIASING_FUNCTION_POINTERS: 0,

        FETCH: 0,
        // WASM: 1,
        // WASM_ASYNC_COMPILATION: 1,
        DISABLE_EXCEPTION_CATCHING: 1,

        ALLOW_MEMORY_GROWTH: 1,
        // INITIAL_MEMORY: "128MB",

        MIN_WEBGL_VERSION: 2,
        MAX_WEBGL_VERSION: 2,

        FILESYSTEM: 0,
        INLINING_LIMIT: 1,
        //WASM_OBJECT_FILES: 0,

        ENVIRONMENT: "web",

        DYNAMIC_EXECUTION: 0,
        // AUTO_JS_LIBRARIES: 0,
        // AUTO_NATIVE_LIBRARIES: 0,
        AUTOLOAD_DYLIBS: 0,
        ALLOW_UNIMPLEMENTED_SYSCALLS: 0
    };

    // disable transpiling to ES5
    cmakeTarget.linkOptions.push("--closure 0");

    for (const opt of Object.keys(emOptions)) {
        cmakeTarget.linkOptions.push(`-s${opt}=${emOptions[opt]}`);
    }
    return cmakeLists(cmakeProject);
}

export async function buildWasm(ctx: Project, buildType: string) {
    const platform_proj_name = ctx.name + "-" + ctx.current_target; // "projectName-web"
    const dest_dir = path.resolve(process.cwd(), "export");
    const output_path = path.join(dest_dir, platform_proj_name);

    ensureDirSync(output_path);

    const cmakeFile = callInDirSync(output_path, () => renderCMakeFile(ctx, buildType));
    writeTextFileSync(path.join(output_path, "CMakeLists.txt"), cmakeFile);

    return await buildCMake({
        os: "web",
        buildType: buildType,
        workingDir: output_path
    });
}