import {iMask, tMask, u31, vIncr, vMask} from "../protocol/interface.js";

export interface Obj {
    h: u31;
    s: u31;
}

export function nextHandle(h: u31) {
    return ((h + vIncr) & vMask) | (h & (tMask | iMask));
}

export function connectAudioNode(node: AudioNode, dest: AudioNode) {
    node.connect(dest);
}

export function disconnectAudioNode(node: AudioNode, dest?: AudioNode) {
    node.disconnect(dest!);
}

export function setAudioParamValue(param: AudioParam, value: number) {
    param.value = value;
}
