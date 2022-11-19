//
// Created by me on 19.11.22.
//

#ifndef WASP_WITREADER_H
#define WASP_WITREADER_H


#include "Node.h"
#include "Wasp.h"

class WitReader {
    Node &analyzeWit(Node &node) {
        return node;
    }

public:
    Node &read(String file) {
        char *string = readFile(file.data);
        Node &node = parse(string, new ParserOptions{.use_tags = true, .kebab_case = true});
        return analyzeWit(node);
    }
};


#endif //WASP_WITREADER_H
