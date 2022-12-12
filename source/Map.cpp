//
// Created by pannous on 13.10.20.
//

#include "Map.h"
//template<class S,class T>
//void put(Map<S,T> map){
//	printef("Map (size: %d)\n", map.size());
//	for (int i = 0; i < map.size(); ++i)
//		printef("%s: %d\n", map.keys[i].data, map.values[i]);
//}

void print(Map<String, int> map) {
    printef("Map (size: %d)\n", map.size());
	for (int i = 0; i < map.size(); ++i)
        printef("%s: %d\n", map.keys[i].data, map.values[i]);
}
