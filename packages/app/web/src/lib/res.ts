declare const __ekfs_onComplete: (id: number, _: number, _len: number) => void;

export const LocalResLib = {
    // LOCAL RESOURCES
    ek_fetch_open: function (pURL: number): number {
        const getNext = () => {
            const next = table.length;
            for (let i = 1; i < next; ++i) {
                if (table[i] == null) {
                    return i;
                }
            }
            return next < 256 ? next : 0;
        };
        let table = (window as any).EK_FETCH_OBJECTS;
        if (!table) {
            (window as any).EK_FETCH_OBJECTS = table = [null];
        }
        const id = getNext();
        if (id) {
            table[id] = {url: UTF8ToString(pURL)};
        }
        return id;
    },
    ek_fetch_load: function (id: number): number {
        const table = (window as any).EK_FETCH_OBJECTS;
        if (!table) {
            return 1;
        }
        const obj = table[id];
        if (!obj || !obj.url) {
            return 2;
        }
        fetch(new Request(obj.url)).then((response) => {
            return response.arrayBuffer();
        }).then((buffer) => {
            const obj = table[id];
            if (obj) {
                obj.buffer = buffer;
                __ekfs_onComplete(id, 0, buffer.byteLength);
            }
        }).catch((reason) => {
            const obj = table[id];
            if (obj) {
                obj.error = reason;
                __ekfs_onComplete(id, 1, 0);
            }
        });
        return 0;
    },
    ek_fetch_close: function (id: number): number {
        const table = (window as any).EK_FETCH_OBJECTS;
        if (table && table[id]) {
            table[id] = null;
            return 0;
        }
        return 1;
    },
    /**
     *
     * @param id
     * @param pBuffer - destination buffer pointer from C++, to set destination offset you just pass (buff + offset) pointer
     * @param toRead - bytes to read from loaded buffer
     * @param offset - source buffer offset from start
     */
    ek_fetch_read: function (id: number, pBuffer: number, toRead: number, offset: number): number {
        const table = (window as any).EK_FETCH_OBJECTS;
        if (!table) {
            return 0;
        }
        const obj = table[id];
        if (!obj) {
            return 0;
        }
        const buf = obj.buffer;
        if (!buf) {
            return 0;
        }
        {
            // DEBUG
            if (offset + toRead > buf.byteLength) {
                toRead = buf.byteLength - offset;
            }
        }
        const bytes = new Uint8Array(buf, offset, toRead);
        HEAPU8.set(bytes, pBuffer);
        return toRead;
    },
};