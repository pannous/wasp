/*
 * Copyright 2017 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

var mode = CodeMirror.defineSimpleMode('wasp', {
    start: [
        {
            token: "number",
            regex: /[+\-]?(?:nan(?::0x[0-9a-fA-F]+)?|infinity|inf|0x[0-9a-fA-F]+\.?[0-9a-fA-F]*p[+\/-]?\d+|\d+(?:\.\d*)?[eE][+\-]?\d*|\d+\.\d*|0x[0-9a-fA-F]+|\d+)/
        },
        {
            token: "keyword",
            regex: /funcref|mut|nop|block|if|then|else|loop|br|br_if|br_table|call|call_indirect|drop|end|return|local\.get|local\.set|local\.tee|global\.get|global\.set|i32\.load|i64\.load|f32\.load|f64\.load|i32\.store|i64\.store|f32\.store|f64\.store|i32\.load8_s|i64\.load8_s|i32\.load8_u|i64\.load8_u|i32\.load16_s|i64\.load16_s|i32\.load16_u|i64\.load16_u|i64\.load32_s|i64\.load32_u|i32\.store8|i64\.store8|i32\.store16|i64\.store16|i32\.const|i64\.const|f32\.const|f64\.const|i32\.eqz|i64\.eqz|i32\.clz|i64\.clz|i32\.ctz|i64\.ctz|i32\.popcnt|i64\.popcnt|f32\.neg|f64\.neg|f32\.abs|f64\.abs|f32\.sqrt|f64\.sqrt|f32\.ceil|f64\.ceil|f32\.floor|f64\.floor|f32\.trunc|f64\.trunc|f32\.nearest|f64\.nearest|i32\.add|i64\.add|i32\.sub|i64\.sub|i32\.mul|i64\.mul|i32\.div_s|i64\.div_s|i32\.div_u|i64\.div_u|i32\.rem_s|i64\.rem_s|i32\.rem_u|i64\.rem_u|i32\.and|i64\.and|i32\.or|i64\.or|i32\.xor|i64\.xor|i32\.shl|i64\.shl|i32\.shr_s|i64\.shr_s|i32\.shr_u|i64\.shr_u|i32\.rotl|i64\.rotl|i32\.rotr|i64\.rotr|f32\.add|f64\.add|f32\.sub|f64\.sub|f32\.mul|f64\.mul|f32\.div|f64\.div|f32\.min|f64\.min|f32\.max|f64\.max|f32\.copysign|f64\.copysign|i32\.eq|i64\.eq|i32\.ne|i64\.ne|i32\.lt_s|i64\.lt_s|i32\.lt_u|i64\.lt_u|i32\.le_s|i64\.le_s|i32\.le_u|i64\.le_u|i32\.gt_s|i64\.gt_s|i32\.gt_u|i64\.gt_u|i32\.ge_s|i64\.ge_s|i32\.ge_u|i64\.ge_u|f32\.eq|f64\.eq|f32\.ne|f64\.ne|f32\.lt|f64\.lt|f32\.le|f64\.le|f32\.gt|f64\.gt|f32\.ge|f64\.ge|i64\.extend_i32_s|i64\.extend_i32_u|i32\.wrap_i64|i32\.trunc_f32_s|i64\.trunc_f32_s|i32\.trunc_f64_s|i64\.trunc_f64_s|i32\.trunc_f32_u|i64\.trunc_f32_u|i32\.trunc_f64_u|i64\.trunc_f64_u|f32\.convert_i32_s|f64\.convert_i32_s|f32\.convert_i64_s|f64\.convert_i64_s|f32\.convert_i32_u|f64\.convert_i32_u|f32\.convert_i64_u|f64\.convert_i64_u|f64\.promote_f32|f32\.demote_f64|f32\.reinterpret_i32|i32\.reinterpret_f32|f64\.reinterpret_i64|i64\.reinterpret_f64|select|unreachable|memory\.size|memory\.grow|type|func|param|result|local|global|module|table|memory|start|elem|data|offset|import|export|i32|i64|f32|f64/
        },
        {
            token: "keyword",
            regex: /def|fun|func|function|nop|block|if|then|else|while|call|data|end|return|get|set|local|global|load|import|export|int|float|number|string/
        },
        {token: "variable", regex: /\$([a-zA-Z0-9_`\+\-\*\/\\\^~=<>!\?@#$%&|:\.]+)/},
        {token: "string", regex: /"(?:[^"\\\x00-\x1f\x7f]|\\[nt\\'"]|\\[0-9a-fA-F][0-9a-fA-F])*"/},
        {token: "comment", regex: /\/\*.*?/, next: "comment"},
        {token: "comment", regex: /\/\/.*$/},
        {token: "comment", regex: /^\s*#.*$/},
        {token: "comment", regex: /\s+#.*$/},
        {regex: /\(/, indent: true},
        {regex: /\)/, dedent: true},
        {regex: /\{/, indent: true},
        {regex: /}/, dedent: true},
        {regex: /\[\n/, indent: true},
        {regex: /]/, dedent: true},

// keep wast syntax for unified style
        {regex: /\(;.*?/, token: "comment", next: "comment"},
        {regex: /;;.*$/, token: "comment"},

    ],

    comment: [
        {regex: /.*?;\)/, token: "comment", next: "start"}, /* wast */
        {regex: /.*?\*\//, token: "comment", next: "start"},
        {regex: /.*/, token: "comment"},
    ],

    meta: {
        dontIndentStates: ['comment'],
    },
});
