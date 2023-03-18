import {rm} from "../lib/utils/utils.js";
import {UtilityConfig} from "../lib/cli/utils.js";

UtilityConfig.verbose = true;

await rm("plugins/firebase/js");
await rm("packages/auph/js");
await rm("packages/app/js");
await rm("cache");
