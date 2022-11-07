//
// Created by pannous on 13.10.20.
//

#ifndef WASP_MAP_H
#define WASP_MAP_H

#include <cstdlib> // OK in WASM!

#include "String.h"
//#include "Node.h"
#include "wasm_helpers.h"
#include <stdarg.h> // va_list OK IN WASM???
//#include <cstdarg> // va_list ok in wasm even without wasi!

#ifndef WASM

#include <initializer_list> // allow List x={1,2,3};

#endif

#define MAP_ALLOCATION_RESERVED_COUNT 1024*8*2 // todo! dynamic grow maps (ugly name cuz ugly impl)


// don't use template! just use int-map
template<class S, class T>
class Map {
	int _size = 0;
public:
	S *keys;// = (S *) calloc(sizeof(S), MAP_ALLOCATION_RESERVED_COUNT);
	T *values;// = (T *) calloc(sizeof(T), MAP_ALLOCATION_RESERVED_COUNT);

	// unnecessary :
	Map() {
		keys = (S *) calloc(sizeof(S), MAP_ALLOCATION_RESERVED_COUNT);
		values = (T *) calloc(sizeof(T), MAP_ALLOCATION_RESERVED_COUNT);
	}

//
	Map(const Map &old) : keys(old.keys), values(old.values) {
		_size = old._size;
	}
//
//	Map &operator=(const Map &old){
//		_size = old._size;
//		keys = old.keys;
//		values = old.values;
//		return *this;
//	}

	S *lookup(T t) {
		for (int i = 0; (values[i] or keys[i]) and i < _size; i++)
			if (values[i] == t)
				return &keys[i];
		return 0;
	}

	T *lookup(S t) {
		for (int i = 0; (values[i] or keys[i]) and i < _size; i++)
			if (keys[i] == t)
				return &values[i];
		return 0;
	}


	int position(S s) {
		for (int i = 0; i < _size; i++) //  or keys[i]!=0
			if (keys[i] == s)
				return i;
		return -1;
	}

	int position(T t) {
		for (int i = 0; i < _size; i++) //  (values[i] or keys[i]) and
			if (values[i] == t)
				return i;
		return -1;
	}

	int count() {
		return _size;
	}

	int size() {
		return _size;
	}

	int add(S key, T value) {
		int found = position(key);
		if (found >= 0) error("DUPLICATE KEY: "s + key);
		keys[_size] = key;
		values[_size] = value;
		_size++;
		return _size;
	}

	// currently same as map[key]=value
	int insert_or_assign(S key, T value) {
		// todo:  key==nil / key.empty (String::) should not be allowed!
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

// MUST USE map.has(x) instead of map[x] otherwise it is created!!
	// prepare assignment a[b]=c  BAD because unknown symbols will be added!!
	T &operator[](S key) {// CREATING on access! use map.has(x) if not desired
		int position1 = position(key);
		if (position1 < 0) {
			if (leave_blank) {
				_size++;// values already contain blank T's so ok
				return values[_size - 1];
			} else if (use_default_constructor) {
//				T* new_default=(T*)calloc(sizeof(T));// necessary for types with field, else all items share the same field!
//				memcpy(new_default, defaulty, sizeof(T));// no, this would copy fields (e.g. pointers to same list)
				insert_or_assign(key, *new T());
				return values[_size - 1];
//				return defaulty;// BAD because symbols["missing"]=9 => defaulty=9 wtf
			} else {
				error("MISSING KEY: "s + key);
				printf("MISSING KEY: ");
//				put(key);
//				printf("%s",key);// todo unsafe!! can be int etc!
				printf("\n");
				return values[_size++];
//				error("MISSING KEY");
			}

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
//		free(keys);// todo  (interrupted by signal 6: SIGABRT) in WebApp why?
//		free(values);
		keys = (S *) calloc(sizeof(T), MAP_ALLOCATION_RESERVED_COUNT);
		values = (T *) calloc(sizeof(T), MAP_ALLOCATION_RESERVED_COUNT);
		_size = 0;
	}

	bool leave_blank = false;//true BAD IDEA especially for pointers!
	bool use_default_constructor = false;
	[[maybe_unused]] T defaulty;

	void setDefault(T d) {
		defaulty = d;
		use_default_constructor = true;
	}

	S *begin() {
		return &keys[0];
	}

	S *end() {
		return &keys[_size];
	}

	bool has(S s) {// todo has(nil) / has(String::empty) should be false
		return position(s) >= 0;
	}
//	void put() {
//		error("use log(map) instead of map(put");
//	}
};

void print(Map<String, int> map);

template<class S, class T>
void print(Map<S, T> map) {
	printf("Map (size: %d)\n", map.size());
	for (int i = 0; i < map.size(); ++i)
		printf("%s: %ld\n", map.keys[i]->data, map.values[i]);
}
//void put(Map<S, T> map);


#endif //WASP_MAP_H

