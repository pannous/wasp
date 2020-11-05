//
// Created by pannous on 13.10.20.
//

#ifndef WASP_MAP_H
#define WASP_MAP_H


#include "String.h"
#include "Node.h"

int _size = 1000;

// don't use template! just use int-map
template<class S, class T>
class Map {
	S *keys = (S *) alloc(sizeof(T), _size);
	T *values = (T *) alloc(sizeof(T), _size);
	int _size;

public:

	S *contains(T t) {
		for(int i = 0; (values[i] or keys[i]) and i<_size; i++)
			if (values[i] == t)
				return &keys[i];
		return 0;
	}

	T *contains(S s) {
		for(int i = 0; (values[i] or keys[i]) and i<_size; i++)
			if (keys[i] == s)
				return &values[i];
		return 0;
	}

	int position(S s) {
		for(int i = 0; (values[i] or keys[i]) and i<_size; i++)
			if (keys[i] == s)
				return i;
		return -1;
	}

	int position(T t) {
		for(int i = 0; (values[i] or keys[i]) and i<_size; i++)
			if (values[i] == t)
				return i;
		return -1;
	}

	int size() {
		return _size;
	}

	int insert_or_assign(S key, T value) {
		int found = position(key);
		if (found>=0) {
			values[found] = value;
			return found;
		} else {
			keys[_size] = key;
			values[_size] = value;
			_size++;
			return _size;
		}
	}

	T &operator[](S key) {
		int position1 = position(key);
		T &t = values[position1];
		return t;
	}
//
//	int operator[](S key) {
//		return position(key);
//	}

	S &operator[](T value) {// inverse lookup (!?)
		return keys[position(value)];
	}

};


#endif //WASP_MAP_H
