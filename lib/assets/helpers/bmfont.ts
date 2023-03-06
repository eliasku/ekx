import {execute} from "../../cli/utils.js";
import {getOrBuildUtility} from "../../cli/utility/bin.js";

export async function bmfont(configPath: string): Promise<number> {
    const bin = await getOrBuildUtility("ekc");
    return await execute(bin, ["bmfont-export", configPath]);
}