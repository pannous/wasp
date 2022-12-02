//
// Created by me on 06.05.22.
//

#ifndef WASP_LIST_H
#define WASP_LIST_H

#include <cstdlib> // OK in WASM!

#define LIST_DEFAULT_CAPACITY 100 //
#define LIST_MAX_CAPACITY 0x1000000000l // debug only!

#include "Util.h"
#include "wasm_helpers.h"
#include "NodeTypes.h"
#include "ABI.h"

#include <stdarg.h> // va_list OK IN WASM???
//#include <cstdarg> // va_list ok in wasm even without wasi!
#ifndef WASM

#include <initializer_list> // allow List x={1,2,3};

#endif

//bool eq(int i,int j){return i==j;}// for List.has => eq

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


// ⚠️ references to List items are NOT safe during list construction due to List::grow()  ( invalid / forbidden )
template<class S>
class List {
public:
    int header = array_header_32;
    Type _type = 0;// reflection on template class S
    int _size = 0;
    // List<int&> error: 'items' declared as a pointer to a reference of type

    int capacity = LIST_DEFAULT_CAPACITY;// grow() by factor 2 internally on demand

    // todo item references are UNSAFE after grow()
    S *items;// In C++ References cannot be put into an array, if you try you get
//    S items[LIST_DEFAULT_CAPACITY];// array type is not assignable

    List() {
        items = (S *) calloc(capacity, sizeof(S));
    }

    List(const List &old) : items(old.items) { // todo: memcopy?
        _size = old._size;
    }

    List(Array_Header a) {
        if (a.length == 0xA0000000)
            error1("double header");// todo: just shift by 4 bytes
        _size = a.length;
        items = (S *) &a.data;// ok? copy data?
//		todo("a.typ");
    }

    // only Plain Old Data structures !
    List(S first, ...) : List() {
        _size = 0;
        va_list args;// WORKS WITHOUT WASI! with stdargs
        va_start(args, first);
        S item = first;
        do {
            items[_size++] = item;
            item = (S) va_arg(args, S);
        } while (item);
        va_end(args);
    }

    List(S *args) {// initiator list C style {x,y,z,0} ZERO 0 ø TERMINATED!!
        if (args == 0)return;
        while (args[_size] and _size < LIST_DEFAULT_CAPACITY)_size++;
        items = (S *) calloc(sizeof(S), _size + 1);
        int i = _size;
        while (i-- > 0)items[i] = args[i];
    }

//	List(S args) {// initiator list C style {x,y,z}
//		if (args == 0)return;
//		while (args[_size] and _size < LIST_ALLOCATION_RESERVED_COUNT)_size++;
//		int i = _size;
//		while (i-- > 0)items[i] = args[i];
//	}


//	List(S args[]) {}

#ifndef WASM

    List(const std::initializer_list<S> &_items) : List() {
        for (const S &s: _items) {
            items[_size++] = s;
        }
    }

#endif

    int size() { return _size; };

    void setType(Type type) {
        _type = type;
    }

    void grow() {
        check_silent(capacity * 2 < LIST_MAX_CAPACITY);
        S *neu = (S *) alloc(capacity * 2, sizeof(S));
        memcpy((void *) neu, (void *) items, capacity * sizeof(S));
//        warn("⚠️ List.grow memcpy messes with existing references! Todo: add List<items> / wrap S with shared_pointer<S> ?");
//      indeed List<int> FUCKS UP just by growing even without references
        free(items);
        items = neu;
        capacity *= 2;
    }

    S &add(S s) {
        items[_size++] = s;
        if (_size >= capacity)grow();
        return items[_size - 1];
    }

//	S &push_back(S s) {// vector compatible
//		add(s)
//	}

    // list after 1 element
//	List &&operator++() {
//		return List(items + 1);
//	}

    // list FROM index, e.g. [1,2,3]>>1 == [2, 3]
//	List &&operator>>(short index) {
//		return List(items + index);
//	}


    S &operator[](unsigned long index) const {
        if (index < 0 or index >= _size) /* and const means not auto_grow*/
            error("index out of range : %d > %d"s % (long) index % _size);
        return items[index];
    }


    S &operator[](unsigned long index) {
        if (index == _size)_size++;// allow indexing one after end? todo ok?
        if (_size >= capacity)grow();
        if (index < 0 or index >= _size) /* and not auto_grow*/
            error("index out of range : %d > %d"s % (long) index % _size);
        return items[index];
    }

//	S &operator[](S& key) {
//		for (int i = 0; i < _size; ++i) {
//			if (items[i] == key)return items[i];
//		}
//        if (_size >= capacity - 1)grow();
//		return items[_size++];// create new!
//	}


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
        free(items);
        items = (S *) alloc(LIST_DEFAULT_CAPACITY, sizeof(S));
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

    void addAll(S *items[]) {
        for (auto s: items)
            add(s);
    }

    void addAll(S items[]) {
        for (auto s: items)
            add(s);
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


// todo literally use Node for this
enum ListBaseKind {
    Empty,
    LinkedListNode
};

template<typename T>
struct ListBase {
    explicit ListBase(ListBaseKind k) : _kind(k) {}

    ListBaseKind _kind;
};

template<typename T>
struct EmptyList : ListBase<T> {
    EmptyList() : ListBase<T>(Empty) {}
};

template<typename T>
struct ListNode : ListBase<T> {
    ListNode(T const &t, ListBase<T> &next) :
            ListBase<T>(ListBaseKind::LinkedListNode), _value(t), _next(next) {}

    T _value;
    ListBase<T> &_next;
};

template<typename T>
struct LinkedList : ListNode<T> {
};


#endif //WASP_LIST_H
