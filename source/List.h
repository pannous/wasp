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
#include "Map.h"

#include <stdarg.h> // va_list OK IN WASM???
//#include <cstdarg> // va_list ok in wasm even without wasi!
#include <initializer_list> // works in wasi!!  allow List x={1,2,3};

#ifndef WASM

#endif

//The template definition (the cpp file in your code) has to be included prior to instantiating a given template class
template<class S>
void heapSort(S arr[], int n, bool (comparator)(S &, S &));

template<class S>
void heapSort(S arr[], int n, float (valuator)(S &));

template<class S>
void heapSort(S arr[], int n);


// ⚠️ references to List items are NOT safe during list construction due to List::grow()  ( invalid / forbidden )
template<class S>
class List {
public:
    int header = array_header_32;// wasn't this moved down ?
    Type _type{};// reflection on template class S
    int size_ = 0;
    int capacity = LIST_DEFAULT_CAPACITY;// grow() by factor 2 internally on demand
    // previous entries must be aligned to int64!
    S *items = 0;// 32bit pointers in wasm! In C++ References cannot be put into an array, if you try you get
    // List<int&> error: 'items' declared as a pointer to a reference of type

    // todo item references are UNSAFE after grow()
//    S items[LIST_DEFAULT_CAPACITY];// array type is not assignable


//    List(int size = LIST_DEFAULT_CAPACITY) {
//        capacity = size;
//        items = (S *) calloc(size, sizeof(S));
//    }

    List(size_t size = LIST_DEFAULT_CAPACITY) {
        capacity = size;
        items = (S *) calloc(size, sizeof(S));
    }


    List(const List &old) : items(old.items) { // todo: memcopy?
        size_ = old.size_;
    }

    List(Array_Header a) {
        if (a.length == 0xA0000000)
            error1("double header");// todo: just shift by 4 bytes
        size_ = a.length;
        items = (S *) &a.data;// ok? copy data?
//		todo("a.typ");
    }


//    List(S first, ...) : List() {
////		items[0] = first;
//        va_list args;// WORKS WITHOUT WASI! with stdargs
//        va_start(args, first);
//        S *item = &first;
//        while (item) {
//            items[_size++] = *item;
//            item = (S *) va_arg(args, S*);
//        }
//        va_end(args);
//    }



//#ifndef PURE_WASM
    List(const std::initializer_list<S> &inis) : List() {
        auto item_count = inis.end() - inis.begin();
        while (item_count >= capacity)grow();
        for (const S &s: inis) {
            if (&s == nullptr)continue;
            items[size_++] = s;
        }
    }
//#endif

    // todo: can get removed thanks to initializer_list
    // only Plain Old Data structures! works outside wasm but shouldn't
    List(S first, ...) : List() {
        size_ = 0;
        va_list args;// WORKS WITHOUT WASI! with stdargs
        va_start(args, first);
        S item = first;
        do {
            print("va_arg#");
            print(size_);
            print(item);
            items[size_++] = item;
            item = (S) va_arg(args, S);
        } while (item);
        va_end(args);
    }


    // todo get rid in favor of vararg /  initializer_list!
//	List(S args[])  same
//    List(S *args) {// initiator list C style {x,y,z,0} ZERO 0 ø TERMINATED!!
//        if (args == 0)return;
//        todo("get rid in favor of vararg /  initializer_list");
//        while (args[size_] and size_ < LIST_DEFAULT_CAPACITY)size_++;
//        items = args;
////        items = (S *) calloc(sizeof(S), size_ + 1);
////        int i = size_;
////        while (i-- > 0)items[i] = args[i];
//    }

    List(S *args, int count, bool share = true) {
        if (args == 0)return;
//        check_silent(count < LIST_MAX_CAPACITY)
        size_ = count;
        if (share)
            items = args;
        else {
            if (capacity < size_ or not items) {
                items = (S *) calloc(size_ + 1, sizeof(S));
                capacity = size_;
            }
            memcpy((void *) items, (void *) args, count * sizeof(S));
        }
    }


    List(S *data, S *end, bool share = true) : List(data, (end - data) / sizeof(S), share) {}


//	List(S args) {// initiator list C style {x,y,z}
//		if (args == 0)return;
//		while (args[_size] and _size < LIST_ALLOCATION_RESERVED_COUNT)_size++;
//		int i = _size;
//		while (i-- > 0)items[i] = args[i];
//	}


//    ~List() { // double-free
//        if (not shared)
//            free(items);
//    }


    size_t size() const { return size_; };

    void setType(Type type) {
        _type = type;
    }

    void grow() {
//        warn("grow");
        auto new_size = capacity * 2;
//        check_silent(new_size < LIST_MAX_CAPACITY);
        S *neu = (S *) alloc(new_size, sizeof(S));
        memcpy((void *) neu, (void *) items, capacity * sizeof(S));
        if (items)free(items);
//        warn("⚠️ List.grow memcpy messes with existing references! Todo: add List<items> / wrap S with shared_pointer<S> ?");
//      indeed List<int> FUCKS UP just by growing even without references
        items = neu;
        capacity = new_size;
    }

    S &add(S s) {
//        if(!items)grow();// how?
        items[size_++] = s;
        if (size_ >= capacity)grow();
        return items[size_ - 1];
    }

    S &add(S *s) {
        items[size_++] = s;
        if (size_ >= capacity)grow();
        return items[size_ - 1];
    }

//	S &add(S s) {// vector compatible
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


    S &operator[](uint64 index) const {
        if (index < 0 or index >= size_) /* and const means not auto_grow*/
            error("index out of range : %d > %d"s % (int64) index % size_);
        return items[index];
    }


//    S &operator[](uint64 index) {
//        if (index == size_)size_++;// allow indexing one after end? todo ok?
//        bool auto_grow= false;
//        if (auto_grow and size_ > capacity)grow();
////        if (index >= capacity)grow();
//        if (index < 0 or index >= size_) { /* and not auto_grow*/
//            if (index >= capacity)
//                error("index out of range : %d > %d"s % (int64) index % size_);
//            else size_ = index + 1;
//        }
//        return items[index];
//    }

//	S &operator[](S& key) {
//		for (int i = 0; i < _size; ++i) {
//			if (items[i] == key)return items[i];
//		}
//        if (_size >= capacity - 1)grow();
//		return items[_size++];// create new!
//	}


    bool operator==(List<S> other) const {
        if (size_ != other.size())return false;
        for (int i = 0; i < size_; ++i) {
            if (items[i] != other.items[i])return false;
        }
        return true;
    }

    S *begin() const {
        return items;
    }

    S *end() const {
        return &items[size_];
    }


    int position(S s) {
        for (int i = 0; i < size_; ++i) {
            if (items[i] == s)return i;
            if (eq(items[i], s))return i;// needs to be defined for all S
        }
        return -1;
    }


    int position(S *s) {
        if (!s)return -1;// don't allow null pointer!
        for (int i = 0; i < size_; ++i)
            if (&items[i] == s)return i;
        return -1;
    }

//
//    int position(S& s) {
//        for (int i = 0; i < _size; ++i)
//            if (&items[i] == s)return i;
//        return -1;
//    }

    void sort(bool (comparator)(S a, S b)) {
        heapSort(items, size_, comparator);
    }

    List<S> &sort(bool (comparator)(S &, S &)) {
        heapSort(items, size_, comparator);
        return *this;
    }

    List<S> &sort(float (valuator)(S &a)) {
        heapSort(items, size_, valuator);
        return *this;
    }

    List<S> &sort() {
        heapSort(items, size_);
        return *this;
    }

    bool has(S *item) {
        if (!item)return false;
        return position(item) >= 0;
    }

//	bool has(S &item) {
//		return position(item) >= 0;
//	}
    bool has(S item) {
        return position(item) >= 0;
    }

    bool contains(S *item) {
        return position(item) >= 0;
    }

    bool contains(S item) {
        return position(item) >= 0;
    }

    void clear() {
        free(items);
        items = (S *) alloc(LIST_DEFAULT_CAPACITY, sizeof(S));
        size_ = 0;
    }

    void remove(S &item) {
        auto pos = position(item);
        if (pos < 0)return;
        memmove(items + pos, items + pos + 1, size_ - pos);
        size_--;
    }

    bool remove(short position) {
        if (position < 0 or size_ <= 0 or position >= size_)return false;
        memmove((void *) (items + position), (void *) (items + position + 1), (size_ - position) * sizeof(S));
        size_--;
        return true;
    }

//	List<S>& clone() { // todo just create all return lists with new List() OR return List<> objects (no references) copy by value ok!!
//		List &neu = *new List<S>();
//		neu._size=_size;
//		neu.items=items;
//		return  neu;
//	}
    bool empty() const {
        return size_ == 0 or items == 0;
    }

    void addAll(S *items[]) {
        for (auto s: items)
            add(s);
    }

    void addAll(S items[]) {
        for (auto s: items)
            add(s);
    }

    template<class T>
    List<T> map(T (*lambda)(S)) {
        List<T> neu;
        for (auto s: *this) {
            neu.add(lambda(s));
        }
        return neu;
    }

    S *find(bool (*lambda)(S &)) {
        for (S *s: *this)
            if (lambda(s))return s;
        return 0;
    }


    String join(String string) {
        String s;
        for (auto i: *this) {
            if (not i)continue;
            s += i;
            s += string;
        }
        return s;
    }

    S &back() {// same as last()
        if (size_ <= 0)error("no back() in empty list.");
        return items[size_ - 1];
    }

    S &last() {
        if (size_ < 1) error("empty list");
        return items[size_ - 1];
    }


    S last(S defaulty) {
        if (size_ < 1)
            return defaulty;
        return items[size_ - 1];
    }


    S &add() {
        size_++;
        if (size_ >= capacity)grow();
        return last();
    }

    void resize(long new_size) {
        if (new_size >= capacity)
            grow();
        size_ = new_size;
    }

    void append(S *value, size_t len) {
        if (size_ + len >= capacity) {
            grow();
        }
        memcpy((void *) &items[size_], (void *) value, len * sizeof(S));
        size_ += len;
    }

//    void insert(S *position, S *value, S *end) {
//        size_t len = end - value;
//        if (_size + len >= capacity){
//            grow();
//            position = &last();
//        }
//        memcpy(position, value, len);
//        _size += len;
//    }

    S *data() const {
        return items;
    }

    S &first() {
        return items[0];
    }

    bool shared = false;

    S &at(int i) {
        if (i < 0 or i >= size_)error("out of bounds");
        return items[i];
    }

    int indexOf(S &item) {
        return position(&item);
    }
};

void print(List<String> list);

template<typename S>
void print(List<S> list);

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


//bool eq(int i,int j){return i==j;}// for List.has => eq




//inline
template<class S>
void swap2(S *a, S *b) {
    // todo!!!
//#if SORTING
    S c = *a;
    *a = *b;
    *b = c;
//#endif
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
        swap2(&arr[i], &arr[largest]);
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
        swap2(&arr[i], &arr[largest]);
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
        swap2(&arr[i], &arr[largest]);
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
        swap2(&arr[0], &arr[i]);
        // call max heapify on the reduced heap
        heapify(arr, i, 0);
    }
}

template<class S>
void heapSort(S arr[], int n, float (valuator)(S &)) {
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i, valuator);
    for (int i = n - 1; i > 0; i--) {
        swap2(&arr[0], &arr[i]);
        heapify(arr, i, 0, valuator);
    }
}

template<class S>
void heapSort(S arr[], int n, bool (comparator)(S &, S &)) {
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i, comparator);
    for (int i = n - 1; i > 0; i--) {
        swap2(&arr[0], &arr[i]);
        heapify(arr, i, 0, comparator);
    }
}


#endif //WASP_LIST_H
