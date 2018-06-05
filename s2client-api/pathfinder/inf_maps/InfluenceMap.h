#ifndef INF_MAP_H
#define INF_MAP_H

#include <vector>

typedef struct Point
{
    int x;
    int y;

} Point;

/** 
 * @InfluenceSource:
 * 
 * @pt: Position of source
 * @radius: Radius of source's influence.
 */ 
typedef struct InfluenceSource {
    Point pt;
    float radius;

    InfluenceSource(Point pt, float rad) {
        this->pt.x = pt.x;
        this->pt.y = pt.y;
        radius = rad;
    }

} InfluenceSource;

class InfluenceMap {
public:
    /** 
     *  @InfluenceMap: Creates a 2D grid for the influence map. This is based on
     *      our bot's visual map. There will be multiple maps in the system;
     *      for example there will be one map for our bot's units, and another
     *      map for enemy units.
     * 
     *  @rows: Number of rows in the 2D grid.
     *  @cols: Number of columns in the 2D grid.
     */ 
    InfluenceMap(int rows, int cols);

    /** 
     *  @initMap: Initialize each of the map's cells with value 1.
     */ 
    void initMap();

    /**
     *  @createSource: Creates an influence source (an agent in the system). 
     *      Each source has a center point where the influence source is 
     *      stationed). 
     * 
     *  Stores sources in private vector of sources.
     */ 
    void createSource(Point pt, float rad);

    /**
     *  @getNumRows: Return the number of rows in influence map.
     */ 
    unsigned int getNumRows();

    /**
     *  @getNumCols: Return the number of columns in influence map.
     */ 
    unsigned int getNumCols();

    /**
     *  @getNumSources: Return the number of sources in vector of sources.
     */ 
    unsigned int getNumSources();

    /**
     * @getInfMap: Returns the influence map
     */ 
    std::vector<std::vector <float>> getInfMap();

    /**
     *  @calcOverlap: A source's influence can overlap with another source's 
     *      influence. When this happens, add these values.
     */ 
    void calcOverlap();

    /**
     *  @exponentialDecay: Exponential decay function that returns influence
     *      based on a cell's distance from the source.
     * 
     *  @source: The source to base the decay off of.
     *  @cell: The cell to update in the influence map.
     *  @decay: How much influence values decay with distance.
     */ 
    float exponentialDecay(const InfluenceSource &source, const Point cell, 
        float decay);

    /**
     *  @calcCells: Returns a vector of cells from the influence map that the
     *      source has range over based on its radius.
     */ 
    std::vector<Point> calcCells(const InfluenceSource &source);

    /**
     *  @propagate: Updates the influence values of every source on the 
     *      influence map.
     * 
     *  @decay: How much influence values decay with distance.
     */ 
    void propagate(float decay);

    /**
     *  @updateMap: Updates the map. (Update the map based on some defined
     *      frequency.)
     */ 
    void updateMap();

    /** 
     *  @printMap: Prints entire 2D grid with delimiters. Used for debugging.
     */ 
    void printMap();

private:
    std::vector<std::vector <float>> infMap;
    std::vector<InfluenceSource> sources;
};

#endif