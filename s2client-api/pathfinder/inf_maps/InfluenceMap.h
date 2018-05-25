#ifndef INF_MAP_H
#define INF_MAP_H

#include <vector>

/* 
 * @InfluenceSource:
 * 
 * @x: Center of source along the x axis.
 * @y: Center of source along the y axis.
 * @radius: Radius of source's influence.
 */ 
typedef struct InfluenceSource {
    int x;
    int y;
    float radius;

    InfluenceSource(int x, int y, float rad) {
        this->x = x;
        this->y = y;
        radius = rad;
    }

} InfluenceSource;

class InfluenceMap {
public:
    /* 
     *  @InfluenceMap: Creates a 2D grid for the influence map. This is based on
     *      our bot's visual map. There will be multiple maps in the system;
     *      for example there will be one map for our bot's units, and another
     *      map for enemy units.
     * 
     *  @rows: Number of rows in the 2D grid.
     *  @cols: Number of columns in the 2D grid.
     */ 
    InfluenceMap(int rows, int cols);

    /* 
     *  @initMap: Initialize each of the map's cells with value of infinity.
     */ 
    void initMap();

    /*
     *  @createSource: Creates an influence source (an agent in the system). 
     *      Each source has a center point where the influence source is 
     *      stationed).
     * 
     *  Stores sources in private vector of sources.
     */ 
    void createSource(int x, int y, float rad);

    /*
     *  @calcOverlap: A source's influence can overlap with another source's 
     *      influence. When this happens, add these values.
     */ 
    void calcOverlap();

    /*
     *  @propagate: Updates the influence values of every source on the 
     *      influence map.
     * 
     *  @momentum: Whether or not the updated influence
     *      value will be biased towards existing value or the new calculated
     *      value.
     *  @decay: How much influence values decay with distance.
     *  @frequency: How often to update the influence values.
     */ 
    void propagate(float momentum, float decay, float frequency);

    /*
     *
     */ 
    float linearInterpolation(float momentum);

    /*
     *
     */ 
    void updateMap();

    /* 
     *  @printMap: Prints entire 2D grid with delimiters. Used for debugging.
     */ 
    void printMap() const;

    int getNumSources();
private:
    std::vector<std::vector <float>> map;
    std::vector<InfluenceSource> sources;
};

#endif