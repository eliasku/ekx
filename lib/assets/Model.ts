import * as path from "path";
import {Asset, AssetDesc} from "./Asset.js";
import {removePathExtension} from "../cli/utils.js";
import {objExport} from "./helpers/objExport.js";
import {H} from "../cli/utility/hash.js";
import {hashFile} from "./helpers/hash.js";
import {ensureDirSync} from "../utils/utils.js";
import {write_stream_string, write_stream_u32, Writer} from "../../packages/calo/lib/generated/calo.js";

export interface ObjImporterDesc extends AssetDesc {
    filepath: string;
}

export class ModelAsset extends Asset {
    static typeName = "model";

    constructor(readonly desc: ObjImporterDesc) {
        super(desc, ModelAsset.typeName);
        this.desc.name = this.desc.name ?? removePathExtension(path.basename(desc.filepath));
    }

    resolveInputs(): number {
        return super.resolveInputs() ^ hashFile(path.resolve(this.owner.basePath, this.desc.filepath));
    }

    async build() {
        // TODO: change ext to `m3d`
        const outputPath = path.join(this.owner.output, this.desc.name + ".model");
        const inputPath = path.join(this.owner.basePath, this.desc.filepath);

        ensureDirSync(path.dirname(outputPath));

        await objExport(inputPath, outputPath);
    }

    writeInfo(w: Writer) {
        write_stream_u32(w, H(ModelAsset.typeName));
        const name = this.desc.name!;
        write_stream_string(w, name);
        write_stream_u32(w, H(name));
    }
}