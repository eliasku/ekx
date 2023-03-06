import {build} from "../lib/cmake/mod.js";
import {UtilityConfig} from "../lib/cli/utils.js";
import {resolveCachePath, resolveEkxPath} from "../lib/utils/dirs.js";

UtilityConfig.verbose = true;

await build({
    definitions: {
        EKX_BUILD_DEV_TOOLS: "OFF",
        EKX_BUILD_TESTS: "ON",
        EKX_BUILD_COVERAGE: "ON",
        EKX_BUILD_EXTERNAL_TESTS: "OFF",
        EKX_INCLUDE_EXAMPLES: "OFF",
        EKX_INCLUDE_PLUGINS: "OFF",
    },
    buildType: "Profiling",
    target: "coverage",
    // set main EKX root dir for `CMakeFiles.txt`
    workingDir: resolveEkxPath(),
    // place build files in central cache folder
    buildsFolder: resolveCachePath("builds/sdk/coverage"),
});
