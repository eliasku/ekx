import * as path from "path";
import * as fs from "fs";
import {Asset, AssetDesc} from "./Asset.js";
import {compress, WebpConfig} from "./helpers/webp.js";
import {H} from "../cli/utility/hash.js";
import {hashFile} from "./helpers/hash.js";
import {ensureDirSync} from "../utils/utils.js";
import {write_stream_u32, Writer} from "../../packages/calo/lib/generated/calo.js";
import {image_data, image_data_type, write_stream_image_data} from "../../packages/calo/lib/generated/gen_sg.js";

export interface TextureImporterDesc extends AssetDesc {
    name: string;
    type?: image_data_type; // Normal
    images: string[];
    webp?: WebpConfig;
}

export class TextureAsset extends Asset {
    static typeName = "texture";

    constructor(readonly desc: TextureImporterDesc) {
        super(desc, TextureAsset.typeName);
    }

    resolveInputs(): number {
        let hash = super.resolveInputs();
        for (const imagePath of this.desc.images) {
            hash ^= hashFile(path.join(this.owner.basePath, imagePath));
        }
        return hash;
    }

    async build() {
        if (this.owner.project.current_target === "ios") {
            this.desc.webp = undefined;
        }

        for (const imagePath of this.desc.images) {
            const srcFilePath = path.join(this.owner.basePath, imagePath);
            const destFilepath = path.join(this.owner.output, imagePath);
            ensureDirSync(path.dirname(destFilepath));
            fs.copyFileSync(srcFilePath, destFilepath);
        }
        if (this.desc.webp) {
            const promises = [];
            for (const imagePath of this.desc.images) {
                const filepath = path.join(this.owner.output, imagePath);
                promises.push(compress(filepath, this.desc.webp));
            }
            await Promise.all(promises);
            if (this.owner.project.current_target === "android") {
                for (const imagePath of this.desc.images) {
                    const filepath = path.join(this.owner.output, imagePath);
                    fs.rmSync(filepath);
                }
            }
        }
        // variants
        this._data.format_mask = 1;
        if (this.desc.webp) {
            if (this.owner.project.current_target === "android") {
                this._data.format_mask = 0;
            }
            this._data.format_mask |= 2;
        }
        this._data.type = this.desc.type === image_data_type.IMAGE_DATA_NORMAL ? 0 : 1;
        this._data.images = this.desc.images;
    }

    _data: image_data = {
        type: 0,
        format_mask: 0,
        images: [],
    };

    writeInfo(w: Writer) {
        write_stream_u32(w, H(this.typeName));
        write_stream_u32(w, H(this.desc.name));
        write_stream_image_data(w, this._data);
    }
}