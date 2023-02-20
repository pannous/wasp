//
// Created by me on 19.11.22.
//

#ifndef WASP_WITREADER_H
#define WASP_WITREADER_H


#include "Node.h"
#include "Wasp.h"

Node &funcDeclaration(String name, Node &node, Node &body, Node *returns, Module *mod);

// https://github.com/WebAssembly/component-model/blob/main/design/mvp/WIT.md
static List<String> wit_keywords = {
        "world",
        "module",
        "static",
        "interface",
        "type",
        "resource",
        "record",
        "func",
        "variant",
        "flags",
        "enum",
        "tuple",
        "union",
        "future",
        "stream",
        "option",
        "char",
        "u8", "u16", "u32", "u64",
        "s8", "s16", "s32", "s64",
        "float32", "float64",
};

/*
 * THIS prefix format is OUTDATED:
 (@interface func (export "args_get")
    (param $argv (@witx pointer (@witx pointer u8)))
    (param $argv_buf (@witx pointer u8))
    (result $error (expected (error $errno)))
)
 the current format looks like:

resource request {
    body: func() -> future<list<u8>>
    headers: func() -> list<string>
}

// todo: map
// option  todo: add nullable
// expected
// union todo: how?
// component 	all imports in the subtype must be present in the supertype with matching types;

keyword ::= 'use'
          | 'type'
          | 'resource'
          | 'func'
          | 'u8' | 'u16' | 'u32' | 'u64'
          | 's8' | 's16' | 's32' | 's64'
          | 'float32' | 'float64'
          | 'char'
          | 'variant'
          | 'union'
          | 'bool'
          | 'string'
          | 'option'
          | 'list'
          | 'result'
          | 'as'
          | 'from'
          | 'static'
          | 'interface'
          | 'tuple'
          | 'future'
          | 'stream'

 */
class WitReader {
    List<Module *> current_module;

    Node &analyzeType(Node *pNode) {
        return *pNode;
    }

    Node &readType(Node &node) {
        Node &alias = node[1];
        if (!node.value.node) {
            if (node.length == 2)
                node.value.node = &node[0];
            else {
                warn("no type");// bug
                return alias;
            }
//                error("no type");
        }
        Node &type = analyzeType(node.value.node);
        if (types.has(type.name))
            type = *types[type.name];
        else {
            type.kind = clazz;// losing generics, only in meta
            types.add(type.name, type.clone());
        }
//        Node &alias = node.from(3); // dataMode=false
        alias.kind = clazz;
//        if(!type["aliases"].has(alias.name))
        type.metas()["aliases"].add(alias);
        String &name = alias.name;
        if (!types.has(name))
            types.add(name, type.clone());// direct mapping!
        else if (types[name] != type)
            error(""s + name + " already aliased to " + types[name]->name + "! Can't remap to " + type.name);

        trace("\nwit type alias:");
        trace(type);
        trace(alias);
        return type;
    }


    // todo resource interface variant are currently handled IDENTICALLY!
    Node &readResource(Node &node) {
        Node &resource = node[1];
        trace("\nwit resource:");
        trace(resource);
        for (auto field: resource) {
            Node &member = analyzeWit(field);
            member.kind = fields;
        }
        if (!types.has(resource.name))
            types.add(resource.name, resource.clone());// direct mapping!
        else if (types[resource.name] != resource)
            error("resource already known:\n"s + resource.serialize());
        else warn(("resource already known: "s + resource.name));
        return resource;
        // todo: module namespacing ,
        //  fuzzing?
    }

    Module &findOrCreateModule(String name) {
        for (Module *mod: libraries) {
            if (mod->name == name)return *mod;
        }
        Module *mod = new Module();
        mod->name = name;
        libraries.add(mod);
        return *mod;
    }

    Node &readInterface(Node &node) {
        Node &interface = node[1];
        if (interface.name == "func") {
            return readFunc(interface);
        }
        Module &mod = findOrCreateModule(interface.name);
        Function modulHolder;
        modulHolder.module = &mod;
        for (auto field: interface) {
//            Node &member =
            analyze(field, modulHolder);

//            Node &member = analyzeWit(field);
//            member.kind = fields;
        }
//        interface.kind = clazz;// we don't distinguish between prototype classes with/without fields
//        if (!types.has(interface.name))
//            types.add(interface.name, interface.clone());// direct mapping!
//        else if (types[interface.name] != interface)
//            error("interface already known:\n"s + interface.serialize());
//        else
//            warn("interface already known: "s + interface.name);
        // todo: module namespacing ,
        //  fuzzing?

        trace("\nwit interface:");
        trace(interface);
        return interface;
    }

    Node &readEnum(Node &node, Kind kind) {
        if (node.length == 0)return node;/* enum %bool { %false, %true, } nonsense */
        Node &enuma = node[1];
        String &name = enuma.name;
        enuma.kind = kind;
        if (not types.has(name))
            types.add(name, enuma.clone(true));
        else if (types[name] !=
                 enuma) { // todo other-colors(orange purple black {}) ≠ other-colors(orange:0 purple:1 black:2 0)
            warn("different enum already known:\n"s + enuma.serialize() + "\n≠\n" + types[name]->serialize());
            return *types[name];
        } else {
            warn("enum already known: "s + name);// ok, same layout
            return *types[name];
        }
        enuma = *types[name]; // switch to clone (be sure to be long living)
        int value = 1; // 0;
        if (kind == flags)value = 1;
        for (Node &field: enuma) {
            field.type = types[name];
            field.kind = kind == flags ? flag_entry : kind == enums ? enum_entry : longs;
            String &name = field.name;
            if (name.empty())
                continue; // todo fix in valueNode
            if (kind == flags) {
                field.value.longy = value;// for boolean option1 and option2 …
                value *= 2;
            } else
                field.value.longy = value++;
            // currently enum fields are just named numbers
#ifndef RUNTIME_ONLY
            addGlobal(field);
            String &namespaced = enuma.name + "." + field.name;
            globals.add(namespaced, Global{globals.count(), field.name, mapType(field),
                                           &field});// options.ok vs result.ok … to avoid name clashing
#endif
        }
        trace("\nwit enum/flags/option/variant:");
        trace(enuma);
        return enuma;
    }

    Node &readVariant(Node &node) {
        Node &variant = readEnum(node, variants);
        trace("\nwit variant:");
        trace(variant);
        return variant;
    }


    Node &readRecord(Node &node) {
        Node &record = node[1];
        trace("\nwit record:");
        trace(record);
        return record;
    }

    Node &readFlags(Node &node) {
        return readEnum(node, flags);
    }

    Node &readFunc(Node &node) {
        String &name = node.name;
        if (name == "static")
            name = node.name = node[1].name;
        Node *func = node.value.node;
        if (not functions.has(name))
            functions.add(name, {.name=name});// todo …
        if (!func) {
            node.setType(functor).setValue({.data=&functions[name]});
            return node;// with empty body
//            error("missing func");
        }
        Node returns = func->values();
        Module *mod = current_module.size() > 0 ? current_module.last() : 0;
//        body.children = func->children;
        trace("\nwit func signature:");
        trace(name);
        trace(returns);
#if RUNTIME_ONLY
        todo("funcDeclaration in runtime? how did we get here?")
        return node;
#else
        return funcDeclaration(name, node, NUL, &returns, mod);
#endif
//        return *func;
    }

    Node &readModule(Node &node) {
//        if (node.length < 2) todow("empty world");
        Node &world = node.last();
        Module &mod = findOrCreateModule(world.name);
        current_module.add(&mod);
        Function modulHolder;
        modulHolder.module = &mod;
        world.value.module = &mod;
        world.kind = modul;
        for (auto field: world) {
//            Node &member = analyze(field, modulHolder);
//            Node &member =
            analyzeWit(field);
        }
        trace("\nwit module:");
        trace(world);
        return world;
    }

    // outdated (@interface … ) format
    Node &readInterfaceFunc(Node &node) {
        Node &func = node[0][2];
        Function &fun = *new Function();
        if (func[0].name == "export")
            fun.name = func[1].name;
        Signature signature;
        signature.functions.add(&fun);
        for (int i = 1; i < node.length; ++i) {
//            Node &arg = node[i];
//            if(arg.name)
        }
        return func;
    }

public:

    Node &analyzeWit(Node &node) {
        if (node.kind == enums)
            return node;// how did we get here?
        Node &first = node.first();
        bool keys = node.kind == key or (node.kind == groups and node.length == 2);
        keys = keys or (node.length == 3 and node[2].name == "interface");
        String &entry = first.name;
        if (wit_keywords.contains(entry)) {
            if (node.name.empty())node.name = entry;
            return analyzeWitEntry(node);
        }
        if (entry == "type")
            return readType(node);
        if (entry == "@witx") {
//            node.remove(0, 0);
        } else if (entry == "@interface") {
            if (node.length > 1 and node[1] == "func")
                return readFunc(node);
            else {
                warn("Old @interface syntax not supported");
                return NUL;
            }
        } else if (entry == "@interface") {
            return readInterfaceFunc(node);
        } else {
            if (keys) return analyzeWitEntry(node);
        }
        for (Node &n: node)
            n = analyzeWitEntry(n);

        return node;
    }

    Node &analyzeWitEntry(Node &n) {
        String &entry = n.name;
        if (entry.empty())
            entry = n.first().name;
        if (entry == "@witx")
            return analyzeWitEntry(n.from(1));//skip
//        if (n.length == 0)n.add(n.next).add(n.next);// todo: oterh ;)
        if (entry == "static")
            if (n.length == 0)return NUL;
            else return readFunc(n);
        else if (entry == "type") {
            return readType(n);
        } else if (entry == "resource") {
            return readResource(n);
        } else if (entry == "variant") {
            return readVariant(n);
        } else if (entry == "flags") {
            return readFlags(n);
        } else if (entry == "enum") {
            return readEnum(n, enums);
        } else if (entry == "option") {
//                todo("ignore, since all node_pointer are optionals by default unless marked as required!")
        } else if (entry == "world") {
            return readModule(n);
        } else if (entry == "module") {
            return readModule(n);
        } else if (entry == "default") {
            return readModule(n);
        } else if (entry == "interface") {
            return readModule(n);
//            return readInterface(n);
        } else if (entry == "record") {
            return readRecord(n);
        } else if (entry == "func") {
            return readFunc(n);
        } else if (n.values().name == "func") { // todo what is this? js style? current-user: func() -> string
            return readFunc(n);
        } else if (n.kind == key and n.value.node->name == "func") {
            return readFunc(n);
        } else if (n.name == "%") {
            warn("wit % nonsense "s + n.serialize());
        } else if (n.kind == groups) {
            if (n.first().name == "default")
                return analyzeWitEntry(n.last());// default export interface …
            for (Node &c: n)
                c = analyzeWit(c);
            return n;
        } else if (n.empty()) {
            return NUL;
        } else
            error("UNKNOWN wit entry: "s + n.serialize());
        return NUL;
    }

    Node &read(String file) {
        ParserOptions options{
                .data_mode = true, // treat = as : (type external-dep)=string
//                .data_mode = false, // keep = as expression
                .arrow = true,
                .dollar_names=true,
                .percent_names=true,
                .at_names = true,
                .use_tags = true,
                .use_generics=true,
                .kebab_case = true,
                .kebab_case_plus = true,
                .space_brace = true,
        };
        Node &node = parse(file.data, options);
        return analyzeWit(node);
    }

};


#endif //WASP_WITREADER_H
