//
// Created by me on 13.10.20.
//

#ifndef WASP_MAP_H
#define WASP_MAP_H


#include "String.h"
#include "Node.h"

template<class S,class T>
class Map {

public:
	bool contains(String &string) {
		return false;
	}

	bool contains(char* string) {
		return false;
	}

	int size() {
		return 0;
	}

	bool insert_or_assign(char* string, int i) {
		return false;
	}
	bool insert_or_assign(String &string, int i) {
		return false;
	}

	int operator[](String name){
		return 0;
	}

};


#endif //WASP_MAP_H
