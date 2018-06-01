#include <iostream>
#include <limits>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "InfluenceMap.h"

#define INF std::numeric_limits<int>::max()
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Maximum influence assuming influence map only deals with positive values
#define maxInf 9999

InfluenceMap::InfluenceMap(int rows, int cols) {
    std::vector<std::vector<float> > infMap;
    
    for (int i = 0; i < rows; ++i) {
        std::vector<float> column(cols);
        infMap.push_back(column);
    }

    this->infMap = infMap;
} // end createMap()


void InfluenceMap::initMap() {
    for (unsigned int row = 0; row < infMap.size(); ++row) {
        for (unsigned int col = 0; col < infMap[row].size(); ++col) {
            infMap[row][col] = 1;
        }
    }
} // end initMap()


void InfluenceMap::createSource(Point pt, float rad) {

    InfluenceSource source(pt, rad);

    int range = round(rad);
    int minX = pt.x - range;
    int maxX = pt.x + range;
    int minY = pt.y - range;
    int maxY = pt.y + range;

    for (int i = minX; i < maxX; ++i) {
        for (int j = maxY; j > minY; --j) {
            infMap[i][j] = 0;
        }
    }

    sources.push_back(source);
} // end createSource()


unsigned int InfluenceMap::getNumSources() {
    return sources.size();
}


void InfluenceMap::calcOverlap() {
// TODO: calcOverlap and deleteOldInfMap
} // end calcOverlap()


float InfluenceMap::exponentialDecay(const InfluenceSource &source, 
    const Point cell, float decay)
{

    // Distance from cell to source center
    float distance = 0;

    // Calculate distance
    float innerCalc = pow(cell.x - source.pt.x , 2) 
        + pow(cell.y - source.pt.y , 2);
    distance = sqrt(innerCalc);

    return exp(-1 * distance * decay);
} // end exponentialDecay()


std::vector<Point> InfluenceMap::calcCells(const InfluenceSource &source) {
    // Cells within radius of source. For now it will just be a square
    //  around the source's center.
    std::vector<Point> cells;

    int range = round(source.radius);
    int minX = source.pt.x - range;
    int maxX = source.pt.x + range;
    int minY = source.pt.y - range;
    int maxY = source.pt.y + range;

    for (int i = minX; i < maxX; ++i) {
        for (int j = maxY; j > minY; --j) {
            // Contruct point without an actual constructor because of InfSource
            //  constructor conflicts
            Point cell;
            cell.x = i;
            cell.y = j;

            cells.push_back(cell);
        }
    }

    return cells;
} // end calcCells

void InfluenceMap::propagate(float decay) {
    // Calculated influence
    float calculatedInf = INF;

    for (unsigned int i = 0; i < getNumSources(); ++i) {
        std::vector<Point> cells = calcCells(sources[i]);
        for (unsigned int j = 0; j < cells.size(); ++j) {
            int x = cells[j].x;
            int y = cells[j].y;

            // Multiply the current influence value with the exponential decay
            //  value based in the cell's distance
            calculatedInf = infMap[x][y] * exponentialDecay(sources[i], 
                cells[j], decay);

            // Cap the influence
            infMap[x][y] += MAX(calculatedInf, maxInf);
        }
    }
} // end propagate()

void InfluenceMap::printMap() const {
    std::vector< std::vector<float> >::const_iterator row;
    std::vector<float>::const_iterator col;

    for (row = infMap.begin(); row != infMap.end(); ++row) {
        for (col = row->begin(); col != row->end(); ++col) {
            std::cout << *col << std::endl;
        }
    }
} // end printMap()