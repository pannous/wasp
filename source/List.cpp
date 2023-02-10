#include "List.h"
#include "String.h"
#define SORTING 1

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
