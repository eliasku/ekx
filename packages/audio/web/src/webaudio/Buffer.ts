import {AuphBuffer, Flag, iMask, Message, Type, u31} from "../protocol/interface.js";
import {error} from "./debug.js";
import {nextHandle, Obj} from "./common.js";

export interface CallbackBuffer {
    b: AudioBuffer;
}

export type BufferData = string | AudioBuffer | null;

export class BufferObj implements Obj {
    constructor(public h: u31,
                public s: u31,
                public b: BufferData,
                // HEAP pointer to C-callback
                public _f: u31 = 0,
                // HEAP pointer to C-userdata
                public _u: u31 = 0) {
    }
}

export let buffers: (BufferObj | null)[] = [null];
const buffersMaxCount = 128;

export function getNextBufferObj(): AuphBuffer | 0 {
    const next = buffers.length;
    for (let i = 1; i < next; ++i) {
        const buffer = buffers[i]!;
        if (buffer.s === 0) {
            return buffer.h;
        }
    }
    if (next < buffersMaxCount) {
        const b = new BufferObj(next | Type.Buffer, 0, null);
        buffers.push(b);
        return b.h;
    }
    return 0;
}

export function _bufferDestroy(obj: BufferObj) {
    // TODO:
    // if ((obj.s & Flag.Stream) !== 0 && obj.data) {
    //     URL.revokeObjectURL(obj.b as string);
    // }
    obj.h = nextHandle(obj.h);
    obj.s = 0;
    obj.b = null;
}

export function _getBufferObj(buffer: AuphBuffer): BufferObj | null {
    const obj = buffers[buffer & iMask]!;
    if (obj && obj.h === buffer) {
        return obj;
    }
    return null;
}

function _decodeAudioData(ctx: AudioContext, obj: BufferObj, buffer: ArrayBuffer) {
    const success = (audioBuffer: AudioBuffer): void => {
        obj.s |= Flag.Loaded;
        obj.b = audioBuffer;
    };
    const fail = (err?: DOMException): void => {
        error(Message.BufferDecodeError, err);
        _bufferDestroy(obj);
    };
    // TODO: maybe callbacks will be deprecated?
    ctx.decodeAudioData(buffer, success, fail);
}

export function _bufferMemory(obj: BufferObj, ctx: AudioContext, data: Uint8Array, flags: u31) {
    obj.s |= Flag.Active;
    const buffer = data.buffer.slice(data.byteOffset, data.byteOffset + data.byteLength);
    // TODO: streaming
    // if (flags & Flag.Stream) {
    //     obj.s |= Flag.Stream;
    _decodeAudioData(ctx, obj, buffer);
}

export function _buffer_set_callback(obj: BufferObj, ctx: AudioContext, f: u31, u: u31) {
    obj.s |= Flag.Active | Flag.Loaded | Flag.Callback;
    obj.b = new AudioBuffer({length: 8192, sampleRate: 44100, numberOfChannels: 2});
    obj._f = f;
    obj._u = u;
}

export function _bufferLoad(obj: BufferObj, ctx: AudioContext, filepath: string, flags: u31) {
    obj.s |= Flag.Active;
    // TODO: streaming
    //if (flags & Flag.Stream) {
    //obj.s |= Flag.Stream;

    fetch(new Request(filepath))
        .then(response => response.arrayBuffer())
        .then(buffer => _decodeAudioData(ctx, obj, buffer))
        .catch((reason) => {
            error(Message.BufferLoadError, reason);
            _bufferDestroy(obj);
        });
}