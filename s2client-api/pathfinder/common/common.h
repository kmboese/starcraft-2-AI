#include "sc2api/sc2_api.h"
#include "astar.h"
#include <iostream>

#include "sc2renderer/sc2_renderer.h"

#define LINUX_USE_SOFTWARE_RENDER 0


//Constants
const int MAP_BOUNDS_BUFFER = 10; //buffer space around the playable map boundaries to avoid A* errors
//Radius within which units are considered near each other
const float UNIT_RADIUS = 1.0f;
//Radius within which units are considered near a point
const float POINT_RADIUS = 2.0f;
//Radius marines must be away from each other
const float SEPARATION_RADIUS = 1.0f;
//Multiplier for separation of units
const float SEPARATION_MULT = 5.0f;
//Radius marines must be away from their centroid
const float CENTROID_RADIUS = 2.0f;
//Radius within which the leader is considered to have reached the goal
const float GOAL_RADIUS = 10.0f;
const float TILE_RADIUS = float(sqrt(2)); //Distance from a tile we are considered near to 

//linux rendering variables
//#if defined(__linux__)
    const float kCameraWidth = 48.0f;
    const int kFeatureLayerSize = 80;
    const int kPixelDrawSize = 3;
    const int kDrawSize = kFeatureLayerSize * kPixelDrawSize * 2;
//#endif

//Scale for window rendering
//16:9 scale
//#define SCALE 60
//4:3 scale
#define SCALE 300 //Scale for window rendering

const int kMapX = 4 * SCALE;
const int kMapY = 3 * SCALE;
const int kMiniMapX = 220;
const int kMiniMapY = 200;

namespace sc2 {
class PathingBot : public Agent {
public:
    virtual void OnGameStart() final;
    virtual void OnStep() final;
    virtual void OnUnitIdle(const Unit* unit) final;
    virtual void OnUnitDestroyed(const Unit* unit);
    virtual void OnGameEnd() final;

private:
    /* ***** Unit Selection Functions ***** */
    /*
        * Selects one random unit from a group of units to be the group leader
        * Returns: a pointer to the leader Unit
    */
    const Unit* SelectLeader(const Units& units, Point2D& goal);

    /* ***** Location Functions ***** */
    //Returns the center point of the playable map space
    Point2D GetMapCenter();


    /* ***** Game Info Functions ***** */
    //Returns the total health of a group of units
    float GetGroupHealth(const Units& units);

    void Render();
    size_t CountUnitType(UNIT_TYPEID unit_type);

};

/* ***** Movement functions ***** */
//Moves units until they are all a certain radius away from a point
bool MoveUnitsNear(Agent *bot, const Units& units, Point2D point, float radius);

//Issue a move command to all given units
bool MoveUnits(Agent *bot, const Units* units, Point2D point);
/*
 * Attempt to move the leader to the next point given by A*
 * Returns: true if the leader moved and we removed a point, false otherwise.
*/
bool PathLeader(Agent* bot, const Unit* leader, std::vector<Point2DI>& path);

/*
* Attempt to move all units to the next point given by A*
* Returns: true if the units moved and we removed a point, false otherwise.
*/
bool PathAll(Agent* bot, const Unit* leader, const Units& units, std::vector<Point2DI>& path);

/*
 * Initialize the A* path
 * Returns: true if the path was successfully initialized, false otherwise
*/
bool InitPath(AStarPathFinder& pathfinder, Point2DI& start, Point2DI& goal, std::vector<Point2DI>& path);


//Returns true if a unit is within a certain distance from a point
bool IsNear(const Unit* unit, Point2D p, float radius);
//Returns true if the leader of the group is near the goal point
bool CheckGoalReached(const Unit* leader, Point2D goal);
//Returns a Point2D as its Point2DI equivalent, rounded down
Point2DI ConvertToPoint2DI(Point2D& p);
//Returns a Point2D as its Point2DI equivalent, rounded down
Point2D ConvertToPoint2D(Point2DI& p);
//Prints the x,y coordinates of a point with formatting
void PrintPoint2D(const Point2D& p);

Point2DI ConvertWorldToMinimap(const GameInfo& game_info, const Point2D& world);
}
