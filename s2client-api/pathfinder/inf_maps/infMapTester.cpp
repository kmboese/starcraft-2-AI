#include <iostream>
#include <stdlib.h>
#include <vector>

#include "InfluenceMap.h"

void TEST_INIT_MAP(int rows, int cols);

int main() {
    TEST_INIT_MAP(4, 3);
    
    return 0;
}

void TEST_INIT_MAP(int rows, int cols) {
    InfluenceMap map(rows, cols);
    map.initMap();
    map.printMap();
}