//
// Created by me on 19.11.22.
//

#ifndef WASP_WITREADER_H
#define WASP_WITREADER_H


#include "Node.h"
#include "Wasp.h"

class WitReader {
    Node &readType(Node node) {
        Node &alias = node[1];
        Node &type = *node.value.node;
        if (types.has(type.name))
            type = types[type.name];
        else {
            type.kind = clazz;
            types.add(type.name,
                      type);
        }
//        Node &alias = node.from(3); // dataMode=false
        alias.kind = clazz;
//        if(!type["aliases"].has(alias.name))
        type["aliases"].add(alias);
        if (!types.has(alias.name))
            types.add(alias.name,
                      type);// direct mapping!
        else {
            if (types[alias.name] != type)
                error(""s + alias.name + " already aliased to " + types[alias.name].name + "! Can't remap to " +
                      type.name);
        }

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

    void readEnum(Node node) {
        if (node.length == 0)return;/* enum %bool { %false, %true, } nonsense */
        Node enuma = node[1];
        enuma.kind = enums;
        int value = 0;
        for (auto field: enuma) {
            field.value = value++;
            field.kind = longs;
            // currently enum fields are just named numbers
        }
        if (!types.has(enuma.name))
            types.add(enuma.name,
                      enuma);
        if (types[enuma.name] != enuma)
            error("enum already known:\n"s + enuma.serialize());
        else warn("enum already known: "s + enuma.name);

        trace("\nwit enum:");
        trace(enuma);
    }

    void readVariant(Node node) {
        Node variant = node[1];
        trace("\nwit variant:");
        trace(variant);
    }


    void readRecord(Node node) {
        Node record = node[1];
        trace("\nwit record:");
        trace(record);
    }

    void readFlags(Node node) {
        Node flags = node[1];
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

    Node &analyzeWit(Node &node) {

        if (node.first().name == "func")
            return readFunc(node);

        bool keys = node.kind == key or (node.kind == groups and node.length == 2);
        keys = keys or (node.length == 3 and node[2].name == "interface");
        for (auto n: node) {
            if (keys)n = node;
            String &entry = n.first().name;
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
                readEnum(n);
            } else if (entry == "interface") {
                readInterface(n);
            } else if (entry == "record") {
                readRecord(n);
            } else if (entry == "flags") {
                readFlags(n);
            } else if (entry == "func") {
                readFunc(n);
            } else if (n.kind == key and n.value.node->name == "func") {
                readFunc(n);
            } else if (n.name == "module") {
                readModule(node);
                break;
            } else if (entry == "module" or entry == "world") {// we don't distinguish yet!
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
                error("UNKNOWN wit entry: "s + n.serialize());
            if (keys)break;
        }
        return node;
    }

public:

    Node &read(String file) {
        ParserOptions *options = new ParserOptions{
                .data_mode = false,
                .arrow = true,
                .dollar_names=true,
                .use_tags = true,
                .kebab_case = true,
                .space_brace = true,
        };
        Node &node = parse(file.data, "", options);
        return analyzeWit(node);
    }
};


#endif //WASP_WITREADER_H
