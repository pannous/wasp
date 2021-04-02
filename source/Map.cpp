//
// Created by pannous on 13.10.20.
//

#include "Map.h"

void log(Map<String, int> map) {
	printf("Map (size: %d)\n", map.size());
	for (int i = 0; i < map.size(); ++i)
		printf("%s: %d\n", map.keys[i].data, map.values[i]);
}
