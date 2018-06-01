#include <assert.h>
#include <iostream>
#include <stdlib.h>
#include <vector>

#include "InfluenceMap.h"

void TEST_INIT_MAP(int rows, int cols);
void TEST_CREATE_SOURCE(InfluenceMap &map);

int main() {
    std::cout << "Testing initMap\n" << std::endl;
    TEST_INIT_MAP(4, 3);

    std::cout << "\nTesting createSource\n" << std::endl;

    InfluenceMap map(10, 10);
    map.initMap();
    TEST_CREATE_SOURCE(map);
    
    return 0;
}

void TEST_INIT_MAP(int rows, int cols) {
    InfluenceMap map(rows, cols);
    map.initMap();
    assert(map.getNumRows() == (unsigned int)rows);
    assert(map.getNumCols() == (unsigned int)cols);

    map.printMap();
}

void TEST_CREATE_SOURCE(InfluenceMap &map) {
    unsigned int numSources = 2;
    Point pt1;
    pt1.x = 5;
    pt1.y = 4;
    map.createSource(pt1, 3.5);

    Point pt2;
    pt2.x = 1;
    pt2.y = 4;
    map.createSource(pt2, 0.2);

    assert(map.getNumSources() == numSources);
    
    std::cout << "Num sources: " << map.getNumSources() << std::endl;
}