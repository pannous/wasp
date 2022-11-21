//
// Created by me on 19.11.22.
//

#ifndef WASP_WITREADER_H
#define WASP_WITREADER_H


#include "Node.h"
#include "Wasp.h"

class WitReader {
    Node &readType(Node node) {

//        node.remove(0, 1);
        Node &type = node[1];
//        Node &alias = node.from(3); // dataMode=false
        Node &alias = *node.value.node;
        trace("\nwit type alias:");
        trace(type);
        trace(alias);
        return alias;
    }

    void readResource(Node node) {
        Node &resource = node[1];
        trace("\nwit resource:");
        trace(resource);
        for (auto field: resource) {
            analyzeWit(field);
        }
    }

    void readEnum(Node node) {
        Node enums = node[1];
        trace("\nwit enum:");
        trace(enums);
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
        for (auto n: node) {
            if (keys)n = node;
//            if(n.has(">"))
//                read


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
            } else if (entry == "record") {
                readRecord(n);
            } else if (entry == "flags") {
                readFlags(n);
            } else if (entry == "func") {
                readFunc(n);
            } else if (n.kind == key and n.value.node->name == "func") {
                readFunc(n);
            } else if (n.kind == groups) {
                for (auto c: node)
                    analyzeWit(c);
            } else if (n.name == "module") {
                readModule(node);
                break;
            } else
                error("UNKNOWN wit entry: "s + n.serialize());
            if (keys)break;
        }
        return node;
    }

public:

    Node &read(String file) {
        ParserOptions *options = new ParserOptions{.use_tags = true, .kebab_case = true, .arrow=true, .space_brace=true, .data_mode = false};
        Node &node = parse(file.data, "", options);
        return analyzeWit(node);
    }
};


#endif //WASP_WITREADER_H
