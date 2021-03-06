#include "flocking.h"
#include "common.h"

#include <iostream>
#include <string>
#include <stdlib.h>

#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_lib.h"
#include "sc2renderer/sc2_renderer.h"

namespace sc2 {

bool Flock(Agent *bot, const Units& units, const Unit* leader, Point2D &move_point) {
    if (units.empty())
        return false;
    //Leader action
    bot->Actions()->UnitCommand(leader, ABILITY_ID::MOVE, move_point);
    //Make all other units space themselves out and follow the leader
    for (auto &unit : units) {
        if (unit != leader) {
            //Scale the movement vector to get better separation
            //NOTE: still messing with the multiplier
            float mult = 1.0f;
            //float mult = 1.0f*Distance2D(unit->pos, move_point);
            Point2D adjustment = GetNeighborsOffset(unit, units, unit->radius);
            //std::cout << "\tDEBUG: distance from unit to others is " << sqrt(std::pow(adjustment.x,2) + std::pow(adjustment.y, 2)) << "\n";
            //float mult = 1 / (sqrt(std::pow(adjustment.x, 2) + std::pow(adjustment.y, 2)));
            move_point.x += mult * adjustment.x;
            move_point.y += mult * adjustment.y;
            bot->Actions()->UnitCommand(unit, ABILITY_ID::MOVE, move_point);
        }
    }
    return true;
}

bool Separate(Agent *bot, const Units& units) {
    bool separated = true; //indicates the units have finished separating
    if (units.size() == 0) {
        return false;
    }
    /*
    if (!MoveFromCentroid(bot, units)) {
    separated = false;
    }
    */
    if (!MoveFromNeighbors(bot, units)) {
        separated = false;
    }
    return separated;
}

bool MoveFromCentroid(Agent* bot, const Units& units) {
    bool separated = true;
    float mult = 2.0; //Multiplier for movement of units away from centroid
    if (units.size() == 0) {
        return false;
    }
    Point2D centroid = GetCentroid(units);
    Point2D diff{ 0.0, 0.0 };
    Point2D move_location{ 0.0, 0.0 };
    for (const auto& unit : units) {
        diff.x = unit->pos.x - centroid.x;
        diff.y = unit->pos.y - centroid.y;
        Normalize2D(diff);
        //Move marines away if they are within a certain radius of the centroid
        if (Distance2D(unit->pos, centroid) < CENTROID_RADIUS) {
            //std::cout << "DEBUG: Distance between units and centroid is " << Distance2D(unit->pos, centroid) << "\n";
            move_location.x = unit->pos.x + mult * diff.x;
            move_location.y = unit->pos.y + mult * diff.y;
            bot->Actions()->UnitCommand(unit, ABILITY_ID::MOVE, move_location);
            //We moved a unit, so we have not finished yet
            separated = false;
        }
    }
    return separated;
}

bool MoveFromNeighbors(Agent* bot, const Units& units) {
    if (units.size() == 0) {
        return false;
    }
    //Minimum and maximum values for movement multiplier note: (15,20) are decent values
    /*
    int lower_bound = 10;
    int upper_bound = 20;
    */
    int lower_bound = int(units.size() - sqrt(units.size()));
    int upper_bound = int(units.size());
    bool separated = true; //keeps track of whether units are grouped properly
    float mult = float((units[0]->radius)*GetRandomInteger(lower_bound, upper_bound)); //Multiplier for movement of units away from each other

    for (const auto& unit : units) {
        Point2D dist{};
        dist = GetNeighborsOffset(unit, units, unit->radius);
        //If unit is withing range of its neighbors, move it away from them
        if (Distance2D(dist, Point2D{ 0.0, 0.0 }) > 0.001f) {
            Point2D move_location{};
            move_location.x = unit->pos.x + mult * dist.x;
            move_location.y = unit->pos.y + mult * dist.y;
            bot->Actions()->UnitCommand(unit, ABILITY_ID::MOVE, move_location);
            separated = false;
        }
    }
    return separated;
}

Point2D GetNeighborsOffset(const Unit* unit, const Units& neighbors, float radius) {
    radius *= SEPARATION_MULT; //units further than this distance away are not considered neighbors
    Point2D diff{ 0.0, 0.0 };
    if (neighbors.size() == 0) {
        return diff;
    }
    //Sum total distance 
    for (const auto& neighbor : neighbors) {
        //Don't check the unit against itself
        if ((neighbor != unit) && (Distance2D(unit->pos, neighbor->pos) < radius)) {
            diff.x += unit->pos.x - neighbor->pos.x;
            diff.y += unit->pos.y - neighbor->pos.y;
        }
    }
    diff /= float(neighbors.size());
    return diff;
}

Point2D GetCentroid(const Units& units) {
    Point2D centroid{ 0.0, 0.0 };
    if (units.size() == 0) {
        return centroid;
    }
    //Sum up all unit x and y positions
    for (const auto &unit : units) {
        centroid.x += unit->pos.x;
        centroid.y += unit->pos.y;
    }
    //Divide centroid location by total number of units
    centroid.x /= units.size();
    centroid.y /= units.size();
    return centroid;
}

Point2D GetCentroidOffset(const Unit* unit, const Units& units) {
    Point2D offset{ 0.0, 0.0 };
    if (units.size() == 0) {
        return offset;
    }
    return (unit->pos - GetCentroid(units));
}

}