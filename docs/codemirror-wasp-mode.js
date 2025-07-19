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
       {token: "comment", regex: /\/\*.*?/, next: "comment"},
        {token: "comment", regex: /\/\/.*$/},
        {token: "comment", regex: /^\s*#.*$/},
        {token: "comment", regex: /\s+#.*$/},
     {token: "string", regex: /"(?:[^"\\\x00-\x1f\x7f]|\\[nt\\'"]|\\[0-9a-fA-F][0-9a-fA-F])*"/},
        {token: "string", regex: /“(?:[^”]*)*”/},
        {token: "string", regex: /'(?:[^']*)*'/},
        {token: "string", regex: /‘(?:[^’]*)*’/},
        {token: "string", regex: /«(?:[^»]*)*»/},
        // {
        //     token: ["keyword", null, "variable-2"]
        //     regex: /(function|func|fun|fn|def|defn|defun|to)(\s+)([a-z$][\w$]*)/,
        // },
        {
            token: "number",
            regex: /[+\-]?(?:nan(?::0x[0-9a-fA-F]+)?|infinity|inf|0x[0-9a-fA-F]+\.?[0-9a-fA-F]*p[+\/-]?\d+|\d+(?:\.\d*)?[eE][+\-]?\d*|\d+\.\d*|0x[0-9a-fA-F]+|\d+)/
        },
        {
            token: "atom",
            regex: /it|this|self|none|nil|null|true|false|True|False|None|π|τ/
        },
        {
            token: "keyword",
            regex: /funcref|mut|nop|block|if|then|else|loop|br|br_if|br_table|call|call_indirect|drop|end|return|local|get|set|load|select|unreachable|memory|size|grow|type|func|param|result|local|global|module|table|memory|start|elem|data|offset|import|export|i32|i64|f32|f64/
        },
        {
            token: "type",
            regex: /int|float|real|double|number|string|bool|byte|char|codepoint/
        },

        {
            token: "operator",
            regex: /[-+\/*=<>!∫⌟⌞≠≔:…\.≥≤⁰¹²×⋅⋆÷^∨¬∈∉⊂⊃#$∧⋀⋁∨⊻‖√∑ᵢ²³⁻¹⁰ⁿ]+/
        },
        {
            token: "keyword",
            regex: /operator|and|or|not|xor|return|assert|else|then|pipe|is|equal|equals|not|and|or|to|xor|be|nop|pass|typeof|upto|range|mod|modulo|plus|times|add|minus|less|bigger|in|of|by|iff|on|as|from|ceil|floor|round|abs|norm/
         },
        {
            token: "keyword",
            regex: /def|fun|func|function|nop|block|if|then|else|while|call|data|end|return|get|set|local|global|load|import|export/
        },
        {token: "variable", regex: /\$([a-zA-Z0-9_`\+\-\*\/\\\^~=<>!\?@#$%&|:\.]+)/},
   
        {regex: /\(/, indent: true},
        {regex: /\)/, dedent: true},
        {regex: /\{/, indent: true},
        {regex: /}/, dedent: true},
        {regex: /\[\n/, indent: true},
        {regex: /]/, dedent: true},

// keep wast syntax for unified style
        {regex: /\(;.*?/, token: "comment", next: "comment"},
        {regex: /;;.*$/, token: "comment"},
        {regex: /<</, token: "meta", mode: {spec: "xml", end: />>/}}
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
