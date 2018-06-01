#include "flocking.h"
#include "common.h"

#include <iostream>
#include <string>

#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2renderer/sc2_renderer.h"

namespace sc2{

bool Flock(Agent *bot, const Units& units, const Unit* leader, const Point2D &move_point) {
    if (units.empty())
        return false;
    bot->Actions()->UnitCommand(leader, ABILITY_ID::MOVE, move_point);
    //Make all other units space themselves out and follow the leader
    for (auto &unit : units) {
        Point3D unit_pos = unit->pos;
        
        if (unit_pos != leader->pos) {
            bot->Actions()->UnitCommand(unit, ABILITY_ID::MOVE, leader->pos);
        }
    }
    return true;
}

bool Separate(Agent *bot, const Units& units) {
    if(units.size() == 0) {
        return false;
    }
    Point2D diff {};
    Point2D move_location {};
    for(const auto& unit : units) {
        diff = GetNeighborsDistance(unit, units);
        move_location.x = unit->pos.x + diff.x;
        move_location.y = unit->pos.y + diff.y;
        bot->Actions()->UnitCommand(unit, ABILITY_ID::MOVE, move_location);
    }
    return true;
}

Point2D GetNeighborsDistance(const Unit* unit, const Units& neighbors) {
    Point2D diff{0.0, 0.0};
    if (neighbors.size() == 0) {
        return diff;
    }
    for (const auto& neighbor : neighbors) {
        //Don't check the unit against itself
        if (neighbor != unit) {
            diff.x += unit->pos.x - neighbor->pos.x;
            diff.y += unit->pos.y - neighbor->pos.y;
        }
    }
    return diff;
}

}