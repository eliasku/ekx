import {writeFileSync} from "node:fs";
import {Type} from "../common.js";

const codegen_ts = {
    declarations: [] as string[],
};

const isSimpleArray = (type: Type) => {
    return type.options.variableLengthArray && type.options.generics?.length === 1;
};

const getTSType = (type: Type):string => {
    const targetTypeName = type.options.target?.ts?.typeName;
    if (targetTypeName) {
        return targetTypeName;
    }
    if (isSimpleArray(type)) {
        return getTSType(type.options.generics![0]) + "[]";
    }
    return type.typeName;
}

export const generateType_ts = (type: Type) => {
    if (type.options.extern || isSimpleArray(type)) {
        return;
    }

    let code = `interface ${getTSType(type)} {\n`;

    for (const [field, fieldType] of type.fields) {
        code += `\t${field}: ${getTSType(fieldType)};\n`;
    }
    code += `}\n`;
    codegen_ts.declarations.push(code);
};

export const save_ts = (name: string, filepath: string) => {
    const code = codegen_ts.declarations.join("\n\n");
    writeFileSync(filepath, code, "utf-8");
};

