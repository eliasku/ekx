import {Asset, AssetDesc} from "./Asset.js";
import {H} from "../cli/utility/hash.js";
import {write_stream_u32, Writer} from "../../packages/calo/lib/generated/calo.js";

export interface DynamicAtlasDesc extends AssetDesc {
    name: string;
    alpha_map?: boolean; // false
    mipmaps?: boolean; // false
}

export class DynamicAtlasAsset extends Asset {
    static typeName = "dynamic_atlas";

    constructor(readonly desc: DynamicAtlasDesc) {
        super(desc, DynamicAtlasAsset.typeName);
    }

    _flags = 0;

    build(): null {
        this._flags = 0;
        if (this.desc.alpha_map) this._flags |= 1;
        if (this.desc.mipmaps) this._flags |= 2;
        return null;
    }

    writeInfo(w: Writer) {
        write_stream_u32(w, H(DynamicAtlasAsset.typeName));
        write_stream_u32(w, H(this.desc.name));
        write_stream_u32(w, this._flags);
    }
}