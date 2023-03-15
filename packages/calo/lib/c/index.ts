import {writeFileSync} from "node:fs";
import {Type} from "../common.js";
import {PrinterInput, printHeader, printHeaderOnly, printImplementation} from "./file.js";
import {toCamelCase} from "./naming.js";
import * as path from "path";
import {ensureDirSync} from "../../../../lib/utils/utils.js";

export interface CGenOptions {
    name: string;
    includeDir: string;
    // if source-dir is not defined - header-only mode is used
    sourceDir?: string;
}

interface Syy {
    preDeclaration?: string;
    declaration?: string;
    definition?: string;
    isPublic?: boolean;
    isType?: boolean;
    isFunction?: boolean;
}

const codegen_c: Syy[] = [];

const getCName = (type: Type): string => {
    if (type.options.variableLengthArray && type.options.generics?.length === 1) {
        return getCName(type.options.generics[0]) + "_array";
    }
    if (type.options.optional && type.options.generics?.length === 1) {
        return getCName(type.options.generics[0]) + "_opt";
    }
    return toCamelCase(type.typeName);
}

const getCStructName = (type: Type) => {
    const targetName = type.options.target?.c?.typeName;
    if (targetName) {
        return targetName;
    }
    return `${getCName(type)}_t`;
};

const getCTypeName = (type: Type) => {
    return getCStructName(type);
};

export const generateType_c = (type: Type) => {
    if (type.options.extern) {
        return;
    }
    const typeName_t = getCStructName(type);

    if (type.options.variableLengthArray && type.options.generics?.length === 1) {
        const T = type.options.generics[0];
        let code = `typedef ${getCTypeName(T)}* ${getCTypeName(type)};\n`;
        codegen_c.push({
            declaration: code,
            isType: true,
            isPublic: false,
        });
        return;
    }

    if (type.options.optional && type.options.generics?.length === 1) {
        const T = type.options.generics[0];
        let code = `typedef ${getCTypeName(T)}* ${getCTypeName(type)};\n`;
        codegen_c.push({
            declaration: code,
            isType: true,
            isPublic: false,
        });
        return;
    }

    if (type.options.enumValues) {
        let code = `typedef enum {\n`;
        const lines: string[] = [];
        for (const name of Object.keys(type.options.enumValues)) {
            const fld = toCamelCase(name).toUpperCase();
            const val = type.options.enumValues[name];
            lines.push(`   ${fld} = ${val}`);
        }
        code += lines.join(",\n") + "\n";
        code += `} ${typeName_t};\n`;
        codegen_c.push({
            declaration: code,
            isType: true,
            isPublic: true,
        });
        return;
    }

    const structName = `${getCName(type)}_`;
    let preDecl = `struct ${structName};`
    let code = `typedef struct ${structName} {\n`;
    for (const [field, fieldType] of type.fields) {
        if (fieldType.options.generics?.length === 1) {
            const T = fieldType.options.generics[0];
            let arrayTypeString = `struct ${getCName(T)}_`;
            if (T.options.extern) {
                arrayTypeString = getCTypeName(T);
            }
            if (fieldType.options.variableLengthArray || fieldType.options.optional) {
                code += `\t${arrayTypeString}* ${field};\n`;
            }
        } else {
            code += `\t${getCTypeName(fieldType)} ${field};\n`;
        }
    }
    code += `} ${typeName_t};\n`;
    codegen_c.push({
        preDeclaration: preDecl,
        declaration: code,
        isType: true,
        isPublic: true,
    });
};

const getReaderName = (type: Type) => {
    let T = type;
    if (type.options.readAsType) {
        T = type.options.readAsType;
    }
    return T.options.target?.c?.reader ?? `read_stream_${getCName(T)}`;
}

export const generateTypeReadFunctions = (type: Type) => {
    if (type.options.apiStreamRead || type.options.readAsType) {
        return;
    }
    const typeName = getCTypeName(type);
    const readerName = getReaderName(type);
    let isPublic = true;

    let sig = `${typeName} ${readerName}(calo_reader_t* r)`;
    let declaration = sig + ";";
    let code = `${sig} {
    ${typeName} val;
`;
    if (type.options.variableLengthArray && type.options.generics?.length === 1) {
        const arrayType = type.options.generics[0];
        code += `    {
        uint32_t count = read_u32(r);
        val = 0;
        arr_reinit(val, count);
        for(uint32_t i = 0; i < count; ++i) {
            val[i] = ${getReaderName(arrayType)}(r);
        }
    }
`;
        isPublic = false;
    } else if (type.options.optional && type.options.generics?.length === 1) {
        const arrayType = type.options.generics[0];
        code += `    {
        bool has = read_u8(r) != 0;
        val = 0;
        if (has) {
            val = (${typeName})malloc(sizeof *val);
            *val = ${getReaderName(arrayType)}(r);
        }
    }
`;
        isPublic = false;
    } else {
        for (const [field, fieldType] of type.fields) {
            if (type.options.transientFields?.[field]) {
                continue;
            }
            if (fieldType.options.enumBaseType) {
                code += `\tval.${field} = (${getCTypeName(fieldType)})${getReaderName(fieldType)}(r);\n`;
            } else {
                code += `\tval.${field} = ${getReaderName(fieldType)}(r);\n`;
            }
        }
    }
    code += `    return val;
}`;
    codegen_c.push({
        isFunction: true,
        isPublic,
        declaration,
        definition: code,
    });
};

const getWriterName = (type: Type) => {
    let T = type;
    if (type.options.readAsType) {
        T = type.options.readAsType;
    }
    return T.options.target?.c?.writer ?? `write_stream_${getCName(T)}`;
}

export const generateStreamWriteFunctions = (type: Type) => {
    if (type.options.apiStreamWrite || type.options.readAsType) {
        return;
    }
    const typeName = getCTypeName(type);
    const writerName = getWriterName(type);
    let isPublic = true;
    let sig = `void ${writerName}(calo_writer_t* w, ${typeName} v)`;
    let declaration = sig + ";";
    let code = `${sig} {\n`;
    if (type.options.variableLengthArray && type.options.generics?.length === 1) {
        const arrayType = type.options.generics[0];
        code += `    {
        uint32_t count = arr_size(v);
        write_u32(w, count);
        for(uint32_t i = 0; i < count; ++i) {
            ${getWriterName(arrayType)}(w, v[i]);
        }
    }
`;
        isPublic = false;
    } else if (type.options.optional && type.options.generics?.length === 1) {
        const arrayType = type.options.generics[0];
        code += `    {
        write_u8(w, v ? 1 : 0);
        if (v) {
            ${getWriterName(arrayType)}(w, *v);
        }
    }
`;
        isPublic = false;
    } else {
        for (const [field, fieldType] of type.fields) {
            if (type.options.transientFields?.[field]) {
                continue;
            }
            code += `\t${getWriterName(fieldType)}(w, v.${field});\n`;
        }
    }
    code += `}`;
    codegen_c.push({
        isFunction: true,
        isPublic,
        declaration,
        definition: code,
    });
};

let includes: { path: string, api?: boolean, user: boolean }[] = [];

export const c_include = (path: string, api = true, user = true) => {
    includes.push({
        path, api, user,
    });
};

c_include("calo.h", true, false);

export const save_c = (options: CGenOptions) => {
    const name = options.name;
    let publicTypesPreDeclarations = codegen_c.filter(e => e.isPublic && e.isType && e.preDeclaration).map(e => e.preDeclaration);
    let privateTypesPreDeclarations = codegen_c.filter(e => !e.isPublic && e.isType && e.preDeclaration).map(e => e.preDeclaration);
    let types = codegen_c.filter(e => e.isType && e.isPublic && e.declaration).map(e => e.declaration);
    let privateTypes = codegen_c.filter(e => e.isType && !e.isPublic && e.declaration).map(e => e.declaration);

    let publicDeclarations = codegen_c.filter(e => e.isPublic && e.isFunction && e.declaration).map(e => e.declaration).join("\n\n");
    let privateDeclarations = codegen_c.filter(e => !e.isPublic && e.isFunction && e.declaration).map(e => e.declaration).join("\n\n");
    let implementation = codegen_c.filter(e => e.isFunction && e.definition).map(e => e.definition).join("\n\n");
    const input: PrinterInput = {
        name,
        includes,
        types: publicTypesPreDeclarations.concat(types).join("\n"),
        publicDeclarations,
        privateDeclarations: privateTypesPreDeclarations.join("\n") + "\n\n" + privateTypes.join("\n") + "\n\n" + privateDeclarations,
        implementation,
    };

    ensureDirSync(options.includeDir);
    if (options.sourceDir != null) {
        let code = printHeader(input);
        ensureDirSync(options.sourceDir);
        writeFileSync(path.join(options.includeDir, name + ".h"), code, "utf-8");
        code = `#include <${name}.h>
${printImplementation(input)}`;
        writeFileSync(path.join(options.sourceDir, name + ".c.h"), code, "utf-8");
    } else {
        const code = printHeaderOnly(input);
        writeFileSync(path.join(options.includeDir, name + "_header_only.h"), code, "utf-8");
    }
};
