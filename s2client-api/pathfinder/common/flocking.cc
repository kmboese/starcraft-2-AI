#include "flocking.h"
#include "common.h"

#include <iostream>
#include <string>

#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2renderer/sc2_renderer.h"

namespace sc2{

bool Flock(Agent *bot, const Units &units, const Unit *leader, const Point2D &move_point) {
    if (units.empty())
        return false;
    bot->Actions()->UnitCommand(leader, ABILITY_ID::MOVE, move_point);
    //Make all other units move in line with the leader
    for (auto &unit : units) {
        Point3D unit_pos = unit->pos;
        if (unit_pos != leader->pos) {
            bot->Actions()->UnitCommand(unit, ABILITY_ID::MOVE, leader->pos);
        }
    }
    return true;
}

}