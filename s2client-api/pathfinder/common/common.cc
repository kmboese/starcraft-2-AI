#include "common.h"
#include "flocking.h"
#include <iostream>
#include <stdlib.h>

#include "sc2renderer/sc2_renderer.h"

const sc2::Unit *leader; //global group leader
float group_health = 0.0; //group health
sc2::Units marines; //group of marines in the simulation
sc2::Units roaches; //group of enemy roaches

//Conditions
bool centered = false; //indicates group of marines initially was centered on the map
bool separated = false; //indicates the group has been separated out

//Original
//using namespace sc2;
namespace sc2 {
Point2DI ConvertWorldToMinimap(const GameInfo& game_info, const Point2D& world) {
    int image_width = game_info.options.feature_layer.minimap_resolution_x;
    int image_height = game_info.options.feature_layer.minimap_resolution_y;
    float map_width = (float)game_info.width;
    float map_height = (float)game_info.height;
    std::cout << "Map " << game_info.map_name << std::endl;
    std::cout << "Width " << game_info.options.feature_layer.minimap_resolution_x << std::endl;
    std::cout << "Height: " << game_info.options.feature_layer.minimap_resolution_y << std::endl;


    // Pixels always cover a square amount of world space. The scale is determined
    // by the largest axis of the map.
    float pixel_size = std::max(map_width / image_width, map_height / image_height);

    // Origin of world space is bottom left. Origin of image space is top left.
    // Upper left corner of the map corresponds to the upper left corner of the upper
    // left pixel of the feature layer.
    float image_origin_x = 0;
    float image_origin_y = map_height;
    float image_relative_x = world.x - image_origin_x;
    float image_relative_y = image_origin_y - world.y;

    int image_x = static_cast<int>((image_relative_x / pixel_size));
    int image_y = static_cast<int>((image_relative_y / pixel_size));

    return Point2DI(image_x, image_y);
}

void PathingBot::OnGameStart() {
    const ObservationInterface* obs = Observation();
    const GameInfo& game_info = obs->GetGameInfo();
    uint32_t gameLoop = obs->GetGameLoop();
    Point2D center = GetMapCenter();
    Point2D playable_max = game_info.playable_max;

    //Render on Linux
#if defined(__linux__)
    renderer::Initialize("Rendered", 50, 50, kMiniMapX + kMapX, std::max(kMiniMapY, kMapY));
#endif

    //Select all marines
    marines = obs->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MARINE));
    roaches = obs->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::ZERG_ROACH));

    //Get the initial group health
    group_health = GetGroupHealth(marines);

    //Move all marines to the center of the map on startup
    for (const auto &marine : marines) {
        std::cout << "Marine pos: (" << marine->pos.x << "," << marine->pos.y << ")\n";
    }
    //Pick a leader and flock units on initialization
    leader = SelectLeader(marines);
    Flock(this, marines, leader, center);
}

void PathingBot::OnStep() {
    const ObservationInterface* obs = Observation();
    const GameInfo& game_info = obs->GetGameInfo();
    uint32_t game_loop = obs->GetGameLoop();
    uint32_t pathing_freq = 10; //How often we run our algorithm
    uint32_t info_freq = 30; //How often we print game info
    uint32_t update_freq = 30; //How often we update game info
    uint32_t sep_freq = pathing_freq / 3; //how often we spread out the marines
    Point2D playable_max = game_info.playable_max; //maximum playable Point on the map
    Point2D center = GetMapCenter();
    float radius = 2.0; //Radius threshold for IsNear(), 2.0 is a pretty good value

    //Update Info
    marines = obs->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MARINE));

    //Path units
    if (game_loop % pathing_freq == 0) {
        bool unit_was_centered = false; //indicates any marine moved to the center
        //Center the marines
        if (!centered) {
            centered = MoveUnits(this, marines, center);
        }
        //Next, Separate the marines
        else if (!separated) {
            separated = Separate(this, marines);
        }
        //After separation, move units as a group
        else{
            Flock(this, marines, leader, playable_max);
            Separate(this, marines);
        }
    }
    //Keep the marines regularly separated out, once centered
    if (game_loop % sep_freq == 0 && centered) {
        Separate(this, marines);
    }
    //Update info
    if (game_loop % update_freq == 0) {
        group_health = GetGroupHealth(marines);
    }
    //Print Info
    if (game_loop % info_freq == 0) {
        //Print game info
        std::cout << "Centroid location: (" << GetCentroid(marines).x << "," << GetCentroid(marines).y
            << ")\n";
        std::cout << "Group health: " << group_health << "\n";
    }
    //Linux options
#if defined(__linux__)
    const SC2APIProtocol::Observation* raw_obs = Observation()->GetRawObservation();
    const SC2APIProtocol::ObservationRender& render =  raw_obs->render_data();

    const SC2APIProtocol::ImageData& minimap = render.minimap();
    sc2::renderer::ImageRGB(&minimap.data().data()[0], minimap.size().x(), minimap.size().y(), 0, std::max(kMiniMapY, kMapY) - kMiniMapY);

    const SC2APIProtocol::ImageData& map = render.map();
    sc2::renderer::ImageRGB(&map.data().data()[0], map.size().x(), map.size().y(), kMiniMapX, 0);

    sc2::renderer::Render();
#endif
}

void PathingBot::OnGameEnd() {
    //Update group health
    group_health = GetGroupHealth(marines);
    std::cout << "**** Game end info: *****\n";
    std::cout << "Group health: " << group_health << "\n";

#if defined(__linux__)
    renderer::Shutdown();
#endif
}

void PathingBot::OnUnitIdle(const Unit* unit) {
    switch (unit->unit_type.ToType()) {
        case UNIT_TYPEID::TERRAN_MARINE: {
            const GameInfo& game_info = Observation()->GetGameInfo();
            //Actions()->UnitCommand(unit, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations.front());
            break;
        }
        default: {
            break;
        }
    }
}

const Unit* PathingBot::SelectLeader(const Units& units) {
    if (units.size() == 0) {
        return nullptr;
    }
    const Unit *leader = nullptr;
    leader = units[(GetRandomInteger(0, int(units.size()) - 1))];
    return leader;
}

Point2D PathingBot::GetMapCenter() {
    const GameInfo& game_info = Observation()->GetGameInfo();
    Point2D center{};
    center.x = game_info.playable_max.x / 2;
    center.y = game_info.playable_max.y / 2;
    return center;
}

float PathingBot::GetGroupHealth(const Units& units) {
    if (units.size() == 0) {
        return 0.0;
    }
    float health = 0.0;
    for (const auto& unit : units) {
        health += unit->health;
    }
    return health;
}

//Returns the number of a given type of unit/building
size_t PathingBot::CountUnitType(UNIT_TYPEID unit_type) {
    return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}

/* ***** Common Functions ****/
bool MoveUnits(Agent *bot, const Units& units, Point2D point) {
    bool all_near = true; //indicates all units are within a radius of the given point
    for (const auto& unit : units) {
        //Move any units not near the center to the center
        if (!IsNear(unit, point, CENTER_RADIUS)) {
            bot->Actions()->UnitCommand(unit, ABILITY_ID::MOVE, point);
            //Actions()->UnitCommand(marine, ABILITY_ID::ATTACK_ATTACK, playable_max);
            all_near = false;
        }
    }
    return all_near;
}

Point2D GetCentroid(const Units& units) {
    Point2D centroid{0.0, 0.0};
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

bool IsNear(const Unit* unit, Point2D p, float radius) {
    if (!unit) {
        return false;
    }
    return((abs(unit->pos.x - p.x) < radius) && (abs(unit->pos.y - p.y) < radius));
}

/* ***** NOT WORKING/UNUSED FUNCTIONS***** */
//Not used
void PathingBot::Render() {
    const SC2APIProtocol::Observation* observation = Observation()->GetRawObservation();
    const SC2APIProtocol::ObservationRender& render = observation->render_data();

    const SC2APIProtocol::ImageData& minimap = render.minimap();
    renderer::ImageRGB(&minimap.data().data()[0], minimap.size().x(), minimap.size().y(), 0, std::max(kMiniMapY, kMapY) - kMiniMapY);

    const SC2APIProtocol::ImageData& map = render.map();
    renderer::ImageRGB(&map.data().data()[0], map.size().x(), map.size().y(), kMiniMapX, 0);

    renderer::Render();
}

}
