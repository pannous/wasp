//
// Created by pannous on 13.10.20.
//

#ifndef WASP_MAP_H
#define WASP_MAP_H


//#include "String.h"
#include "Node.h"
#include "wasm_helpers.h"
#include <stdarg.h> // va_list OK IN WASM???
//#include <cstdarg> // va_list ok in wasm even without wasi!

#ifndef WASM

#include <initializer_list> // allow List x={1,2,3};

#endif

#define MAP_ALLOCATION_RESERVED_COUNT 1024*8 // todo! dynamic grow maps (ugly name cuz ugly impl)


template<class S>
inline void swap(S *a, S *b) {
	S c = *a;
	*a = *b;
	*b = c;
}


template<class S>
void heapify(S arr[], int n, int i) {
	int largest = i; // Initialize largest as root
	int l = 2 * i + 1; // left = 2*i + 1
	int r = 2 * i + 2; // right = 2*i + 2
	// If left child is larger than root
	if (l < n && arr[l] > arr[largest])
		largest = l;
	// If right child is larger than largest so far
	if (r < n && arr[r] > arr[largest])
		largest = r;
	// If largest is not root
	if (largest != i) {
		swap(&arr[i], &arr[largest]);
		// Recursively heapify the affected sub-tree
		heapify(arr, n, largest);
	}
}

template<class S>
void heapify(S arr[], int n, int i, float (valuator)(S &)) {
	int largest = i; // Initialize largest as root
	int l = 2 * i + 1; // left = 2*i + 1
	int r = 2 * i + 2; // right = 2*i + 2
	// If left child is larger than root
	if (l < n && valuator(arr[l]) > valuator(arr[largest]))
		largest = l;
	// If right child is larger than largest so far
	if (r < n && valuator(arr[r]) > valuator(arr[largest]))
		largest = r;
	// If largest is not root
	if (largest != i) {
		swap(&arr[i], &arr[largest]);
		// Recursively heapify the affected sub-tree
		heapify(arr, n, largest, valuator);
	}
}


template<class S>
void heapify(S arr[], int n, int i, bool (comparator)(S &, S &)) {
	int largest = i; // Initialize largest as root
	int l = 2 * i + 1; // left = 2*i + 1
	int r = 2 * i + 2; // right = 2*i + 2
	if (l < n && comparator(arr[l], arr[largest]))
		largest = l;
	if (r < n && comparator(arr[r], arr[largest]))
		largest = r;
	if (largest != i) {
		swap(&arr[i], &arr[largest]);
		heapify(arr, n, largest, comparator);
	}
}


template<class S>
void heapSort(S arr[], int n) {
	for (int i = n / 2 - 1; i >= 0; i--)
		heapify(arr, n, i);
	// One by one extract an element from heap
	for (int i = n - 1; i > 0; i--) {
		// Move current root to end
		swap(&arr[0], &arr[i]);
		// call max heapify on the reduced heap
		heapify(arr, i, 0);
	}
}

template<class S>
void heapSort(S arr[], int n, float (valuator)(S &)) {
	for (int i = n / 2 - 1; i >= 0; i--)
		heapify(arr, n, i, valuator);
	for (int i = n - 1; i > 0; i--) {
		swap(&arr[0], &arr[i]);
		heapify(arr, i, 0, valuator);
	}
}

template<class S>
void heapSort(S arr[], int n, bool (comparator)(S &, S &)) {
	for (int i = n / 2 - 1; i >= 0; i--)
		heapify(arr, n, i, comparator);
	for (int i = n - 1; i > 0; i--) {
		swap(&arr[0], &arr[i]);
		heapify(arr, i, 0, comparator);
	}
}


template<class S>
class List {
	int _size = 0;
public:
	S *items = (S *) calloc(sizeof(S), MAP_ALLOCATION_RESERVED_COUNT);

	List() {}

	List(S first, ...) {
		items[0] = first;
		va_list args;// WORKS WITHOUT WASI! with stdargs
		va_start(args, first);
		S *i = &first;
		while (i) {
			i = (S *) va_arg(args, S*);
		}
		va_end(args);
	}

	List(S *args) {// initiator list C style {x,y,z,0} ZERO 0 ø TERMINATED!!
		while (args[_size] and _size < MAP_ALLOCATION_RESERVED_COUNT) {
			items[_size] = args[_size];
			_size++;
		}
	}

#ifndef WASM

	List(const std::initializer_list<S> &_items) {
		for (const S &s : _items) {
			items[_size++] = s;
		}
	}

#endif

	int size() { return _size; };

	S &add(S s) {
		items[_size++] = s;
		return items[_size - 1];
	}

	S &operator[](short index) {
		if (index < 0 or index > _size)error("index out of range : %d > %d"s % index % _size);
		return items[index];
	}

	S &operator[](S key) {
		for (int i = 0; i < _size; ++i) {
			if (items[i] == key)return items[i];
		}
		return items[_size++];// create new!
	}


	bool operator==(List<S> other) {
		if (_size != other.size())return false;
		for (int i = 0; i < _size; ++i) {
			if (items[i] != other.items[i])return false;
		}
		return true;
	}

	S *begin() {
		return items;
	}

	S *end() {
		return &items[_size];
	}


	int position(S s) {
		for (int i = 0; i < _size; ++i) {
			if (items[i] == s)return i;
			if (eq(items[i], s))return i;// char*
		}
		return -1;
	}

	void sort(bool (comparator)(S a, S b)) {
		heapSort(items, _size, comparator);
	}

	List<S> &sort(bool (comparator)(S &, S &)) {
		heapSort(items, _size, comparator);
		return *this;
	}

	List<S> &sort(float (valuator)(S &a)) {
		heapSort(items, _size, valuator);
		return *this;
	}

	List<S> &sort() {
		heapSort(items, _size);
		return *this;
	}

	bool has(S *item) {
		return position(item) >= 0;
	}

//	bool has(S &item) {
//		return position(item) >= 0;
//	}
	bool has(S item) {
		return position(item) >= 0;
	}

	bool contains(S &item) {
		return position(item) >= 0;
	}

	void clear() {
		items = (S *) calloc(sizeof(S), MAP_ALLOCATION_RESERVED_COUNT);
		_size = 0;
	}
};

// don't use template! just use int-map
template<class S, class T>
class Map {
	int _size = 0;
public:
	S *keys = (S *) calloc(sizeof(S), MAP_ALLOCATION_RESERVED_COUNT);
	T *values = (T *) calloc(sizeof(T), MAP_ALLOCATION_RESERVED_COUNT);

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

	int size() {
		return _size;
	}

	// currently same as map[key]=value
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
	T &operator[](S key) {// CREATING on access! use map.has(x) if not desired
		int position1 = position(key);
		if (position1 < 0) {
			if (use_default) {
				insert_or_assign(key, defaulty);// prepare assignment a[b]=c  BAD because unknown symbols will be added!!
				return values[_size - 1];// MUST USE map.has(x) instead of map[x] otherwise it is created!!
//				return defaulty;// BAD because symbols["missing"]=9 => defaulty=9 wtf
			} else {
				error("MISSING KEY: "s + key);
				printf("MISSING KEY: ");
//				log(key);
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
		free(keys);
		free(values);
		keys = (S *) calloc(sizeof(T), MAP_ALLOCATION_RESERVED_COUNT);
		values = (T *) calloc(sizeof(T), MAP_ALLOCATION_RESERVED_COUNT);
		_size = 0;
	}

	bool use_default = false;
	T defaulty;

	void setDefault(T d) {
		defaulty = d;
		use_default = true;
	}

	S *begin() {
		return &keys[0];
	}

	S *end() {
		return &keys[_size];
	}

	bool has(S s) {
		return position(s) >= 0;
	}
//	void log() {
//		error("use log(map) instead of map(log");
//	}
};

void log(Map<String, int> map);

//int String::in(List<chars> list){
//	for(chars word : list){
//		if(eq(data,word))return list.position(word);
//	}
//	return -1;
//}

#endif //WASP_MAP_H
