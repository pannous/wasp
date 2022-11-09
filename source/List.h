//
// Created by me on 06.05.22.
//

#ifndef WASP_LIST_H
#define WASP_LIST_H

#include <cstdlib> // OK in WASM!

#define LIST_ALLOCATION_RESERVED_COUNT 100 // Todo!

#include "Util.h"
#include "wasm_helpers.h"
#include "NodeTypes.h"
#include "ABI.h"

#include <stdarg.h> // va_list OK IN WASM???
//#include <cstdarg> // va_list ok in wasm even without wasi!
#ifndef WASM

#include <initializer_list> // allow List x={1,2,3};

#endif


//The template definition (the cpp file in your code) has to be included prior to instantiating a given template class
template<class S>
void heapSort(S arr[], int n, bool (comparator)(S &, S &));

template<class S>
void heapSort(S arr[], int n, float (valuator)(S &));

template<class S>
void heapSort(S arr[], int n);


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
public:
	int header = array_header_32;
	int _type = 0;// reflection on template class S
	int _size = 0;
	S *items;

	List() {
		items = (S *) calloc(sizeof(S), LIST_ALLOCATION_RESERVED_COUNT);
	}

	List(Array_Header a) {
		if (a.length == 0xA0000000)
			error1("double header");// todo: just shift by 4 bytes
		_size = a.length;
		items = (S *) &a.data;// ok? copy data?
//		todo("a.typ");
	}

	List(S first, ...) : List() {
//		items[0] = first;
		va_list args;// WORKS WITHOUT WASI! with stdargs
		va_start(args, first);
		S *item = &first;
		while (item) {
			items[_size++] = item;
			item = (S *) va_arg(args, S*);
		}
		va_end(args);
	}

	List(S *args) {// initiator list C style {x,y,z,0} ZERO 0 ø TERMINATED!!
		if (args == 0)return;
		while (args[_size] and _size < LIST_ALLOCATION_RESERVED_COUNT)_size++;
		items = (S *) calloc(sizeof(S), _size + 1);
		int i = _size;
		while (i-- > 0)items[i] = args[i];
	}

#ifndef WASM

	List(const std::initializer_list<S> &_items) : List() {
		for (const S &s: _items) {
			if (&s == 0) // todo doesn't work for String because {"a","b",0} => String(0)!
				error1("initializer_lists DO NOT NEED 0 termination!");// break;
			items[_size++] = s;
		}
	}

#endif

	int size() { return _size; };

	void setType(Type type) {
		_type = type;
	}

	S &add(S s) {
		items[_size++] = s;
		return items[_size - 1];
	}

	// list after 1 element
	List &&operator++() {
		return List(items + 1);
	}

	// list FROM index, e.g. [1,2,3]>>1 == [2, 3]
	List &&operator>>(short index) {
		return List(items + index);
	}


	S &operator[](short index) {
		if (index < 0 or index > _size) /* and not auto_grow*/
			error("index out of range : %d > %d"s % index % _size);
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

	S *begin() const {
		return items;
	}

	S *end() const {
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
		items = (S *) alloc(sizeof(S), LIST_ALLOCATION_RESERVED_COUNT);
//		items = (S *) calloc(sizeof(S), LIST_ALLOCATION_RESERVED_COUNT);
		_size = 0;
	}

	void remove(S &item) {
		auto pos = position(item);
		if (pos < 0)return;
		memcpy(items + pos, items + pos + 1, _size - pos);
		_size--;
	}

	S last(S defaulty) {
		if (_size < 1)
			return defaulty;
		return items[_size - 1];
	}

	S &last() {
		if (_size < 1)
			error("empty list");
		return items[_size - 1];
	}

//	List<S>& clone() { // todo just create all return lists with new List() OR return List<> objects (no references) copy by value ok!!
//		List &neu = *new List<S>();
//		neu._size=_size;
//		neu.items=items;
//		return  neu;
//	}
	bool empty() const {
		return _size == 0 or items == 0;
	}
};

void print(List<String> list);

//int String::in(List<chars> list){
//	for(chars word : list){
//		if(eq(data,word))return list.position(word);
//	}
//	return -1;
//}


template<class S>
class List;

template<class S>
bool contains(List<S> list, S match);


#endif //WASP_LIST_H
