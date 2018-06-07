#include <iostream>
#include <limits>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "InfluenceMap.h"

#define INF std::numeric_limits<int>::max()
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Maximum influence assuming influence map only deals with positive values
#define maxInf 9999

// Power function for integers
int int_pow(int base, int exp) {
    int result = 1;
    
    for (int i = 0; i < exp; ++i) {
        result *= base;
    }
    return result;
}

InfluenceMap::InfluenceMap(int y_rows, int x_cols) {
    if (y_rows < 0 || x_cols < 0) {
        std::cerr << "Invalid dimensions--dimensions cannot be negative\n" 
            << std::endl;
        exit(0);
    }

    std::vector<std::vector<float> > infMap;
    
    for (int i = 0; i < y_rows; ++i) {
        std::vector<float> column(x_cols);
        infMap.push_back(column);
    }

    this->infMap = infMap;
} // end createMap()


void InfluenceMap::initMap() {
    // Rows correspond to y-axis; columns correspond to x-axis.
    for (unsigned int row = 0; row < infMap.size(); ++row) {
        for (unsigned int col = 0; col < infMap[row].size(); ++col) {
            infMap[row][col] = INF;
        }
    }
} // end initMap()


void InfluenceMap::createSource(int x, int y, float rad) {
    Point pt;
    pt.x = x;
    pt.y = y;

    InfluenceSource src(pt, rad);

    // int roundedRadius = round(src.radius);

    /* Find bounds of source. Rows correspond to y-axis; columns correspond to
        x-axis. */
    int minX = floor(src.pt.x - src.radius);
    int minY = floor(src.pt.y - src.radius);
    int maxX = round(src.pt.x + src.radius);
    int maxY = round(src.pt.y + src.radius);

    if (minX > maxX || minY > maxY) {
        std::cerr << "Invalid influence source in createSource\n" << std::endl;
        exit(0);
    }

    // Translate vector 2d grid into Cartesian plane.
    for (int i = maxY; i >= minY; --i) { // rows: y
        /* Radius out of bounds. Should exit since further calculations will not
          be saved. */
        if (i < 0) {
            break;
        }

        // Radius out of bounds. Continue until hit first element in map.
        if ((unsigned int)i >= infMap.size()) {
            continue;
        }

        for (int j = minX; j <= maxX; ++j) { // cols: x
            if (j >= 0 && (unsigned int)j >= infMap[i].size()) {
                break;
            }

            if (j < 0) {
                continue;
            }

            // If distance from point to center is leq the radius of the circle,
            //  initialize the circle. 
            int x_coor = src.pt.x - j;
            int y_coor = src.pt.y - i;
            if ((int_pow(x_coor, 2) + int_pow(y_coor, 2)) 
                <= int_pow(src.radius, 2))
            {
                infMap[i][j] = 0;
            }
        }
    }  

    sources.push_back(src);
} // end createSource()

unsigned int InfluenceMap::getNumRows() {
    return infMap.size();
}

unsigned int InfluenceMap::getNumCols() {
    return infMap[0].size();
}

unsigned int InfluenceMap::getNumSources() {
    return sources.size();
}

std::vector<std::vector <float>> InfluenceMap::getInfMap() {
    return infMap;
}


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

    if (minX > maxX || minY > maxY) {
        std::cerr << "Invalid influence source in calcCells\n" << std::endl;
        exit(0);
    }

    for (int i = minX; i <= maxX; ++i) {
        // Radius out of bounds. Continue until hit first element in map.
        if (i < 0) {
            continue;
        }
        /* Radius out of bounds. Should exit since further calculations will not
          be saved. */
        if ((unsigned int)i >= infMap.size()) {
            break;
        }

        for (int j = minY; j <= maxY; ++j) {
            if (j < 0) {
                continue;
            }

            if ((unsigned int)j >= infMap[i].size()) {
                break;
            }
            // Contruct point without an actual constructor because of InfSource
            //  constructor conflicts
            Point cell;
            cell.x = i;
            cell.y = j;

            cells.push_back(cell);
        }
    }

    return cells;    // propagate map

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
            //  value based in the cell's distance. Set previous influence to 1
            //  on first propagation and to avoid multiplying by 0; may cause
            //  incorrect values after more than 1 propagation...
            float prevInf = infMap[x][y];
            if (prevInf == 0) {
                prevInf = 1;
            }
            calculatedInf = prevInf * exponentialDecay(sources[i], 
                cells[j], decay);

            // Cap the influence
            infMap[x][y] += MIN(calculatedInf, maxInf);
        }
    }
} // end propagate()

void InfluenceMap::updateMap(float decay) {
    // Clear influence map
    initMap();

    // Propagate map
    propagate(decay);
}

void InfluenceMap::printMap() {
    std::vector< std::vector<float> >::const_iterator row;
    std::vector<float>::const_iterator col;

    std::cout << "Rows: " << getNumRows() << std::endl;
    std::cout << "Columns: " << getNumCols() << "\n" << std::endl;
    
    for (row = infMap.begin(); row != infMap.end(); ++row) {
        for (col = row->begin(); col != row->end(); ++col) {
            std::cout << " | " << *col << " | ";
        }
        std::cout << std::endl;
    }
} // end printMap()