#include "common.h"
#include "flocking.h"
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

const sc2::Unit *leader;

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

    //Select all marines
    Units marines = obs->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MARINE));
    //Move all marines to the center of the map on startup
    for (const auto &marine : marines) {
        //Point2D rand = {center.x + GetRandomInteger(1,10), center.y + GetRandomInteger(1,10)};
        //Actions()->UnitCommand(marine, ABILITY_ID::MOVE, game_info.playable_max);
        Actions()->UnitCommand(marine, ABILITY_ID::MOVE, GetMapCenter());
        std::cout << "Marine pos: (" << marine->pos.x << "," << marine->pos.y << ")\n";
    }
    //Pick a leader and flock units on initialization
    leader = SelectLeader(marines);
    //Flock(this, marines, leader, GetMapCenter());

    //Get all marines' locations
}

void PathingBot::OnStep() {
    const ObservationInterface* obs = Observation();
    const GameInfo& game_info = obs->GetGameInfo();
    uint32_t game_loop = obs->GetGameLoop();
    uint32_t loop_frequency = 30; //How often we run our algorithm
    Point2D playable_max = game_info.playable_max; //maximum playable Point on the map
    Point2D center = GetMapCenter();
    float radius = 2.0; //Radius threshold for IsNear(), 2.0 is a pretty good value

    //Select all marines
    Units marines = obs->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MARINE));
    if (game_loop % loop_frequency == 0) {
        for (const auto& marine : marines) {
            //Move units to the center if they are not "near" the center
            if (!IsNear(marine, center, radius)) {
                Actions()->UnitCommand(marine, ABILITY_ID::MOVE, center);
            }
        }
        
        //Flock(this, marines, leader, GetMapCenter());
        //Print centroid location
        std::cout << "Centroid location: (" << GetCentroid(marines).x << "," << GetCentroid(marines).y
            << ")\n";
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
    renderer::Shutdown();
}

void PathingBot::OnUnitIdle(const Unit* unit) {
    switch (unit->unit_type.ToType()) {
        case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
            if (CountUnitType(UNIT_TYPEID::TERRAN_SCV) < OPTIMAL_SCV_COUNT) {
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_SCV);
                std::cout << "Trained SCV\n";
            }
            break;
        }
        case UNIT_TYPEID::TERRAN_SCV: {
            const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
            if (!mineral_target) {
                break;
            }
            Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
            break;
        }
        //Train marines in groups (wait until we have a pool of resources to begin building)
        case UNIT_TYPEID::TERRAN_BARRACKS: {
            Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARINE);
            break;
        }
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
    leader = units[unsigned int(GetRandomInteger(0, units.size() - 1))];
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

/*Attempt to build a given structure
 * Note: default argument can only be given in header declaration
*/
bool PathingBot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type) {
    const ObservationInterface* observation = Observation();
    int unit_count = 0; //How many units are chosen to build structures

                        // If a unit already is building a supply structure of this type, do nothing.
                        // Also get an scv to build the structure.
    const Unit* unit_to_build = nullptr;
    Units units = observation->GetUnits(Unit::Alliance::Self);
    for (const auto& unit : units) {
        //Select SCV
        if (unit->unit_type == unit_type) {
            unit_to_build = unit;
            //if an SCV is currently building this structure, don't build it
            for (const auto& order : unit->orders) {
                if (order.ability_id == ability_type_for_structure) {
                    return false;
                }
            }
            break;
        }
    }

    float rx = GetRandomScalar();
    float ry = GetRandomScalar();

    //Don't build buildings near minerals
    float mineral_threshold = 5.0f;
    const Unit* nearest_minerals = FindNearestMineralPatch(unit_to_build->pos);
    Point2D build_location{ unit_to_build->pos.x + rx * BUILD_RADIUS, unit_to_build->pos.y + ry * BUILD_RADIUS };
    while (DistanceSquared2D(build_location, nearest_minerals->pos) < mineral_threshold) {
        rx = GetRandomScalar();
        ry = GetRandomScalar();
        build_location.x = unit_to_build->pos.x + rx * BUILD_RADIUS;
        build_location.y = unit_to_build->pos.y + ry * BUILD_RADIUS;
        nearest_minerals = FindNearestMineralPatch(unit_to_build->pos);
    }

    Actions()->UnitCommand(unit_to_build,
        ability_type_for_structure,
        Point2D(unit_to_build->pos.x + rx * BUILD_RADIUS, unit_to_build->pos.y + ry * BUILD_RADIUS));
    return true;
}

bool PathingBot::TryBuildSupplyDepot() {
    const ObservationInterface* observation = Observation();

    // If we are not supply capped, don't build a supply depot.
    if (observation->GetFoodUsed() <= observation->GetFoodCap() - SUPPLY_BUFFER)
        return false;

    // Try and build a depot. Find a random SCV and give it the order.
    return TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT);
}

const Unit* PathingBot::FindNearestMineralPatch(const Point2D& start) {
    Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
    float distance = std::numeric_limits<float>::max();
    const Unit* target = nullptr;
    for (const auto& u : units) {
        if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
            float d = DistanceSquared2D(u->pos, start);
            if (d < distance) {
                distance = d;
                target = u;
            }
        }
    }
    return target;
}

Units* PathingBot::SelectMarines() {
    //Get all friendly units
    Units units = Observation()->GetUnits(Unit::Alliance::Ally);
    //vector to hold all selected marines
    Units* marines{};
    //Select all marines from our friendly units
    for (const auto& u : units) {
        if (u->unit_type == UNIT_TYPEID::TERRAN_MARINE) {
            marines->push_back(u);
        }
    }
    return marines;
}

bool PathingBot::TryBuildBarracks() {
    const ObservationInterface* observation = Observation();

    if (CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1) {
        return false;
    }

    if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) > OPTIMAL_BARRACKS_COUNT) {
        return false;
    }

    return TryBuildStructure(ABILITY_ID::BUILD_BARRACKS);
}

void PathingBot::PrintMinerals() {
    std::cout << "Minerals: " << Observation()->GetMinerals() << std::endl;
}

//Returns the number of a given type of unit/building
size_t PathingBot::CountUnitType(UNIT_TYPEID unit_type) {
    return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}

/* ***** NOT WORKING/NOT USED FUNCTIONS***** */
//Not working
void PathingBot::MoveCamera() {
    // const GameInfo& game_info = Observation()->GetGameInfo();

    // Point2D cameraLocation = Observation()->GetCameraPos();
    // std::cout << "Camera location: " << cameraLocation.x << " " << cameraLocation.y << std::endl;
    // Point2D newLocation(cameraLocation.x + 20, cameraLocation.y + 20);
    // // Point2D target = Observation()->GetStartLocation() + Point2D(3.0f, 3.0f);
    // Point2DI targetMinimap = ConvertWorldToMinimap(game_info, newLocation);
    // std::cout << "New Camera location: " << targetMinimap.x << " " << targetMinimap.y << std::endl;

    // ActionsFeatureLayer()->CameraMove(targetMinimap);
}

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