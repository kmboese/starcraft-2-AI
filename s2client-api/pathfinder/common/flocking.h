#include "sc2api/sc2_api.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2renderer/sc2_renderer.h"

namespace sc2 {

//Flocks a given group of units
bool Flock(Agent *bot, const Units &units, const Point2D &boundary_point);

}