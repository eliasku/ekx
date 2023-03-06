import {execute} from "../../cli/utils.js";
import {getOrBuildUtility} from "../../cli/utility/bin.js";

export async function objExport(input: string, output: string): Promise<number> {
    const bin = await getOrBuildUtility("ekc");
    return await execute(bin, ["obj", input, output]);
}