export function unlock(unlocked: () => boolean) {
    // "touchstart", "touchend", "mousedown", "pointerdown"
    const events = ["touchstart", "touchend", "mousedown", "click", "keydown"];
    const doc = document;
    const handle = () => {
        if(unlocked()) {
            for (let event of events) {
                doc.removeEventListener(event, handle, true);
            }
        }
    };
    for (let event of events) {
        doc.addEventListener(event, handle, true);
    }
}
