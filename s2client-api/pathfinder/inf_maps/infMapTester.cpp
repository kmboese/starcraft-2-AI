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
    // std::cout << "Testing initMap\n" << std::endl;
    // TEST_INIT_MAP(4, 3);

    InfluenceMap map(10, 10);
    map.initMap();

    std::cout << "\nTesting createSource\n" << std::endl;

    TEST_CREATE_SOURCE(map);

    // std::cout << "\nTesting propagate\n" << std::endl;
    // TEST_PROPAGATE(map);

    // std::cout << "\nFINISHED TESTING\n" << std::endl;
    
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
    // unsigned int numSources = 2;
    // map.createSource(5, 4, 3.5);

    map.createSource(0, 9, 21);

    // assert(map.getNumSources() == numSources);
    
    std::cout << "Num sources: " << map.getNumSources() << std::endl;

    // float decay = 0;
    // map.propagate(decay);
    map.printMap();
}

void TEST_PROPAGATE(InfluenceMap map) {
    // Create 2 sources
    map.createSource(5, 4, 3.5);
    map.createSource(1, 4, 0.2);

    // Test propagate for one cycle
    float decay = 0.5;
    map.propagate(decay);
    map.printMap();
}

// Create a massive map and check values
void TEST_MAX_PROPAGATE() {

}

// Create 2 sources and update 3 times
void TEST_UPDATE() {

}