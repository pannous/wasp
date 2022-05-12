//
// Created by me on 06.05.22.
//

#ifndef WASP_LIST_H
#define WASP_LIST_H

#include <cstdlib> // OK in WASM!

#define LIST_ALLOCATION_RESERVED_COUNT 100 // Todo!

#include "wasm_helpers.h"
#include "Util.h"
#include "NodeTypes.h"

#include <stdarg.h> // va_list OK IN WASM???
//#include <cstdarg> // va_list ok in wasm even without wasi!
#ifndef WASM

#include <initializer_list> // allow List x={1,2,3};

#endif

template<class S>
class List {
	int _size = 0;
public:
	S *items;

	List() {
		items = (S *) calloc(sizeof(S), LIST_ALLOCATION_RESERVED_COUNT);
	}

	List(Array_header a) {
		if (a.len == 0xA0000000)error1("double header");// todo: just shift by 4 bytes
		_size = a.len;
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

	List(S *args) : List() {// initiator list C style {x,y,z,0} ZERO 0 ø TERMINATED!!
		while (args[_size] and _size < LIST_ALLOCATION_RESERVED_COUNT) {
			items[_size] = args[_size];
			_size++;
		}
	}

#ifndef WASM

	List(const std::initializer_list<S> &_items) : List() {
		for (const S &s: _items) {
			items[_size++] = s;
		}
	}

#endif

	int size() { return _size; };

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
