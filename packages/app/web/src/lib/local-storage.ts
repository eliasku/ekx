export const LocalStorageLib = {
    web_ls_set_f64: (pKey:number, value:number):boolean => {
        const ls = window.localStorage;
        if (ls && pKey) {
            ls.setItem(UTF8ToString(pKey), ""+value);
            return true;
        }
        return false;
    },
    web_ls_get_f64: (pKey:number, pValue:number):boolean => {
        const ls = window.localStorage;
        if (ls && pKey) {
            const item = ls.getItem(UTF8ToString(pKey));
            if (item != null) {
                const value = parseFloat(item);
                if (value != null) {
                    if (pValue) {
                        HEAPF64[pValue >>> 3] = value;
                    }
                    return true;
                }
            }
        }
        return false;
    },
    /**
     *
     * @param pKey - NULL-terminated c-string
     * @param pValue - NULL-terminated c-string, if 0 - delete key,
     * @returns - true if operation is completed, false if not supported or invalid arguments
     */
    web_ls_set: (pKey:number, pValue:number):boolean => {
        const ls = window.localStorage;
        if (ls && pKey) {
            const key = UTF8ToString(pKey);
            if (pValue) {
                ls.setItem(key, UTF8ToString(pValue));
            } else {
                ls.removeItem(key);
            }
            return true;
        }
        return false;
    },
    /**
     * You can read or check key existence in user's LocalStorage
     *
     * @param pKey - CString or 0
     * @param pDest - Buffer to write the value (or 0 if you don't need read the value)
     * @param maxLength - Max buffer length
     * @param pOutStringLen - result string length (bytes written to buffer EXCLUDING null-terminator byte)
     * @returns - true if key is valid and exists in storage, false otherwise
     */
    web_ls_get: (pKey:number, pDest:number, maxLength:number, pOutStringLen:number):boolean => {
        const ls = window.localStorage;
        if (ls && pKey) {
            const value = ls.getItem(UTF8ToString(pKey));
            if (value != null) {
                let written = 0;
                if (pDest && maxLength > 0) {
                    written = stringToUTF8(value, pDest, maxLength);
                }
                if (pOutStringLen) {
                    HEAPU32[pOutStringLen >> 2] = written;
                }
                return true;
            }
        }
        return false;
    },
};