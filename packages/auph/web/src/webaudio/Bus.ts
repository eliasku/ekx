import {getAudioContextObject} from "./Mixer.js";
import {AuphBus, Flag, iMask, Type, Unit} from "../protocol/interface.js";
import {connectAudioNode, disconnectAudioNode, Obj} from "./common.js";

export class BusObj implements Obj {
    h = 0;
    s = Flag.Active | Flag.Running;
    G = Unit;

    constructor(readonly g: GainNode) {
    }
}

export const busLine: BusObj[] = [];

export function createBusObj(ctx: AudioContext): BusObj {
    const next = busLine.length;
    const obj = new BusObj(ctx.createGain());
    obj.h = next | Type.Bus;
    busLine.push(obj);
    return obj;
}

export function initBusPool(ctx: AudioContext) {
    const master = createBusObj(ctx).g;
    connectAudioNode(master, ctx.destination);
    connectAudioNode(createBusObj(ctx).g, master);
    connectAudioNode(createBusObj(ctx).g, master);
    connectAudioNode(createBusObj(ctx).g, master);
}

export function termBusPool() {
    for (let i = 0; i < busLine.length; ++i) {
        disconnectAudioNode(busLine[i].g);
    }
    busLine.length = 0;
}

export function _getBus(bus: AuphBus): BusObj | null {
    const obj = busLine[bus & iMask];
    return (obj && obj.h === bus) ? obj : null;
}

export function _getBusGain(handle: AuphBus): GainNode | undefined {
    const obj = _getBus(handle);
    return obj ? obj.g : undefined;
}

export function _setBusConnected(bus: BusObj, connected: boolean): void {
    const flag = !!(bus.s & Flag.Running);
    if (flag !== connected) {
        const master = busLine[0];
        const dest = bus === master ? getAudioContextObject()!.destination : master.g;
        if (connected) {
            connectAudioNode(bus.g, dest);
        } else {
            disconnectAudioNode(bus.g, dest);
        }
        bus.s ^= Flag.Running;
    }
}