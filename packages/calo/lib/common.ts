export interface TypeTargetOptions {
    typeName?: string;
    reader?: string;
    writer?: string;
}

export interface TypeOptions {
    primitiveBitSize?: number;
    primitiveBytes?: number;
    variableLengthArray?: boolean;
    staticLength?: number;
    optional?: boolean;
    generics?: Type[],
    extern?: boolean,
    // api provide stream read function
    apiStreamRead?: boolean,
    apiStreamWrite?: boolean,

    // also write as type
    readAsType?: Type,

    ///
    target?: {
        c?: TypeTargetOptions,
        ts?: TypeTargetOptions,
    },

    enumBaseType?: Type;
    enumValues?: Record<string, any>;

    transientFields?: Record<string, boolean>;
}

export interface TypeInfo {
    isPod?: boolean;
    byteSize?: number;
}

export interface Type {
    typeName: string;
    fields: Map<string, Type>;
    options: TypeOptions;
    info: TypeInfo;
}

const isPod = (type: Type): boolean => {
    if (type.info.isPod !== undefined) {
        return type.info.isPod;
    }
    let pod = true;
    for (const [, fieldType] of type.fields) {
        if (fieldType === type) {
            throw new Error("nested should use references!");
        }
        if (!isPod(fieldType)) {
            pod = false;
            break;
        }
    }
    type.info.isPod = pod;
    return pod;
}

const calcStructSize = (type: Type) => {
    if (type.info.byteSize !== undefined) {
        return type.info.byteSize;
    }
    let byteSize = 0;
    if (!type.fields.size) {
        byteSize = Math.ceil((type.options.primitiveBitSize ?? 8) / 8);
    } else {
        for (const [, fieldType] of type.fields) {
            if (fieldType === type) {
                throw new Error("nested should use references!");
            }
            byteSize += calcStructSize(fieldType);
        }
    }
    type.info.byteSize = byteSize;
    return byteSize;
}

export const type = (typeName: string, fields: Record<string, Type> = {}, options: TypeOptions = {}): Type => ({
    typeName,
    fields: new Map(Object.entries(fields)),
    options,
    info: {},
});

export const _enum = (typeName: string, values: Record<string, number> = {}, baseType: Type = u32, options: TypeOptions = {}): Type => ({
    typeName,
    fields: new Map(),
    options: {
        ...options,
        enumBaseType: baseType,
        enumValues: values,
        readAsType: baseType,
    },
    info: {},
});

export const CData = type("CData", {}, {
    extern: true,
    apiStreamRead: true,
    apiStreamWrite: true,
    target: {
        c: {
            typeName: "const void*",
            reader: "read_stream_data",
            writer: "write_stream_data",
        },
        ts: {
            typeName: "number",
        },
    },
});

export const CString = type("CString", {}, {
    extern: true,
    apiStreamRead: true,
    apiStreamWrite: true,
    target: {
        c: {
            typeName: "const char*",
            reader: "read_stream_string",
            writer: "write_stream_string",
        },
        ts: {
            typeName: "string",
        },
    },
});

export const char = type("char", {}, {
    primitiveBitSize: 8,
    extern: true,
    apiStreamRead: true,
    apiStreamWrite: true,
    target: {
        c: {
            typeName: "char",
            reader: "read_u8",
            writer: "write_u8",
        },
        ts: {
            typeName: "number",
        },
    },
});

export const u8 = type("u8", {}, {
    primitiveBitSize: 8,
    extern: true,
    apiStreamRead: true,
    apiStreamWrite: true,
    target: {
        c: {
            typeName: "uint8_t",
            reader: "read_u8",
            writer: "write_u8",
        }
    },
});

export const u16 = type("u16", {}, {
    primitiveBitSize: 16,
    extern: true,
    apiStreamRead: true,
    apiStreamWrite: true,
    target: {
        c: {
            typeName: "uint16_t",
            reader: "read_u16",
            writer: "write_u16",
        },
    },
});

export const u32 = type("u32", {}, {
    primitiveBitSize: 32,
    extern: true,
    apiStreamRead: true,
    apiStreamWrite: true,
    target: {
        c: {
            typeName: "uint32_t",
            reader: "read_u32",
            writer: "write_u32",
        }
    },
});

export const i32 = type("i32", {}, {
    primitiveBitSize: 32,
    extern: true,
    apiStreamRead: true,
    apiStreamWrite: true,
    target: {
        c: {
            typeName: "int32_t",
            reader: "read_i32",
            writer: "write_i32",
        }
    },
});

export const f32 = type("f32", {}, {
    primitiveBitSize: 32,
    extern: true,
    apiStreamRead: true,
    apiStreamWrite: true,
    target: {
        c: {
            typeName: "float",
            reader: "read_f32",
            writer: "write_f32",
        }
    },
});

export const f64 = type("f64", {}, {
    primitiveBitSize: 64,
    extern: true,
    apiStreamRead: true,
    apiStreamWrite: true,
    target: {
        c: {
            typeName: "double",
            reader: "read_f64",
            writer: "write_f64",
        }
    },
});

export const bool = type("bool", {}, {
    extern: true,
    readAsType: u8,
    target: {
        c: {
            typeName: "bool",
        },
        ts: {
            typeName: "boolean",
            reader: "read_stream_boolean",
            writer: "write_stream_boolean",
        },
    },
});

export const optional = (elementType: Type) => type("Optional", {}, {
    optional: true,
    generics: [elementType],
});

export const array = (elementType: Type) => type("Array", {}, {
    variableLengthArray: true,
    generics: [elementType],
});

export const static_array = (elementType: Type, length: number) => type("StaticArray", {}, {
    extern: true,
    apiStreamWrite: true,
    apiStreamRead: true,
    staticLength: length,
    generics: [elementType],
});

export const static_string = (length: number) => type("StaticString", {}, {
    extern: true,
    apiStreamWrite: true,
    apiStreamRead: true,
    staticLength: length,
    generics: [char],
    target: {
        ts: {
            typeName: "StaticString<" + length + ">",
        }
    }
});

export const collectTypes = (allTypes: Set<Type>, ...mainTypeList: Type[]) => {
    for (const type of mainTypeList) {
        if (allTypes.has(type)) {
            continue;
        } else {
            allTypes.add(type);
        }

        if (type.options.generics) {
            collectTypes(allTypes, ...type.options.generics);
        }
        for (const [field, fieldType] of type.fields) {
            if (type.options.transientFields?.[field]) {
                continue;
            }
            collectTypes(allTypes, fieldType);
        }
    }
}

