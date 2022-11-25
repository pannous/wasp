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

#define MAP_MAX_CAPACITY 100000
#define MAP_INITIAL_CAPACITY 8

// don't use template! just use int-map
template<class S, class T>
class Map {
public:
    int _size = 0;
    int capacity = MAP_INITIAL_CAPACITY;// initial
    S *keys;// = (S *) calloc(sizeof(S), MAP_ALLOCATION_RESERVED_COUNT);
    T *values;// = (T *) calloc(sizeof(T), MAP_ALLOCATION_RESERVED_COUNT);

    //	bool leave_blank == use_malloc_constructor = true;// return reference to freshly nulled malloc data, same ^^
    bool use_constructor = true;// *new T() makes sense for List of references but NOT for list of Data!!
    bool leave_blank = false;//true would be VERY BAD IDEA especially for pointers! todo what is the point?
    bool use_default = false;// no, this would copy fields (e.g. pointers to same list)  todo what is the point?

    [[maybe_unused]] T defaulty;

	// unnecessary :
	Map() {
        keys = (S *) calloc(sizeof(S), capacity);
        values = (T *) calloc(sizeof(T), capacity);
    }

	Map(T default0) : defaulty(default0) {}

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
            if (s == keys[i])
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
        if (_size >= capacity)grow();
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
            if (_size >= capacity)
                grow();
            return _size;
        }
	}

// MUST USE map.has(x) instead of map[x] otherwise it is created!!
	// prepare assignment a[b]=c  BAD because unknown symbols will be added!!
	T &operator[](S key) {// CREATING on access! use map.has(x) if not desired
        if (_size >= capacity)grow();
		int position1 = position(key);
		if (position1 < 0) {
			if (leave_blank) {
				return values[_size++];// values already contain blank T's so ok
			} else if (use_default and false) {
				// todo remove after you understand that this is a bad idea … and don't come up with that idea again
				T &t = values[_size++];
//				memcpy(t, defaulty, sizeof(T));// BAD because this would copy fields (e.g. pointers to same list)
//				return defaulty;// BAD because symbols["missing"]=9 => defaulty=9 wtf
				return t;
			} else if (use_constructor) {
//				insert_or_assign(key, *new T());
				insert_or_assign(key, T());// BAD because stack value? ok because copy by value? todo
				return values[_size - 1];// increased above!
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

    void grow() { // todo don't grow when holding references!
        capacity = capacity * 2;
        check_silent(capacity < MAP_MAX_CAPACITY);
        S *new_keys = (S *) alloc(sizeof(S), capacity);
        T *new_values = (T *) alloc(sizeof(T), capacity);
        memcpy((void *) new_keys, (void *) keys, sizeof(S) * capacity / 2);
        memcpy((void *) new_values, (void *) values, sizeof(T) * capacity / 2);
        free(keys);
        free(values);
        keys = new_keys;
        values = new_values;
    }

    void clear() {
        free(keys);// todo  (interrupted by signal 6: SIGABRT) in WebApp why?
        free(values);
        keys = (S *) calloc(sizeof(S), capacity);
        values = (T *) calloc(sizeof(T), capacity);
        _size = 0;
    }

    void setDefault(T d) {
		if (sizeof(T) > 8)
			todo("careful! only use setDefault for value types without nested data!");
		defaulty = d;
		use_default = true;// we can't tell if defaulty is 'good' otherwise
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
	bool empty() {
		return _size <= 0;
	}
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

