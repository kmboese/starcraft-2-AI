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
bool Flock(Agent *bot, const Units &units, const Unit *leader, const Point2D &move_point);

}