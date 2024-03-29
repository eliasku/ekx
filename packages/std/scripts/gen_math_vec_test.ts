import {writeFileSync} from "fs";

function f32_trunc(x: number, digits_after_point: number) {
    const mult = Math.pow(10, digits_after_point);
    return Math.round(x * mult) / mult;
}

const generate_vec_test_suite = (name: string, fields: string[], T: string) => {
    const name_t = name + "_t";
    const a_values: number[] = [];
    const b_values: number[] = [];
    const digits_after_point = T === "float" ? 6 : 0;
    for (let i = 0; i < fields.length; ++i) {
        a_values[i] = f32_trunc(100 * Math.random() - 50, digits_after_point);
        b_values[i] = f32_trunc(100 * Math.random() - 50, digits_after_point);
    }

    const number_literal = (x: number) => {
        if (T === "float") {
            return (x | 0) === x ? `${x}.0f` : `${x}f`;
        }
        return `${x | 0}`;
    }

    const list = (pattern: string, del = ", ") => {
        let values: string[] = [];
        for (const field of fields) {
            values.push(pattern.replace(/\$/g, field));
        }
        return values.join(del);
    }

    const a = () => `${name_t} a = ${name}(${a_values.map(number_literal).join(" ,")});`;
    const b = () => `${name_t} b = ${name}(${b_values.map(number_literal).join(" ,")});`;

    const checks = (op: (a: number[], b: number[], i: number) => number) => {
        let values = [];
        for (let i = 0; i < fields.length; ++i) {
            const field = fields[i];
            values.push(`\t\t\tCHECK_NEAR_EQ(r.${field}, ${number_literal(f32_trunc(op(a_values, b_values, i), digits_after_point))});`);
        }
        return values.join("\n");
    }

    const simple_tests: {
        op_type1?: string,
        op_type2?: string,
        op_expr?: string,
        op_inv_expr?: string,
        c_arg_list?: string,
        cop?: string,
        op?: string,
        op_inv?: string,
        _eval: (a: number[], b: number[], i: number) => number,
        _eval_inv?: (a: number[], b: number[], i: number) => number,
        binary: boolean
    }[] = [
        {
            cop: "neg",
            op: "-",
            _eval: (a, b, i) => -a[i],
            binary: false
        },
        {
            cop: "add",
            op: "+",
            _eval: (a, b, i) => a[i] + b[i],
            binary: true
        },
        {
            cop: "sub",
            op: "-",
            _eval: (a, b, i) => a[i] - b[i],
            binary: true
        },
        {
            cop: "mul",
            op: "*",
            _eval: (a, b, i) => a[i] * b[i],
            binary: true
        },
        {
            // TODO:
            //cop: "div",
            op: "/",
            _eval: (a, b, i) => a[i] / b[i],
            binary: true
        },
        {
            cop: "scale",
            c_arg_list: "a, b.x",
            op_type1: name,
            op_type2: "float",
            //op: "*",
            op_expr: "a * b.x",
            // op_inv: "*",
            op_inv_expr: "a.x * b",
            _eval: (a, b, i) => a[i] * b[0],
            _eval_inv: (a, b, i) => a[0] * b[i],
            binary: true
        }
    ];

    const simple_tests_code = [];
    for (const simple_test of simple_tests) {
        let op = undefined;
        let op_inv = simple_test.op_inv;
        let cop = simple_test.cop;
        let arg_list = "a";
        let op_type1 = simple_test.op_type1 ?? name;
        let op_type2 = simple_test.op_type2 ?? name;
        let op_case_name = op ?? (simple_test.op + name);
        if (simple_test.binary) {
            //op = op ?? "a " + simple_test.op + " b";
            op_case_name = op ?? op_type1 + " " + op + " " + op_type2;
            arg_list = simple_test.c_arg_list ?? "a, b";
        }
        if (cop) {
            simple_tests_code.push(
                `\t\tIT("has ${cop}") {
\t\t\t${a()}
\t\t\t${b()}
\t\t\t${name_t} r = ${cop}_${name}(${arg_list});
${checks(simple_test._eval)}
\t\t}`);
        }
        if (op) {
            simple_tests_code.push(`\t\tIT("has ${op_type1} ${op_inv} ${op_type2}") {
\t\t\t${a()}
\t\t\t${b()}
\t\t\t${name_t} r = ${simple_test.op_expr ?? (simple_test.binary ? "a " + op + " b" : op + "a")};
${checks(simple_test._eval)}
\t\t}`);
        }
        if (op_inv) {
            simple_tests_code.push(`\t\tIT("has ${op_type2} ${op_inv} ${op_type1}") {
\t\t\t${a()}
\t\t\t${b()}
\t\t\t${name_t} r = ${simple_test.op_inv_expr ?? "b " + op_inv + " a"};
${checks(simple_test._eval_inv!)}
\t\t}`);
        }
    }

    return `\tDESCRIBE(${name}) {

${simple_tests_code.join("\n\n")}

\t}
`;
}

let code = `
#ifndef MATH_VEC_TEST_H
#define MATH_VEC_TEST_H

#include "math_test_common.h"

SUITE(vec_math) {
`;
code += generate_vec_test_suite("vec2", ["x", "y"], "float");
code += generate_vec_test_suite("vec3", ["x", "y", "z"], "float");
code += generate_vec_test_suite("vec4", ["x", "y", "z", "w"], "float");
// code += generate_vec_test_suite("ivec2", ["x", "y"], "int");
// code += generate_vec_test_suite("ivec3", ["x", "y", "z"], "int");
// code += generate_vec_test_suite("ivec4", ["x", "y", "z", "w"], "int");
code += `}
#endif // MATH_VEC_TEST_H
`;

writeFileSync("test/math/math_vec_test.c", code, "utf8");
