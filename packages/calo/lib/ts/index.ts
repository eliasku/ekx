import {writeFileSync} from "node:fs";
import {bool, char, static_string, Type} from "../common.js";

const codegen_ts = {
    declarations: [] as string[],
};

const isSimpleArray = (type: Type) => {
    return type.options.variableLengthArray && type.options.generics?.length === 1;
};

const getTSTypeName = (type: Type): string => {
    const targetTypeName = type.options.target?.ts?.typeName;
    if (targetTypeName) {
        return targetTypeName;
    }
    if (isSimpleArray(type)) {
        return getTSTypeName(type.options.generics![0]) + "[]";
    }
    return type.typeName;
}

export const generateTypeDeclaration = (type: Type) => {
    if (type.options.extern || isSimpleArray(type)) {
        return;
    }
    if (type.options.optional || type.options.variableLengthArray || type.options.staticLength) {
        return;
    }
    const typeName = getTSTypeName(type);

    if (type.options.enumValues) {
        let code = `export const enum ${typeName} {\n`;
        for (const name of Object.keys(type.options.enumValues)) {
            const val = type.options.enumValues[name];
            code += `   ${name} = ${val},\n`;
        }
        code += `}`;
        codegen_ts.declarations.push(code);
        return;
    }

    let code = `export interface ${typeName} {\n`;
    for (const [field, fieldType] of type.fields) {
        const T = fieldType.options.generics?.[0];
        if (T && fieldType.options.optional) {
            code += `\t${field}?: ${getTSTypeName(T)};\n`;
        } else if (T && fieldType.options.staticLength) {
            if (fieldType.options.generics?.[0] !== char) {
                code += `\t${field}: StaticArray<${getTSTypeName(T)}, ${fieldType.options.staticLength}>;\n`;
            } else {
                code += `\t${field}: StaticString<${fieldType.options.staticLength}>;\n`;
            }
        } else {
            code += `\t${field}: ${getTSTypeName(fieldType)};\n`;
        }
    }
    code += `}`;
    codegen_ts.declarations.push(code);
};

const getReaderName = (type: Type) => {
    let T = type;
    if (type.options.readAsType) {
        if (type === bool) {
            // special case, use TS reader/writer
        } else {
            T = type.options.readAsType;
        }
    }
    return T.options.target?.ts?.reader ?? `read_stream_${getTSTypeName(T)}`;
}

export const generateStreamRead = (type: Type) => {
    if (type.options.apiStreamRead || type.options.readAsType) {
        return;
    }
    if (type.options.optional || type.options.variableLengthArray || type.options.staticLength) {
        return;
    }
    const typeName = getTSTypeName(type);
    const readerName = getReaderName(type);
    let code = `export const ${readerName} = (r: Reader): ${typeName} => {
    const val = {} as ${typeName};
`;
    for (const [field, fieldType] of type.fields) {
        if (type.options.transientFields?.[field]) {
            continue;
        }
        const N = fieldType.options.staticLength;
        const T = fieldType.options.generics?.[0];
        if (N && T === char) {
            code += `\tval.${field} = read_stream_static_string(r, ${N});\n`;
        } else if (N && T) {
            code += `\tfor(let i = 0, sz = ${N}; i < sz; ++i) val.${field}[i] = ${getReaderName(T)}(r);\n`;
        } else if (T && fieldType.options.variableLengthArray) {
            code += `\tval.${field} = [];\n`;
            code += `\tfor(let i = 0, sz = read_stream_u32(r); i < sz; ++i) val.${field}[i] = ${getReaderName(T)}(r);\n`;
        } else if (T && fieldType.options.optional) {
            code += `\t{ const has = read_stream_u8(r); if (has) val.${field} = ${getReaderName(T)}(r); }\n`;
        } else if (fieldType.options.enumBaseType) {
            code += `\tval.${field} = ${getReaderName(fieldType)}(r) as ${getTSTypeName(fieldType)};\n`;
        } else {
            code += `\tval.${field} = ${getReaderName(fieldType)}(r);\n`;
        }
    }
    code += `    return val;
}`;
    codegen_ts.declarations.push(code);
};

const getWriterName = (type: Type) => {
    let T = type;
    if (type.options.readAsType) {
        if (type === bool) {
            // special case, use TS reader/writer
        } else {
            T = type.options.readAsType;
        }
    }
    return T.options.target?.ts?.writer ?? `write_stream_${getTSTypeName(T)}`;
}

export const generateStreamWrite = (type: Type) => {
    if (type.options.apiStreamWrite || type.options.readAsType) {
        return;
    }
    if (type.options.optional || type.options.variableLengthArray || type.options.staticLength) {
        return;
    }
    const typeName = getTSTypeName(type);
    const writerName = getWriterName(type);
    let code = `export const ${writerName} = (w: Writer, v: ${typeName}): void => {\n`;
    for (const [field, fieldType] of type.fields) {
        if (type.options.transientFields?.[field]) {
            continue;
        }
        const N = fieldType.options.staticLength;
        const T = fieldType.options.generics?.[0];
        if (N && T === char) {
            code += `\twrite_stream_static_string(w, v.${field}, ${N});\n`;
        } else if (N && T) {
            code += `\tfor(let i = 0; i < ${N}; ++i) ${getWriterName(T)}(w, v.${field}[i]);\n`;
        } else if (T && fieldType.options.variableLengthArray) {
            code += `\twrite_stream_u32(w, v.${field}.length);\n`;
            code += `\tfor(let i = 0; i < v.${field}.length; ++i) ${getWriterName(T)}(w, v.${field}[i]);\n`;
        } else if (T && fieldType.options.optional) {
            code += `\twrite_stream_u8(w, v.${field} != null ? 1 : 0);\n`;
            code += `\tif(v.${field} != null) ${getWriterName(T)}(w, v.${field});\n`;
        } else {
            code += `\t${getWriterName(fieldType)}(w, v.${field});\n`;
        }
    }
    code += `}`;
    codegen_ts.declarations.push(code);
};

export const generateType = (type: Type) => {
    generateTypeDeclaration(type);
    generateStreamRead(type);
    generateStreamWrite(type);
}

export const save_ts = (name: string, filepath: string) => {
    let code = codegen_ts.declarations.join("\n\n");
    code = `import {
\tread_stream_u8,
\tread_stream_u16,
\tread_stream_u32,
\tread_stream_i8,
\tread_stream_i16,
\tread_stream_i32,
\tread_stream_f32,
\tread_stream_f64,
\tread_stream_string,
\tread_stream_static_string,
\tread_stream_boolean,
\twrite_stream_u8,
\twrite_stream_u16,
\twrite_stream_u32,
\twrite_stream_i8,
\twrite_stream_i16,
\twrite_stream_i32,
\twrite_stream_f32,
\twrite_stream_f64,
\twrite_stream_string,
\twrite_stream_static_string,
\twrite_stream_boolean,
} from "./calo.js";

import type {
\tchar,
\tu8,
\tu16,
\tu32,
\ti8,
\ti16,
\ti32,
\tf32,
\tf64,
\tStringHash32,
\tVec2,
\tVec3,
\tVec4,
\tRect,
\tColor,
\tColor2F,
\tMat2,
\tMat3x2,
\tStaticString,
\tStaticArray,
\tWriter,
\tReader,
} from "./calo.js";

${code}`;


    writeFileSync(filepath, code, "utf-8");
};

