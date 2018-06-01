#include "sc2api/sc2_api.h"
#include <iostream>

#include "sc2renderer/sc2_renderer.h"

#define LINUX_USE_SOFTWARE_RENDER 0
// How many workers to have built at all times
#define OPTIMAL_SCV_COUNT 14
#define OPTIMAL_BARRACKS_COUNT 6
// How close we can get to our supply cap before building more supply depots
#define SUPPLY_BUFFER 6
//How far away from their position SCV's will build structures
#define BUILD_RADIUS 15.0f
//Radius distance at which units are considered "near" to a point
#define UNIT_RADIUS 2.0
//Radius within which units are considered near
#define POINT_RADIUS 3.0

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
    virtual void OnGameEnd() final;

private:
    /* ***** Unit Selection Functions ***** */
    /*
        * Selects one random unit from a group of units to be the group leader
        * Returns: a pointer to the leader Unit
    */
    const Unit* SelectLeader(const Units& units);

    /* ***** Location Functions ***** */
    //Returns the center point of the playable map space
    Point2D GetMapCenter();


    /* ***** Game Info Functions ***** */
    //Returns the total health of a group of units
    float GetGroupHealth(const Units& units);

    void Render();
    size_t CountUnitType(UNIT_TYPEID unit_type);

};

//Movement functions
//Moves all given units to the center of the map
bool MoveUnits(Agent *bot, const Units& units, Point2D point);
//Returns the center point of a group of units
Point2D GetCentroid(const Units& units);
//Returns true if a unit is within a certain distance from a point
bool IsNear(const Unit* unit, Point2D p, float radius);
//Returns true if the leader of the group is near the goal point
bool CheckGoalReached(const Unit* leader, Point2D goal);

Point2DI ConvertWorldToMinimap(const GameInfo& game_info, const Point2D& world);
}
