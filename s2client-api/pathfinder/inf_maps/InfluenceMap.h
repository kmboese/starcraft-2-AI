#ifndef INF_MAP_H
#define INF_MAP_H

#include <vector>

typedef struct Point
{
    int x;
    int y;

    Point() {}

    Point(int x, int y) {
        this->x = x;
        this->y = y;
    }
} Point;

/** 
 * @InfluenceSource:
 * 
 * @pt: Position of source
 * @radius: Radius of source's influence.
 */ 
typedef struct InfluenceSource
{
    Point pt;
    float radius;

    InfluenceSource() {}

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
     *  @rows: Number of rows in the 2D grid. The y-axis.
     *  @cols: Number of columns in the 2D grid. The x-axis.
     */ 
    InfluenceMap(int y_rows, int x_cols);

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
     *  @createMultSources: Creates multiple influence sources. Used for
     *      updating map or creating all sources at once.
     * 
     *  @srcs: Vector of sources.
     * 
     *  Stores sources in private vector of sources.
     */ 
    void createMultSources(std::vector<InfluenceSource> &srcs);

    /**
     *  @updateAllSources: Updates sources vector with influence source's new 
     *      positon.
     * 
     *  @srcs: Vector of sources with new locations.
     * 
     *  Stores sources in private vector of sources.
     */ 
    void updateAllSources(const std::vector<InfluenceSource> srcs);

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
    std::vector<Point> calcCells(const InfluenceSource &src);

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
     * 
     *  @decay: How much influence values decay with distance. Should be the
     *      same value used for propagate decay.
     */ 
    void updateMap(const std::vector<InfluenceSource> srcs, float decay);

    /** 
     *  @printMap: Prints entire 2D grid with delimiters. Used for debugging.
     */ 
    void printMap();

private:
    /**
     * @infMap: The influence map. Uses Cartesian coordinates (SC2 coordinates),
     *      so bottom left is (0,0) and top right is (maxX, maxY). X-coor are on
     *      horizontal axis and y-coor are on vertical axis.
     */ 
    std::vector<std::vector <float>> infMap;
    std::vector<InfluenceSource> sources;
};

#endif