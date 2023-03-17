import * as path from "path";
import {Asset, AssetDesc} from "./Asset.js";
import {H} from "../cli/utility/hash.js";
import {hashGlob} from "./helpers/hash.js";
import {ensureDirSync, expandGlobSync} from "../utils/utils.js";
import {msgfmt} from "./helpers/msgfmt.js";
import {
    write_stream_static_string,
    write_stream_string,
    write_stream_u32,
    Writer
} from "../../packages/calo/lib/generated/calo.js";

export interface TranslationsDesc extends AssetDesc {
    filepath: string
}

export class TranslationsAsset extends Asset {
    static typeName = "translations";

    readonly languages = new Map<string, string>();

    constructor(readonly desc: TranslationsDesc) {
        super(desc, TranslationsAsset.typeName);
        desc.name = desc.name ?? path.basename(desc.filepath);
    }

    resolveInputs(): number {
        return super.resolveInputs() ^ hashGlob(path.join(this.owner.basePath, this.desc.filepath, "*.po"));
    }

    async build() {
        const files = expandGlobSync(path.join(this.owner.basePath, this.desc.filepath, "*.po"));
        for (const file of files) {
            this.languages.set(path.basename(file.path, ".po"), path.join(this.owner.basePath, this.desc.filepath, path.basename(file.path)));
        }

        const outputPath = path.join(this.owner.output, this.desc.name!);
        ensureDirSync(outputPath);

        this._langs = [];
        for (const [lang, filepath] of this.languages) {
            this._langs.push(lang);
            await msgfmt(filepath, path.join(outputPath, lang) + ".mo");
        }
    }

    _langs: string[] = [];

    writeInfo(w: Writer) {
        write_stream_u32(w, H("strings"));
        write_stream_string(w, this.desc.name!);
        write_stream_u32(w, this._langs.length);
        for (const l of this._langs) {
            write_stream_static_string(w, l, 8);
        }
    }
}
