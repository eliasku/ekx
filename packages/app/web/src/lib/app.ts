declare const _ek_app_js__loop: (ts: number) => void;
declare const _ek_app_js__on_focus: (flags: number) => void;
declare const _ek_app_js__on_key: (type: number, code: number, flags: number) => number;
declare const _ek_app_js__on_resize: (drawable_width: number, drawable_height: number, dpr: number) => void;

declare const _ek_app_js__on_touch: (type: number, id: number, x: number, y: number) => boolean;
declare const _ek_app_js__on_mouse: (type: number, button: number, x: number, y: number) => boolean;
declare const _ek_app_js__on_wheel: (x: number, y: number) => boolean;

export const AppLib = {
    ek_app_js_set_mouse_cursor: function (cursor: number): void {
        const PARENT = 0;
        const ARROW = 1;
        const BUTTON = 2;
        const HELP = 3;
        const map: string[] = [
            "auto", // 0
            "default", // 1
            "pointer", // 2
            "help" // 3
        ];
        cursor = cursor | 0;
        if (cursor >= 0 && cursor < map.length) {
            const gameview = document.getElementById("gameview");
            if (gameview) {
                gameview.style.cursor = map[cursor];
            }
        }
    },

    ek_app_js_init__deps: ['$GL'],

    ek_app_js_init: function (flags: number) {
        const BUTTONS: number[] = [0, 2, 1, 2, 2];

        const TYPES: Record<string, number> = {
            "keydown": 14,
            "keyup": 15,
            "keypress": 16,

            "mousemove": 8,
            "mousedown": 9,
            "mouseup": 10,
            "wheel": 13,
            "touchstart": 5,
            "touchend": 7,
            "touchcancel": 7,
            "touchmove": 6,
        };
        const KEY_CODES: Record<string, number> = {
            "ArrowUp": 1,
            "ArrowDown": 2,
            "ArrowLeft": 3,
            "ArrowRight": 4,
            "Escape": 5,
            "Space": 6,
            "Enter": 7,
            "Backspace": 8,
            "KeyA": 16,
            "KeyC": 17,
            "KeyV": 18,
            "KeyX": 19,
            "KeyY": 20,
            "KeyZ": 21,
            "KeyW": 22,
            "KeyS": 23,
            "KeyD": 24,
        };

        const onKey = (event: KeyboardEvent) => {
            const type = TYPES[event.type];
            if (type) {
                const code = KEY_CODES[event.code];
                if (code) {
                    if (_ek_app_js__on_key(type, code, 0)) {
                        event.preventDefault();
                    }
                }
            }
        };

        const wnd = window;
        wnd.addEventListener("keypress", onKey, true);
        wnd.addEventListener("keydown", onKey, true);
        wnd.addEventListener("keyup", onKey, true);

        const handleResize = () => {
            const dpr = (flags & 8) ? 1 : window.devicePixelRatio;

            const div = document.getElementById("gamecontainer")!;
            const rc = div.getBoundingClientRect();
            const css_w = rc.width;
            const css_h = rc.height;

            // TODO: configurable min aspect (70/100)
            // TODO: landscape and different modes, native letterbox
            const w = css_w;
            const h = css_h;
            const offset_x = 0;
            const offset_y = 0;

            // TODO:
            //if (webKeepCanvasAspectRatio) {
            //var aspect = ek_app.config.width / ek_app.config.height;
            //if (aspect > 1.0) {
            //    if (w / aspect < h) {
            //        h = w / aspect;
            //    }
            //    offset_y = (css_h - h) / 2;
            //} else {
            //    if (h * aspect < w) {
            //        w = h * aspect;
            //    }
            //    offset_x = (css_w - w) / 2;
            //}
            //}

            const drawableWidth = (w * dpr) | 0;
            const drawableHeight = (h * dpr) | 0;

            _ek_app_js__on_resize(drawableWidth, drawableHeight, dpr);

            const gameview = document.getElementById("gameview") as HTMLCanvasElement | null;
            if (gameview) {
                if (gameview.width !== drawableWidth ||
                    gameview.height !== drawableHeight) {

                    gameview.width = drawableWidth;
                    gameview.height = drawableHeight;

                    gameview.style.width = w + "px";
                    gameview.style.height = h + "px";
                }
                gameview.style.transform = "translateX(" + offset_x + "px) translateY(" + offset_y + "px)";
            }
        };

        // callback call after timeout after last call (if call again before timeout,
        // planned callback is cancelled and re-scheduled to be called after timeout)
        const throttle = (callback: () => void, millisecondsLimit: number) => {
            let timer: any = -1;
            return () => {
                if (timer >= 0) {
                    clearTimeout(timer);
                }
                timer = setTimeout(() => {
                    timer = -1;
                    callback();
                }, millisecondsLimit);
            }
        };

        wnd.addEventListener("resize", throttle(handleResize, 100), true);
        handleResize();

        const onTouch = (e: TouchEvent) => {
            const type = TYPES[e.type];
            if (type) {
                // https://developer.mozilla.org/en-US/docs/Web/API/MouseEvent/button
                const rect = (e.target as HTMLElement).getBoundingClientRect();
                let cancelDefault = false;
                for (let i = 0; i < e.changedTouches.length; ++i) {
                    const touch = e.changedTouches[i];
                    const id = touch.identifier + 1;
                    const x = touch.clientX - rect.left;
                    const y = touch.clientY - rect.top;
                    cancelDefault = cancelDefault || _ek_app_js__on_touch(type, id, x, y);
                }
                if (cancelDefault) {
                    e.preventDefault();
                }
            }
        };
        const onMouse = (e: MouseEvent) => {
            const type = TYPES[e.type];
            // https://developer.mozilla.org/en-US/docs/Web/API/MouseEvent/button
            const rect = (e.target as HTMLElement).getBoundingClientRect();
            const x = e.clientX - rect.left;
            const y = e.clientY - rect.top;
            if (type && _ek_app_js__on_mouse(type, BUTTONS[e.button], x, y)) {
                e.preventDefault();
            }
        };

        const onWheel = (e: WheelEvent) => {
            if (_ek_app_js__on_wheel(e.deltaX, e.deltaY)) {
                e.preventDefault();
            }
        };

        let nonPassiveOpt: false | { passive: false } = false;
        try {
            const def = Object.defineProperty({}, 'passive', {
                get: () => {
                    nonPassiveOpt = { passive: false };
                },
            }) as AddEventListenerOptions;
            window.addEventListener("test", () => { }, def);
        } catch (e) {
        }
        /** {CanvasElement} */
        const canvas = document.getElementById("gameview") as HTMLCanvasElement;
        canvas.addEventListener("mousedown", onMouse, nonPassiveOpt);
        canvas.addEventListener("mouseup", onMouse, nonPassiveOpt);
        canvas.addEventListener("mousemove", onMouse, nonPassiveOpt);
        canvas.addEventListener("wheel", onWheel, nonPassiveOpt);
        canvas.addEventListener("touchstart", onTouch, nonPassiveOpt);
        canvas.addEventListener("touchend", onTouch, nonPassiveOpt);
        canvas.addEventListener("touchmove", onTouch, nonPassiveOpt);
        canvas.addEventListener("touchcancel", onTouch, nonPassiveOpt);

        const webgl_attributes: EmscriptenGLAttributes = {
            alpha: false,
            depth: !!(flags & 1),
            stencil: !!(flags & 2),
            antialias: !!(flags & 4),
        };
        const gl = canvas.getContext("webgl2", webgl_attributes);
        if (!gl) {
            console.error("Failed to create WebGL context");
            return false;
        }
        canvas.addEventListener("webglcontextlost", (e) => {
            alert("WebGL context lost. You will need to reload the page.");
            e.preventDefault();
        }, false);

        webgl_attributes.majorVersion = 2;
        webgl_attributes.minorVersion = 0;
        // extensions required for sokol by default
        // webgl_attributes.enableExtensionsByDefault = true;
        const handle = GL.registerContext(gl, webgl_attributes);
        if (!GL.makeContextCurrent(handle)) {
            console.error("Failed to set current WebGL context");
            return false;
        }

        // check visibility,
        // TODO: bind this with running loop and set after ready event
        let hidden: string | undefined;
        let visibilityChange: string | undefined;
        if (typeof document.hidden !== "undefined") {
            // Opera 12.10 and Firefox 18 and later support
            hidden = "hidden";
            visibilityChange = "visibilitychange";
        } else if (typeof (document as any).msHidden !== "undefined") {
            hidden = "msHidden";
            visibilityChange = "msvisibilitychange";
        } else if (typeof (document as any).webkitHidden !== "undefined") {
            hidden = "webkitHidden";
            visibilityChange = "webkitvisibilitychange";
        }

        // Handle page visibility change
        const handleFocus = (_?: Event): void => {
            let flags = 0;
            if (hidden && !(document as any)[hidden]) {
                flags |= 1;
            }
            if (document.hasFocus()) {
                flags |= 2;
            }
            _ek_app_js__on_focus(flags);
        }

        if (typeof document.addEventListener === "undefined" || !visibilityChange) {
            console.warn("No Page Visibility API");
        } else {
            document.addEventListener(visibilityChange, handleFocus, false);
        }
        wnd.addEventListener("blur", handleFocus, false);
        wnd.addEventListener("focus", handleFocus, false);

        handleFocus();

        return true;
    },
    ek_app_js_run: function (): void {
        const loop = (time: number) => {
            requestAnimationFrame(loop);
            _ek_app_js__loop(time / 1000.0);
        };
        loop(-1.0);
    },
    ek_app_js_close: function (): void {
        window.close();
    },
    ek_app_js_lang: function (dest: number, maxLength: number): void {
        const lang = window.navigator.language;
        if (lang) {
            stringToUTF8(lang, dest, maxLength);
        } else {
            HEAPU8[dest] = 0;
        }
    },
    ek_app_js_navigate: function (pURL: number): number {
        try {
            window.open(UTF8ToString(pURL), "_blank");
            return 0;
        } catch {
        }
        return 1;
    },
    ek_app_js_share: function (pContent: number): number {
        if (navigator.share) {
            navigator.share({
                // title: "",
                // url: "",
                text: UTF8ToString(pContent)
            }).then(() => {
                //console.log('Thanks for sharing!');
            }).catch(console.error);
            return 0;
        } else {
            // not supported
            return 1;
        }
    },
};