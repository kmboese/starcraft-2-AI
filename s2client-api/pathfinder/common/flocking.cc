#include "flocking.h"
#include "common.h"

#include <iostream>
#include <string>

#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2renderer/sc2_renderer.h"

namespace sc2{

bool Flock(Agent *bot, const Units& units, const Point2D &boundary_point) {
    if (units.empty())
        return false;
    //Pick a leader
    unsigned int unit_index = GetRandomInteger(0, units.size() - 1);
    const Unit* leader = units[unit_index];
    bot->Actions()->UnitCommand(leader, ABILITY_ID::MOVE, boundary_point);
    //Make all other units move in line with the leader
    for (auto &unit : units) {
        Point3D unit_pos = unit->pos;
        if (unit_pos != leader->pos) {
            continue;
        }
    }
}

}