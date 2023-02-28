//
// Created by pannous on 13.10.20.
//

#ifndef WASP_MAP_H
#define WASP_MAP_H

#include <cstdlib> // OK in WASM!

#include "String.h"
//#include "Node.h"
#include "wasm_helpers.h"
#include <cstdarg> // va_list OK IN WASM???
//#include <cstdarg> // va_list ok in wasm even without wasi!

#ifndef WASM

#include <initializer_list> // allow List x={1,2,3};

#endif

#define MAP_MAX_CAPACITY 100000
#define MAP_INITIAL_CAPACITY 8

template<class S, class T>
class Map {
public: // todo careful Map<char*,…> eq
    int capacity = MAP_INITIAL_CAPACITY;// initial
    S *keys = (S *) calloc(sizeof(S), capacity);
    T *values = (T *) calloc(sizeof(T), capacity);
    int _size = 0;

    int map_header = map_header_32;

    [[maybe_unused]] T defaulty;
    bool dont_use_constructor = false;// blank/copy makes sense for List of references but NOT for list of Objects
    bool use_default = false;// no, this would copy fields (e.g. pointers to same list)  todo what is the point? remove
    bool leave_blank = false;// true generally VERY BAD IDEA! map["x"] would return uninitialized &T, e.g. deep fields 0
    //	bool leave_blank == use_malloc_constructor = true;// return reference to freshly nulled malloc data, same ^^


    // unnecessary :
//    Map() {
//        keys = (S *) calloc(sizeof(S), capacity);
//        values = (T *) calloc(sizeof(T), capacity);
//    }

//    Map(T default0) : defaulty(default0) {}

//
//    Map(const Map &old) : keys(old.keys), values(old.values) {
//        _size = old._size;
//    }
//
//	Map &operator=(const Map &old){
//		_size = old._size;
//		keys = old.keys;
//		values = old.values;
//		return *this;
//	}

    S *lookup(T &t) {
        for (int i = 0; i < _size; i++)
            if (values[i] == t)
                return &keys[i];
        return 0;
    }

    T *lookup(S &t) const {
        for (int i = 0; i < _size; i++)
            if (keys[i] == t)
                return &values[i];
        return 0;
    }

    S *has(T t) const { return lookup(t); }

    T *has(S s) const {
        return lookup(s);
    }

    // returns -1 if not found
    int position(S s) {
        for (int i = 0; i < _size; i++) //  or keys[i]!=0
            if (s == keys[i])
                return i;
        return -1;
    }

    int position(S *s) {
        if (s >= begin() and s <= end())
            return s - begin();// pointer matches directly
        for (int i = 0; i < _size; i++)
            if (*s == keys[i])// compare VALUES!
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

    int size() const {
        return _size;
    }

    int add(S *key, T *value) {
        int found = position(key);
        if (found >= 0) error("DUPLICATE KEY: "s + key); // or use insert_or_assign
        if (keys == 0)
            error("how?");
        keys[_size] = key;
        values[_size] = value;
        _size++;
        if (_size >= capacity)grow();
        return _size;
    }

//
// hopefully c++ is smart enough to not copy S / T twice
    int add(S key, T value) {
		int found = position(key);
		if (found >= 0)
			error("DUPLICATE KEY: "s + key); // or use insert_or_assign
		if (keys == 0 or _size >= capacity) grow();
		keys[_size] = key;
		values[_size] = value;
		_size++;
		return _size;
	}

    // similar to map[key]=value
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

    bool remove(S &key) {
        int found = position(key);
        if (found >= 0) {
            keys[found] = keys[_size - 1];
            values[found] = values[_size - 1];
            _size--;
            return true;
        } else warn("can't remove non-existing key");
        return false;
    }

    bool remove(T &val) {
        int found = position(val);
        if (found >= 0) {
            values[found] = values[_size - 1];
            keys[found] = keys[_size - 1];
            _size--;
            return true;
        } else warn("can't remove non-existing value");
        return false;
    }


    // MUST USE map.has(x) instead of map[x] otherwise it is created!!
    // prepare assignment a[b]=c  BAD because unknown symbols will be added!!
    T &operator[](S key) {// CREATING on access! use map.has(x) if not desired
//        trace("map[key]");
        if (_size >= capacity)grow();
        int position1 = position(key);
        if (position1 >= 0) {
//            trace("Key known");
            return values[position1];
        }
        if (leave_blank) { // leaves deep fields uninitialized.
            // Problematic for functions["exit"].signature.return_types …
            trace("leave_blank");
            keys[_size] = key;
            return values[_size++];// values already contain blank T's so ok? no can FAIL with "T not initialized"
        } else if (use_default and false) {
            // todo remove after you understand that this is a bad idea … and don't come up with that idea again
//				memcpy(t, defaulty, sizeof(T));// BAD because this would copy fields (e.g. pointers to same list)
//				return defaulty;// BAD because symbols["missing"]=9 => defaulty=9 wtf
        } else if (dont_use_constructor) {
            error("MISSING KEY: "s + key);
        } else { // use default constructor
            keys[_size] = key;
            values[_size] = T();
            _size++;
//            insert_or_assign(key, T()); // creates intermediate stack value, or is c++ smart?
            return last();
        }
        return last();
    }

    T &operator[](S *key) {// CREATING on access! use map.has(x) if not desired
        if (_size >= capacity)grow();
        int position1 = position(key);
        if (position1 >= 0)
            return values[position1];
        if (dont_use_constructor)
            error("MISSING KEY: "s + key);
        // use default constructor
        keys[_size] = key;
        values[_size] = T();
        _size++;
        return last();
    }
    // todo dangling intermediate which adds element only on assignment
//    T& operator[]=(S s){}
//   Return a proxy object which will have:
//    operator=(T const &) overloaded for writes
//    operator T() for reads


    S &operator[](T &value) {// inverse lookup
        return keys[position(value)];
    }

    /*T*/ S &operator[](size_t position) {
        return keys[position];
//        return values[position];
    }

//    T &operator[](int position) {
//        return values[position];
//    }

//    T &operator[](size_t position) {
//        return values[position]; // functions that differ only in their return type cannot be overloaded
//    }

    T &value(int position) {
        check_silent(position >= 0);
        check_silent(position < size());
        return values[position];
    }

    T &at(int position) {
        check_silent(position >= 0);
        check_silent(position < size());
        return values[position];
    }

    void grow() { // todo don't grow when holding references!
        capacity = capacity * 2;
        check_silent(capacity < MAP_MAX_CAPACITY);
//        warn("GROWING");
        S *new_keys = (S *) alloc(sizeof(S), capacity);
        T *new_values = (T *) alloc(sizeof(T), capacity);
        if (keys and values) {
            memcpy((void *) new_keys, (void *) keys, sizeof(S) * capacity / 2);
            memcpy((void *) new_values, (void *) values, sizeof(T) * capacity / 2);
// currently we can't guarantee that external references exist, e.g. fun.signature consumeExportSection() wasm_reader.cpp:433
// may result in AddressSanitizer: heap-use-after-free if references are held during grow/construction
// todo use free after … runtime is debugged or save Map invented. currently ok though
            free(keys);
            free(values);
        }
        keys = new_keys;
        values = new_values;
    }

    void clear() {
        if (size() == 0)return;
        free(keys);// todo  (interrupted by signal 6: SIGABRT) in WebApp why?
        free(values);
        keys = (S *) calloc(sizeof(S), capacity);
        values = (T *) calloc(sizeof(T), capacity);
        _size = 0;
    }

    void setDefault(T d) {
        if (sizeof(T) > 8) todo("careful! only use setDefault for value types without nested data!");
        defaulty = d;
        use_default = true;// we can't tell if defaulty is 'good' otherwise
    }


//	void put() {
//		error("use log(map) instead of map(put");
//	}
    bool empty() {
        return _size <= 0;
    }


    S *begin() const {
        return &keys[0];
    }

    S *end() const {
        return &keys[_size];
    }// one BEHIND last!

    T &last() {
        if (_size <= 0) error("no last item in empty map");
        return values[_size - 1];
    }

    List<T> valueList() {
        return List<T>(values, size());
    }

//    S* find(bool (lambda)(S&)) {
//        for (S *s: *this)
//            if (lambda(s))return s;
//        return 0;
//    }


    T *find(bool (*lambda)(T &)) {
        for (int i = 0; i < _size; ++i)
            if (lambda(values[i]))return &values[i];
        return 0;
    }

    bool contains(S key) {
        return position(key) >= 0;
    }

    uint get(S key) {
        return this[key];
    }
};


template<class S, class T>
void print(Map<S, T> map) {
    printf("Map (size: %d)\n", map.size());
    for (int i = 0; i < map.size(); ++i)
        printf("%s: %s\n", String(map.keys[i]).data, String(map.values[i]).data);
    // todo external String(o) ≈ o.toString()
}


template<class T>
void print(Map<String, T> map) {
    printf("Map (size: %d)\n", map.size());
    for (int i = 0; i < map.size(); ++i)
        printf("%s: %s\n", map.keys[i].data, String(map.values[i]).data);
}

//void put(Map<S, T> map);


#endif //WASP_MAP_H

