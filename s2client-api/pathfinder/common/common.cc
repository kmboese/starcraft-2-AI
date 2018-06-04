#include "common.h"
#include "flocking.h"
#include "astar.h"
#include <iostream>
#include <stdlib.h>

#include "sc2renderer/sc2_renderer.h"

//Scale for window rendering
//16:9 scale
//#define SCALE 60
//4:3 scale
#define SCALE 240
const int kMapX = 4 * SCALE;
const int kMapY = 3 * SCALE;
const int kMiniMapX = 220;
const int kMiniMapY = 200;

const sc2::Unit *leader; //global group leader
float group_health = 0.0; //group health
sc2::Units marines; //group of marines in the simulation
sc2::Units roaches; //group of enemy roaches

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
    //renderer::Initialize("Rendered", 50, 50, kMiniMapX + kMapX, std::max(kMiniMapY, kMapY));

    

//    //Select all marines
//    marines = obs->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MARINE));
//    roaches = obs->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::ZERG_ROACH));
//
//    //Get the initial group health
//    group_health = GetGroupHealth(marines);
//
//    //Move all marines to the center of the map on startup
//    for (const auto &marine : marines) {
//        std::cout << "Marine pos: (" << marine->pos.x << "," << marine->pos.y << ")\n";
//    }
//    //Pick a leader and flock units on initialization
//    leader = SelectLeader(marines);
//    Flock(this, marines, leader, playable_max);
//
//    //Get all marines' locations

//DPS _BEG
//======================================

    //path finder
    //DPS_PrintObservation("OnGBeg", obs);
    AStarPathFinder pathFinder(game_info);

    Point2DI src(30, 30);           //source
    Point2DI dst(60, 60);           //destination
    std::vector<Point2DI> outPath;  //output path

    //find path
    if (pathFinder.FindPath(src, dst, outPath))
    {
        //print output path 
        PrintBestPath(outPath);
    }
    else
    {
        //path not found, error printed to console now
        std::cout << "Failed to find path" << std::endl;
    }

//======================================
//DPS END
}

void PathingBot::OnStep() {
    const ObservationInterface* obs = Observation();
    const GameInfo& game_info = obs->GetGameInfo();
    uint32_t game_loop = obs->GetGameLoop();
    uint32_t pathing_freq = 30; //How often we run our algorithm
    uint32_t info_freq = pathing_freq; //How often we print game info
    uint32_t update_freq = pathing_freq; //How often we update game info
    Point2D playable_max = game_info.playable_max; //maximum playable Point on the map
    Point2D center = GetMapCenter();
    float radius = 2.0; //Radius threshold for IsNear(), 2.0 is a pretty good value

    /*Units marines = obs->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MARINE));
    Units roaches = obs->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::ZERG_ROACH));*/
    //Path units
    if (game_loop % pathing_freq == 0) {
        for (const auto& marine : marines) {
            //Move units to the center if they are not "near" the center, and not the leader
            if (!IsNear(marine, center, radius) && (marine != leader)) {
                Actions()->UnitCommand(marine, ABILITY_ID::MOVE, center);
                //Actions()->UnitCommand(marine, ABILITY_ID::ATTACK_ATTACK, playable_max);
            }
        }
        //Flock(this, marines, leader, playable_max);
    }
    //Update info
    if (game_loop % update_freq == 0) {
        marines = obs->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MARINE));
        group_health = GetGroupHealth(marines);
    }
    //Print Info
    if (game_loop % info_freq == 0) {
        //Print game info
        std::cout << "Centroid location: (" << GetCentroid(marines).x << "," << GetCentroid(marines).y
            << ")\n";
        std::cout << "Group health: " << group_health << "\n";
    }
    
    /*
    //Move all marines to the center of the map
    for (const auto &marine : marines) {
        Point2D center = GetMapCenter();
        Point2D move_location = { game_info.playable_max.x, marine->pos.y };
        //Actions()->UnitCommand(marine, ABILITY_ID::MOVE, game_info.playable_max);
        //Actions()->UnitCommand(marine, ABILITY_ID::MOVE, rand);
        Actions()->UnitCommand(marine, ABILITY_ID::MOVE, move_location);
        std::cout << "Marine pos: (" << marine->pos.x << "," << marine->pos.y << ")\n";
    }
    */

    /*
    //Periodically print game info
    if (gameLoop % 100 == 0) {
        PrintMinerals();
    }
    */

    // moveCamera();
    //TryBuildSupplyDepot();
    //TryBuildBarracks();
    //Render();
}

void PathingBot::OnGameEnd() {
    //Update group health
    group_health = GetGroupHealth(marines);
    //renderer::Shutdown();
    std::cout << "**** Game end info: *****\n";
    std::cout << "Group health: " << group_health << "\n";
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

//DPS _BEG
//======================================
////! In non realtime games this function gets called after each step as indicated by step size.
////! In realtime this function gets called as often as possible after request/responses are received from the game gathering observation state.
//void PathingBot::OnStep()
//{
//    //DPS_Print(__FUNCTION__); //a lot
//}
//
////! Called when a game has ended.
//void PathingBot::OnGameEnd()
//{
//    DPS_Print(__FUNCTION__);
//}
//
//! Called when a unit becomes idle, this will only occur as an event so will only be called when the unit becomes
//! idle and not a second time. Being idle is defined by having orders in the previous step and not currently having
//! orders or if it did not exist in the previous step and now does, a unit being created, for instance, will call both
//! OnUnitCreated and OnUnitIdle if it does not have a rally set.
//!< \param unit The idle unit.
//void PathingBot::OnUnitIdle(const Unit* unit)
//{
//    //only marines were seen
//    //DPS_PrintUnit(__FUNCTION__, unit);
//}

//======================================

////! Called when a game is started after a load. Fast restarting will not call this.
//void PathingBot::OnGameFullStart()
//{
//    DPS_Print(__FUNCTION__);
//}
//
////! Called whenever one of the player's units has been destroyed.
////!< \param unit The destroyed unit.
//void PathingBot::OnUnitDestroyed(const Unit* unit)
//{
//    //only marines were seen
//    //DPS_PrintUnit(__FUNCTION__, unit);
//}
//
////! Called when a Unit has been created by the player.
////!< \param unit The created unit.
//void PathingBot::OnUnitCreated(const Unit* unit)
//{
//    //only marines were seen
//    //DPS_PrintUnit(__FUNCTION__, unit);
//}
//
////! Called when an upgrade is finished, warp gate, ground weapons, baneling speed, etc.
////!< \param upgrade The completed upgrade.
//void PathingBot::OnUpgradeCompleted(UpgradeID id)
//{
//    DPS_Print(__FUNCTION__);
//}
//
////! Called when the unit in the previous step had a build progress less than 1.0 but is greater than or equal to 1.0 in the current step.
////!< \param unit The constructed unit.
//void PathingBot::OnBuildingConstructionComplete(const Unit* unit)
//{
//    DPS_PrintUnit(__FUNCTION__, unit);
//}
//
////! Called when a nydus is placed.
//void PathingBot::OnNydusDetected()
//{
//    DPS_Print(__FUNCTION__);
//}
//
////! Called when a nuclear launch is detected.
//void PathingBot::OnNuclearLaunchDetected()
//{
//    DPS_Print(__FUNCTION__);
//}
//
////! Called when an enemy unit enters vision from out of fog of war.
////!< \param unit The unit entering vision.
//void PathingBot::OnUnitEnterVision(const Unit* unit)
//{
//    //only roaches were seen
//    //DPS_PrintUnit(__FUNCTION__, unit);
//}
//
////! Called for various errors the library can encounter. See ClientError enum for possible errors.
//void PathingBot::OnError(const std::vector<ClientError>& clientErr, const std::vector<std::string>& protoErr)
//{
//    DPS_Print(__FUNCTION__);
//}

//======================================
//DPS END

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

Point2D PathingBot::GetCentroid(const Units& units) {
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

bool PathingBot::IsNear(const Unit* unit, Point2D p, float radius) {
    if (!unit) {
        return false;
    }
    return((abs(unit->pos.x - p.x) < radius) && (abs(unit->pos.y - p.y) < radius));

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