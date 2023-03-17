import * as path from "path";
import * as fs from "fs";
import {Asset} from "./Asset.js";
import {removePathExtension} from "../cli/utils.js";
import {H} from "../cli/utility/hash.js";
import {hashFile} from "./helpers/hash.js";
import {ensureDirSync} from "../utils/utils.js";
import {write_stream_string, write_stream_u32, Writer} from "../../packages/calo/lib/generated/calo.js";

export interface AudioFile {
    filepath: string;
    name?: string; // filepath without extension
    lazy?: boolean; // true
    dev?: boolean; // false

    // audio specific
    streaming?: boolean; // false
}

export class AudioAsset extends Asset {
    static typeName = "audio";

    constructor(readonly desc: AudioFile) {
        super(desc, AudioAsset.typeName);
        const streaming = this.desc.streaming ?? false;
        const lazy = this.desc.lazy ?? true;
        this.priority = 100;
        if (streaming) {
            this.priority += 1;
        }
        if (lazy) {
            this.priority += 1;
        }
    }

    resolveInputs(): number {
        return hashFile(path.resolve(this.owner.basePath, this.desc.filepath)) ^
            super.resolveInputs();
    }

    _skip = false;
    _name = "";
    _flags = 0;
    build(): null {
        const outputPath = path.join(this.owner.output, this.desc.filepath);
        const inputPath = path.join(this.owner.basePath, this.desc.filepath);
        ensureDirSync(path.dirname(outputPath));

        fs.copyFileSync(inputPath, outputPath);
        this._name = this.desc.name ?? removePathExtension(this.desc.filepath);
        const streaming = this.desc.streaming ?? false;
        const lazy = this.desc.lazy ?? true;
        this._flags = 0;
        if (streaming) {
            this._flags |= 1;
        }
        if (lazy) {
            this._flags |= 2;
        }

        const dev = this.desc.dev ?? false;
        this._skip = dev && !this.owner.devMode;

        return null;
    }

    writeInfo(w: Writer) {
        if(this._skip) {
            return;
        }
        write_stream_u32(w, H("audio"));
        write_stream_u32(w, H(this._name));
        write_stream_u32(w, this._flags);
        write_stream_string(w, this.desc.filepath);
    }
}