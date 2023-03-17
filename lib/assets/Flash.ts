import * as path from "path";
import {Asset, AssetDesc} from "./Asset.js";
import {isDir, isFile, removePathExtension} from "../cli/utils.js";
import {flashExport} from "./helpers/flashExport.js";
import {MultiResAtlasAsset} from "./Atlas.js";
import {H} from "../cli/utility/hash.js";
import {logger} from "../cli/logger.js";
import {hashFile, hashGlob} from "./helpers/hash.js";
import {ensureDirSync, writeTextFileSync} from "../utils/utils.js";
import {write_stream_string, write_stream_u32, Writer} from "../../packages/calo/lib/generated/calo.js";

export interface FlashDesc extends AssetDesc {
    filepath: string;
    atlas?: string; // main
}

export class FlashAsset extends Asset {
    static typeName = "flash";

    constructor(readonly desc: FlashDesc) {
        super(desc, FlashAsset.typeName);
        desc.name = desc.name ?? removePathExtension(path.basename(desc.filepath));
    }

    resolveInputs(): number {
        const input = path.resolve(this.owner.basePath, this.desc.filepath);
        let hash = super.resolveInputs();
        if (isDir(input)) {
            hash ^= hashGlob(path.join(input, "**/*"));
        } else if (isFile(input + ".fla")) {
            hash ^= hashFile(input + ".fla");
        }
        return hash;
    }

    _skip = false;
    async build() {
        const targetAtlas = this.desc.atlas ?? "main";
        ensureDirSync(path.join(this.owner.cache, this.desc.name!));
        const configPath = path.join(this.owner.cache, this.desc.name!, "flash_export_config.txt");
        const sgOutput = path.join(this.owner.output, this.desc.name + ".sg");
        const imagesOutput = path.join(this.owner.cache, this.desc.name!, targetAtlas);
        const atlasAsset = this.owner.find(MultiResAtlasAsset.typeName, targetAtlas) as MultiResAtlasAsset | undefined;
        if (atlasAsset) {
            // path <path>
            // output_path <output>
            // output_images_path <outputImages>
            // resolutions_num %u
            // resolution_scale %f
            // ...
            const exportConfig = `${path.resolve(this.owner.basePath, this.desc.filepath)}
${sgOutput}
${imagesOutput}
${atlasAsset.desc.resolutions?.length ?? 0}
${atlasAsset.desc.resolutions?.map(r => `${r.scale}`)?.join("\n")}
`;
            writeTextFileSync(configPath, exportConfig);
            ensureDirSync(imagesOutput);
            await flashExport(configPath);
            atlasAsset.inputs.push(path.join(imagesOutput, "images.txt"));

            // header for .sg file
            this._skip = false;
        } else {
            this._skip = true;
            logger.error(`Atlas target ${targetAtlas} is not found`);
            logger.info("atlases: " + this.owner.map.get(MultiResAtlasAsset.typeName)?.keys());
        }
    }

    writeInfo(w: Writer) {
        write_stream_u32(w, H("scene"));
        write_stream_u32(w, H(this.desc.name!));
        write_stream_string(w, this.desc.name! + ".sg");
    }
}