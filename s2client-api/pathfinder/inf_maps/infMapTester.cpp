#include <assert.h>
#include <iostream>
#include <stdlib.h>
#include <vector>

#include "InfluenceMap.h"

void TEST_INIT_MAP(int rows, int cols);
void TEST_CREATE_SOURCE(InfluenceMap map);
void TEST_PROPAGATE(InfluenceMap map);
void TEST_MAX_PROPAGATE();

int main() {
    std::cout << "Testing initMap\n" << std::endl;
    TEST_INIT_MAP(4, 3);

    InfluenceMap map(10, 10);
    map.initMap();

    std::cout << "\nTesting createSource\n" << std::endl;

    TEST_CREATE_SOURCE(map);

    std::cout << "\nTesting propagate\n" << std::endl;
    TEST_PROPAGATE(map);
    
    return 0;
}

void TEST_INIT_MAP(int rows, int cols) {
    InfluenceMap map(rows, cols);
    map.initMap();
    assert(map.getNumRows() == (unsigned int)rows);
    assert(map.getNumCols() == (unsigned int)cols);

    map.printMap();
}

void TEST_CREATE_SOURCE(InfluenceMap map) {
    unsigned int numSources = 2;
    Point pt1;
    pt1.x = 5;
    pt1.y = 4;
    map.createSource(pt1, 3.5);

    Point pt2;
    pt2.x = 0;
    pt2.y = 9;
    map.createSource(pt2, 0.2);

    assert(map.getNumSources() == numSources);
    
    std::cout << "Num sources: " << map.getNumSources() << std::endl;

    float decay = 0;
    map.propagate(decay);
    map.printMap();
}

void TEST_PROPAGATE(InfluenceMap map) {
    // Create 2 sources
    Point pt1;
    pt1.x = 5;
    pt1.y = 4;
    map.createSource(pt1, 3.5);

    Point pt2;
    pt2.x = 1;
    pt2.y = 4;
    map.createSource(pt2, 0.2);

    // Test propagate for one cycle
    float decay = 0.5;
    map.propagate(decay);
    map.printMap();
}

// Create a massive map and check values
void TEST_MAX_PROPAGATE() {

}