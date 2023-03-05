import {LoadRequest} from "../shared/texture-loader";

declare global {
    class LibraryManager {
        static library: LibraryManager;
    }

    function mergeInto(library: LibraryManager, module: any): void;

    const HEAPU8: Uint8Array;
    const HEAP32: Int32Array;
    const HEAPU32: Uint32Array;
    const HEAPF64: Float64Array;

    function UTF8ToString(ptr: number): string;

    function stringToUTF8(str: string, pDest: number, maxLength: number): number;

    interface EmscriptenGLContext {
        readonly GLctx: WebGLRenderingContext;
    }

    interface EmscriptenGLAttributes extends WebGLContextAttributes {
        majorVersion?: number;
        enableExtensionsByDefault?: boolean;
    }

    interface EmscriptenGL {
        readonly currentContext: EmscriptenGLContext;
        registerContext: (gl: WebGLRenderingContext, attributes: EmscriptenGLAttributes) => number;
        makeContextCurrent: (handle: number) => boolean;
    }

    const GL: EmscriptenGL;

    const App: {
        getTextureLoader: (id: number) => Loader | null,
        loadTexture: (request: LoadRequest) => number,
        destroyTextureLoader: (id: number) => number,
    };
}