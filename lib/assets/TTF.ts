import * as path from "path";
import * as fs from "fs";
import {Asset, AssetDesc} from "./Asset.js";
import {removePathExtension} from "../cli/utils.js";
import {H} from "../cli/utility/hash.js";
import {hashFile} from "./helpers/hash.js";
import {logger} from "../cli/logger.js";
import {ensureDirSync} from "../utils/utils.js";
import {
    write_stream_f32,
    write_stream_string,
    write_stream_u32,
    Writer
} from "../../packages/calo/lib/generated/calo.js";

export interface TTFImporterDesc extends AssetDesc {
    filepath: string;
    glyph_cache?: string;
    base_font_size?: number;
}

export class TTFAsset extends Asset {
    static typeName = "ttf";

    constructor(readonly desc: TTFImporterDesc) {
        super(desc, TTFAsset.typeName);
        desc.name = desc.name ?? removePathExtension(path.basename(desc.filepath));
    }

    resolveInputs(): number {
        const filepath = path.resolve(this.owner.basePath, this.desc.filepath);
        try {
            const hash = hashFile(filepath);
            return hash ^ super.resolveInputs();
        } catch (err) {
            logger.warn("file not found:", filepath);
            throw err;
        }
    }

    build(): null {
        const outputPath = path.join(this.owner.output, this.desc.name + ".ttf");
        ensureDirSync(path.dirname(outputPath));
        fs.copyFileSync(path.resolve(this.owner.basePath, this.desc.filepath), outputPath);
        return null;
    }

    writeInfo(w: Writer) {
        write_stream_u32(w, H(this.typeName));
        write_stream_u32(w, H(this.desc.name!));
        write_stream_string(w, this.desc.name + ".ttf");
        write_stream_u32(w, H(this.desc.glyph_cache ?? "default_glyph_cache"));
        write_stream_f32(w, this.desc.base_font_size ?? 48);
    }
}