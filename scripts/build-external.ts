import box2d from "./external-recipes/box2d.js";
import chipmunk2d from "./external-recipes/chipmunk2d.js";
import cairo from "./external-recipes/cairo.js";
import freetype from "./external-recipes/freetype/freetype.js";
import headers from "./external-recipes/headers.js";
import quickjs from "./external-recipes/quickjs.js";
import pugixml from "./external-recipes/pugixml.js";
import miniz from "./external-recipes/miniz.js";
import imgui from "./external-recipes/imgui.js";
import {logger} from "../lib/cli/logger.js";
import {UtilityConfig} from "../lib/cli/utils.js";

UtilityConfig.verbose = true;

type LibFn = undefined | (() => (undefined | void | Promise<void>));

interface Lib {
    clean?: LibFn;
    fetch?: LibFn;
    test?: LibFn;
}

const libs: { [key: string]: Lib } = {
    headers,
    chipmunk2d,
    box2d,
    cairo,
    freetype,
    quickjs,
    pugixml,
    miniz,
    imgui,
};

async function runSerialTasks(task: string) {
    for (const name of Object.keys(libs)) {
        const lib: Lib = libs[name];
        if (task in lib && (lib as any)[task] != null) {
            const taskFn = (lib as any)[task];
            if (taskFn != null) {
                try {
                    const promise = taskFn();
                    if (promise instanceof Promise) {
                        await promise.catch(e => {
                            logger.error(`${task} failed for ${name}:`, e);
                        });
                    }
                } catch (e) {
                    logger.error(e);
                }
            }
        }
    }
}

function runParallelTasks(task: string) {
    const tasks = [];
    for (const name of Object.keys(libs)) {
        const lib: Lib = libs[name];
        if (task in lib && (lib as any)[task] != null) {
            const taskFn = (lib as any)[task];
            if (taskFn != null) {
                try {
                    const promise = taskFn();
                    if (promise instanceof Promise) {
                        tasks.push(promise.catch(e => {
                            logger.error(`${task} failed for ${name}:`, e);
                        }));
                    }
                } catch (e) {
                    logger.error(e);
                }
            }
        }
    }
    return Promise.allSettled(tasks);
}

await runParallelTasks("clean");
await runSerialTasks("fetch");
await runSerialTasks("test");
