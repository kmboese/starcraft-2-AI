#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "InfluenceMap.h"

#define INFINITY 999999

InfluenceMap::InfluenceMap(int rows, int cols) {

    std::vector<std::vector<float> > map;
    
    for (int i = 0; i < rows; ++i) {
        std::vector<float> column(cols);
        map.push_back(column);
    }

    this->map = map;
} // end createMap()

void InfluenceMap::initMap() {

    for (const auto &row : this->map) {
        for (const auto &col : row) {
            map[row][col] = INFINITY;
        }
    }

} // end initMap()

void InfluenceMap::createSource(int x, int y, float rad) {

    InfluenceSource source(x, y, rad);

    int range = round(rad);
    int minX = x - range;
    int maxX = x + range;
    int minY = y - range;
    int maxY = y + range;

    for (int i = minX; i < maxX; ++i) {
        for (int j = maxY; j > minY; --j) {
            map[i][j] = 0;
        }
    }

    sources.push_back(source);
} // end createSource()

void InfluenceMap::propagate(float momentum, float decay, float frequency) {
    for (int i = 0; i < sources.size(); ++i) {
        /*
         * Possible exponential equation:
         * 
         * calculated_inf = inf[curr_cell] * 
         *  exponentialFunc(-cell.distance_from_center * decay)
         * 
         * pseudocode:
         * 
         *  For every source on the map:
         *      Create new influence values based on the 3 parameters using
         *          exponential equation above
         *      New influence value is the max of the calculated value and a
         *          global maximum influence value
         *      Use linear interpolation to "blend" the values using the 
         *          momentum parameter
         *      inf[cell] = new interpolated value
         */ 
    }
} // end propagate()

int InfluenceMap::getNumSources() {
    return sources.size();
}

void InfluenceMap::printMap() const {

} // end printMap()