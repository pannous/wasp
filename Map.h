//
// Created by pannous on 13.10.20.
//

#ifndef WASP_MAP_H
#define WASP_MAP_H


#include "String.h"
#include "Node.h"

// don't use template! just use int-map
template<class S,class T>
class Map {
	T *values = (T*) alloc(sizeof(T), 10);
	int _size;

public:
	bool contains(String &string) {
		return false;
	}

	bool contains(char* string) {
		return false;
	}

	int size() {
		return _size;
	}

	bool insert_or_assign(char* string, T i) {
		values[_size++] = i;
		return false;
	}
	T& insert_or_assign(String &string, T i) {
		return i;
	}

	T& operator[](String name){
		return values[0];
	}

};


#endif //WASP_MAP_H
