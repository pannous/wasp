//
// Created by pannous on 13.10.20.
//

#ifndef WASP_MAP_H
#define WASP_MAP_H


#include "String.h"
#include "Node.h"

int MAX = 1000;

// don't use template! just use int-map
template<class S, class T>
class Map {
	S *keys = (S *) alloc(sizeof(T), MAX);
	T *values = (T *) alloc(sizeof(T), MAX);
	int _size;

public:

	S *contains(T t) {
		int i = 0;
		while (values[i] and i++ < MAX)if (values[i] == t)return &keys[i];
		return 0;
	}

	T *contains(S s) {
		int i = 0;
		while (values[i] and i++ < MAX)if (keys[i] == s)return &values[i];
		return 0;
	}

	int position(S s) {
		int i = 0;
		while (values[i] and i++ < _size)if (keys[i] == s)return i;
		return -1;
	}

	int position(T t) {
		int i = 0;
		while (values[i] and i++ < _size)if (values[i] == t)return i;
		return -1;
	}

	int size() {
		return _size;
	}

	int insert_or_assign(S key, T value) {
		int found = position(key);
		if (found) {
			values[found] = value;
			return found;
		} else {
			keys[_size++] = key;
			values[_size] = value;
			return _size - 1;
		}
	}

	T &operator[](S key) {
		return values[position(key)];
	}

	S &operator[](T value) {// inverse lookup (!?)
		return keys[position(value)];
	}

};


#endif //WASP_MAP_H
