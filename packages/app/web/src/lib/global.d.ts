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
        readonly GLctx: WebGL2RenderingContext;
    }

    interface EmscriptenGLAttributes extends WebGLContextAttributes {
        minorVersion?: number;
        majorVersion?: number;
        enableExtensionsByDefault?: boolean;
    }

    interface EmscriptenGL {
        readonly currentContext: EmscriptenGLContext;
        registerContext: (gl: WebGL2RenderingContext, attributes: EmscriptenGLAttributes) => number;
        makeContextCurrent: (handle: number) => boolean;
    }

    const GL: EmscriptenGL;

    const App: {
        getTextureLoader: (id: number) => Loader | null,
        loadTexture: (request: LoadRequest) => number,
        destroyTextureLoader: (id: number) => number,
    };
}