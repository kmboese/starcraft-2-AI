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
bool Flock(Agent *bot, const Units& units, const Unit *leader, Point2D &move_point);

/*
 * Separates a group of units from each other, using minimum relative proximity as a goal
 * Arguments:
    * bot: bot agent to get access to game observations
    * units: group of units to separate
 * Returns: true if the separation succeeded, false otherwise
*/
bool Separate(Agent *bot, const Units& units);
//Moves a group of units away from their center point
bool MoveFromCentroid(Agent* bot, const Units& units);
//Moves a group of units away from each other
bool MoveFromNeighbors(Agent* bot, const Units& units);
//Return the offset point from all neighboring units that are within a certain radius of the target unit
Point2D GetNeighborsOffset(const Unit* unit, const Units& neighbors, float radius);
//Returns the center point of a group of units
Point2D GetCentroid(const Units& units);
//Returns the distance of a unit from the centroid of a group
Point2D GetCentroidOffset(const Unit* unit, const Units& units);

}
