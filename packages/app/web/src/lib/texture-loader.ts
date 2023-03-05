import type {LoadRequest} from "../shared/texture-loader";

export const LibTextureLoader = {
    texloader_create__deps: ['$GL'],
    texloader_create: (pUrls: number, numImages: number, pBasePath: number, pVersion: number, flags: number, formatMask: number): number => {
        const req: LoadRequest = {
            urls: [],
            formatMask,
            flags,
            gl: GL.currentContext.GLctx
        };
        if (pBasePath) {
            req.basePath = UTF8ToString(pBasePath);
        }
        if (pVersion) {
            req.version = UTF8ToString(pVersion);
        }

        for (let i = 0; i < numImages; ++i) {
            const pUrl = HEAPU32[(pUrls >> 2) + i];
            if (pUrl) {
                req.urls[i] = UTF8ToString(pUrl);
            }
        }

        return App.loadTexture(req);
    },
    texloader_get: (id: number, pState: number): number => {
        const obj = App.getTextureLoader(id);
        if (obj && pState) {
            let i = pState >>> 2;
            HEAP32[i++] = obj.progress;
            HEAP32[i++] = obj.w;
            HEAP32[i++] = obj.h;
            HEAP32[i++] = obj.texture != null ? 2 : 1;
            HEAP32[i++] = obj.textureID;
            return 0;
        }
        return 1;
    },
    texloader_destroy: (id: number): number => {
        return App.destroyTextureLoader(id);
    },
};
