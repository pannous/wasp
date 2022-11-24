//
// Created by me on 19.11.22.
//

#ifndef WASP_WITREADER_H
#define WASP_WITREADER_H


#include "Node.h"
#include "Wasp.h"

// https://github.com/WebAssembly/component-model/blob/main/design/mvp/WIT.md
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
    Node &analyzeType(Node *pNode) {
        return *pNode;
    }

    Node &readType(Node node) {
        Node &alias = node[1];
        Node &type = analyzeType(node.value.node);
        if (types.has(type.name))
            type = types[type.name];
        else {
            type.kind = clazz;// losing generics, only in meta
            types.add(type.name, type);
        }
//        Node &alias = node.from(3); // dataMode=false
        alias.kind = clazz;
//        if(!type["aliases"].has(alias.name))
        type.metas()["aliases"].add(alias);
        String &name = alias.name;
        if (!types.has(name))
            types.add(name, type);// direct mapping!
        else if (types[name] != type)
            error(""s + name + " already aliased to " + types[name].name + "! Can't remap to " + type.name);

        trace("\nwit type alias:");
        trace(type);
        trace(alias);
        return type;
    }


    // todo resource interface variant are currently handled IDENTICALLY!
    void readResource(Node node) {
        Node &resource = node[1];
        trace("\nwit resource:");
        trace(resource);
        for (auto field: resource) {
            Node &member = analyzeWit(field);
            member.kind = fields;
        }
        if (!types.has(resource.name))
            types.add(resource.name,
                      resource);// direct mapping!
        else if (types[resource.name] != resource)
            error("resource already known:\n"s + resource.serialize());
        else warn(("resource already known: "s + resource.name));
        // todo: module namespacing ,
        //  fuzzing?
    }

    void readInterface(Node node) {
        Node &interface = node[1];
        if (interface.name == "func") {
            readFunc(interface);
            return;
        }
        for (auto field: interface) {
            Node &member = analyzeWit(field);
            member.kind = fields;
        }
        interface.kind = clazz;// we don't distinguish between prototype classes with/without fields
        if (!types.has(interface.name))
            types.add(interface.name,
                      interface);// direct mapping!
        else if (types[interface.name] != interface)
            error("interface already known:\n"s + interface.serialize());
        else
            warn("interface already known: "s + interface.name);
        // todo: module namespacing ,
        //  fuzzing?

        trace("\nwit interface:");
        trace(interface);
    }

    Node &readEnum(Node &node, Kind kind) {
        if (node.length == 0)return node;/* enum %bool { %false, %true, } nonsense */
        Node &enuma = node[1];
        enuma.kind = enums;
        int value = 0;
        if (kind == flags)value = 1;
        for (auto field: enuma) {
            String &name = field.name;
            if (name.empty())
                continue; // todo fix in valueNode
            if (kind == flags)
                field.value = value * 2;// for boolean option1 and option2 …
            else
                field.value = value++;
            field.kind = longs;
            // currently enum fields are just named numbers
            addGlobal(field);
        }
        String &name = enuma.name;
        if (!types.has(name))
            types.add(name, enuma);
        if (types[name] != enuma)
            error("enum already known with different fields :\n"s + enuma.serialize() + "\n≠\n" +
                  types[name].serialize());
        else warn("enum already known: "s + name);// ok, same layout
        trace("\nwit enum/option/variant:");
        trace(enuma);
        return enuma;
    }

    void readVariant(Node node) {
        Node &variant = readEnum(node, variants);
        trace("\nwit variant:");
        trace(variant);
    }


    void readRecord(Node node) {
        Node &record = node[1];
        trace("\nwit record:");
        trace(record);
    }

    void readFlags(Node node) {
        readEnum(node, flags);
        trace("\nwit flags:");
        trace(flags);
    }

    Node &readFunc(Node node) {
        String &name = node.name;
        if (name == "static")
            name = node[1].name;
        Node *func = node.value.node;
        if (!func)return ERROR;
//            error("missing func");
        Node body = func->values();
//        body.children = func->children;
        trace("\nwit func signature:");
        trace(name);
        trace(body);
        return *func;
    }

    void readModule(Node node) {
        Node module = node[2];// node[1] == "$"
        trace("\nwit module:");
        trace(module);
    }

    // outdated (@interface … ) format
    void readInterfaceFunc(Node &node) {
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

    }

    Node &analyzeWit(Node &node) {

        if (node.first().name == "func")
            return readFunc(node);

        bool keys = node.kind == key or (node.kind == groups and node.length == 2);
        keys = keys or (node.length == 3 and node[2].name == "interface");
        for (auto n: node) {
            if (keys)n = node;
            if (n.kind == key)
                n.add(n.value.node);
            String &entry = n.first().name;
            if (entry == "@witx") {
                node.remove(0, 0);
                continue;
            }
            if (entry == "static")
                if (n.length == 0)continue;
                else readFunc(n);
            else if (entry == "type") {
                readType(n);
            } else if (entry == "resource") {
                readResource(n);
            } else if (entry == "variant") {
                readVariant(n);
            } else if (entry == "enum") {
                readEnum(n, enums);
            } else if (entry == "option") {
//                todo("ignore, since all nodes are optionals by default unless marked as required!")
            } else if (entry == "@interface") {
                readInterfaceFunc(node);
            } else if (entry == "interface") {
                readInterface(n);
            } else if (entry == "record") {
                readRecord(n);
            } else if (entry == "flags") {
                readFlags(n);
            } else if (entry == "func") {
                readFunc(n);
            } else if (entry == "@interface") {
                if (node.length > 1 and node[1] == "func")
                    readFunc(node);
                else warn("Old @interface syntax not supported");
            } else if (n.kind == key and n.value.node->name == "func") {
                readFunc(n);
            } else if (n.name == "module") {
                readModule(node);
                break;
            } else if (entry == "module" or entry == "world" or entry == "resource") {// we don't distinguish yet!
                n.add(node[1]);// hack
                readModule(n);
                break;
            } else if (n.name == "%") {
                warn("wit % nonsense "s + node.serialize());
                break;
            } else if (n.kind == groups) {
                for (auto c: node)
                    analyzeWit(c);
            } else
                error("UNKNOWN wit entry: "s + node.serialize());
            if (keys)break;
        }
        return node;
    }

public:

    Node &read(String file) {
        ParserOptions options{
                .data_mode = false,
                .arrow = true,
                .dollar_names=true,
                .at_names = true,
                .use_tags = true,
                .use_generics=true,
                .kebab_case = true,
                .space_brace = true,
        };
        Node &node = parse(file.data, options);
        return analyzeWit(node);
    }
};


#endif //WASP_WITREADER_H
