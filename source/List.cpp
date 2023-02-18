#include "List.h"
#include "String.h"

#define SORTING 1


//The template definition (the cpp file in your code) has to be included prior to instantiating a given template class
template<class S>
void heapSort(S arr[], int n, bool (comparator)(S &, S &));

template<class S>
void heapSort(S arr[], int n, float (valuator)(S &));

template<class S>
void heapSort(S arr[], int n);


void print(List<String> list) {
    for (String s: list) {
        print(s);
    }
}

template<class S>
void print(List<S> list) {
    for (auto s: list) {
        print(s);
    }
}

