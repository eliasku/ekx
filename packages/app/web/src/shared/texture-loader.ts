export const enum Flags {
    PremultiplyAlpha = 1,
    CubeMap = 2
}

export interface LoadRequest {
    basePath?: string;
    urls: string[];
    version?: string;
    flags: Flags;
    gl?: WebGLRenderingContext;
    formatMask: number;
}
