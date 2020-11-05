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
	S *keys = (S *) calloc(sizeof(T), MAX);
	T *values = (T *) calloc(sizeof(T), MAX);
	int _size = 0;

public:

	S *contains(T t) {
		for (int i = 0; (values[i] or keys[i]) and i < _size; i++)
			if (values[i] == t)
				return &keys[i];
		return 0;
	}

	T *contains(S s) {
		for (int i = 0; (values[i] or keys[i]) and i < _size; i++)
			if (keys[i] == s)
				return &values[i];
		return 0;
	}

	int position(S s) {
		for (int i = 0; i < _size and values[i] or keys[i]; i++)
			if (s == keys[i])
				return i;
		return -1;
	}

	int position(T t) {
		for (int i = 0; (values[i] or keys[i]) and i < _size; i++)
			if (values[i] == t)
				return i;
		return -1;
	}

	int size() {
		return _size;
	}

	int insert_or_assign(S key, T value) {
		int found = position(key);
		if (found >= 0) {
			values[found] = value;
			return found;
		} else {
			keys[_size] = key;
			values[_size] = value;
			_size++;
			return _size;
		}
	}

//	T *operator[](S key) {
	T &operator[](S key) {
		int position1 = position(key);
		if (position1 < 0) {
			if(use_default)return defaulty;
			else error("MISSING KEY: %s\n"s % key);
		}
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

	void clear() {
		free(keys);
		free(values);
		keys = (S *) calloc(sizeof(T), MAX);
		values = (T *) calloc(sizeof(T), MAX);
		_size = 0;
	}

	bool use_default=false;
	T defaulty=0;
	void setDefault(T d){
		defaulty = d;
		use_default = true;
	}
};


#endif //WASP_MAP_H
