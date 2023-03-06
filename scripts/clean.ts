import {rm} from "../lib/utils/utils.js";
import {UtilityConfig} from "../lib/cli/utils.js";

UtilityConfig.verbose = true;

await rm("plugins/firebase/web/dist");
await rm("plugins/firebase/web/lib");
await rm("packages/auph/web/dist");
await rm("packages/app/js");
await rm("build");