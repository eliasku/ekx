import {build} from "../modules/cmake/mod.js";
import {UtilityConfig} from "../modules/cli/utils.js";
import {resolveCachePath, resolveEkxPath} from "../modules/utils/dirs.js";

UtilityConfig.verbose = true;

// build main packages, plugins and tools
await build({
    definitions: {
        EKX_BUILD_DEV_TOOLS: "ON",
        EKX_BUILD_TESTS: "ON",
        EKX_BUILD_COVERAGE: "OFF",
        EKX_BUILD_EXTERNAL_TESTS: "OFF",
        EKX_INCLUDE_EXAMPLES: "ON",
        EKX_INCLUDE_PLUGINS: "ON",
    },
    test: true,
    // set main EKX root dir for `CMakeFiles.txt`
    workingDir: resolveEkxPath(),
    // place build files in central cache folder
    buildsFolder: resolveCachePath("builds/sdk/release"),
});
