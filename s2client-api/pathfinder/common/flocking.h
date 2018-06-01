#include "sc2api/sc2_api.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2renderer/sc2_renderer.h"


namespace sc2 {

/*
 * Runs the flocking algorithm on a group of units.
 * Arguments:
    * bot: pointer to the bot agent to get access to game observations
    * units: Group of units on which to run the flocking algorithm
    * leader: The lead unit of the group
    * move_point: The point to which the leader moves initially
 * Returns: true if the algorithm succeeds, false otherwise.
*/
bool Flock(Agent *bot, const Units& units, const Unit *leader, const Point2D &move_point);

/* 
 * Separates a group of units from each other
 * Arguments: 
    * bot: bot agent to get access to game observations
    * units: group of units to separate
 * Returns: true if the separation succeeded, false otherwise
*/
bool Separate(Agent *bot, const Units& units);
//Moves all given units to the center of the map
bool CenterUnits(Agent *bot, const Units& units, Point2D center);
//Returns true if unit is within a certain radius of a point
bool IsNear(const Unit* unit, Point2D p, float radius);

Point2D GetNeighborsDistance(const Unit* unit, const Units& neighbors);
Point2D GetCentroid(const Units& units);
}