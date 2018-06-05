#include "common.h"
#include "flocking.h"
#include "astar.h"
#include <iostream>
#include <stdlib.h>

#include "sc2renderer/sc2_renderer.h"

namespace sc2 {

const Unit* leader; //global group leader
float group_health = 0.0; //group health
bool group_damaged = false; //indicates where the group took damage during the step
Units marines; //group of marines in the simulation
Units roaches; //group of enemy roaches
Point2D goal; //Goal point for A*
Point2DI int_goal; //Goal as an integer x,y location
std::vector<Point2DI> outPath;  //A* shortest path point vector
Point2D next_point; //Next point to which the leader will move

//Bot Checkpoints
bool centered = false; //indicates group of marines initially was centered on the map
bool separated = false; //indicates the group has been separated out
bool goal_reached = false; //indicates the group of units has reached its goal.

void PathingBot::OnGameStart() {
    const ObservationInterface* obs = Observation();
    const GameInfo& game_info = obs->GetGameInfo();
    uint32_t gameLoop = obs->GetGameLoop(); //keep track of the game loop for update frequencies
    Point2D center = GetMapCenter();
    Point2D playable_max = game_info.playable_max;
    //intialize the goal at the beginning of the game (arbitrarily chosen)
    goal = playable_max;
    goal.x -= MAP_BOUNDS_BUFFER;
    goal.y -= MAP_BOUNDS_BUFFER;
    int_goal = ConvertToPoint2DI(goal);

    //Render on Linux
#if defined(__linux__)
    //renderer::Initialize("Rendered", 50, 50, kMiniMapX + kMapX, std::max(kMiniMapY, kMapY));
    renderer::Initialize("Feature layers", 50, 50, 2 * kDrawSize, 2 * kDrawSize);
#endif

    //Select all marines
    marines = obs->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MARINE));
    roaches = obs->GetUnits(Unit::Alliance::Enemy, IsUnit(UNIT_TYPEID::ZERG_ROACH));
    //Get the initial group health
    group_health = GetGroupHealth(marines);
    //Move all marines to the center of the map on startup
    for (const auto &marine : marines) {
        //std::cout << "Marine pos: (" << marine->pos.x << "," << marine->pos.y << ")\n";
    }
    //Pick a leader and flock units on initialization
    leader = SelectLeader(marines);
    Flock(this, marines, leader, center);

    //Initialize path finding
    //DPS _BEG
    //======================================
    //path finder
    //DPS_PrintObservation("OnGBeg", obs);
    AStarPathFinder pathFinder(game_info, true);

    Point2D leader_pos = leader->pos;
    Point2DI int_leader_pos = ConvertToPoint2DI(leader_pos);
    std::cout << "\tDEBUG: int_goal == " << "(" << int_goal.x << ", " << int_goal.y << ")\n";
    std::cout << "\tDEBUG: int_leader_pos == " << "(" << int_leader_pos.x << ", " << int_leader_pos.y << ")\n";


    //find path
    if (pathFinder.FindPath(int_leader_pos, int_goal, outPath))
    {
        //print output path 
        PrintBestPath(outPath);
    }
    else
    {
        //path not found, error printed to console now
        std::cout << "Failed to find path" << std::endl;
    }
    //Initialize the first point to move to
    next_point = ConvertToPoint2D(outPath.front());
    outPath.pop_back();
    //======================================

    //DPS END
}

void PathingBot::OnStep() {
    const ObservationInterface* obs = Observation();
    const GameInfo& game_info = obs->GetGameInfo();
    uint32_t game_loop = obs->GetGameLoop();
    uint32_t pathing_freq = 10; //How often we run our algorithm
    uint32_t info_freq = 10; //How often we print game info
    uint32_t update_freq = 10; //How often we update game info
    uint32_t sep_freq = pathing_freq / 1; //how often we spread out the marines
    Point2D center = GetMapCenter();

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
        else if (!goal_reached)  {
            //If leader is near the next point, pop the next point from the path and move the leader to it
            if (IsNear(leader, next_point, TILE_RADIUS)) {
                next_point = ConvertToPoint2D(outPath.front());
                outPath.pop_back();
                Actions()->UnitCommand(leader, ABILITY_ID::MOVE, next_point);
            }
            else {
                Actions()->UnitCommand(leader, ABILITY_ID::MOVE, next_point);
                std::cout << "Position of leader == " << "(" << leader->pos.x << " ," << leader->pos.y << ")\n";
            }
            //Flock(this, marines, leader, goal);
            goal_reached = CheckGoalReached(leader, goal);
        } 
        //End the game after the goal is reached
        else {
            Separate(this, marines);
            std::cout << "Position of leader == " << "(" << leader->pos.x << " ," << leader->pos.y << ")\n";
        }
    }
    //Keep the marines regularly separated out, once centered
    if ((game_loop % sep_freq == 0) && separated && (!goal_reached) ) {
        //std::cout << "\t\tSeparation loop entered!" << std::endl;
        //Flock(this, marines, leader, goal);
        //Separate(this, marines);
        //MoveFromCentroid(this, marines);
    }
    //Update info
    if (game_loop % update_freq == 0) {
        float old_group_health = group_health; 
        group_health = GetGroupHealth(marines);
        group_damaged = (group_health != old_group_health);
    }
    //Print Info
    if (game_loop % info_freq == 0) {
        //Print game info
        /*
        std::cout << "Centroid location: (" << GetCentroid(marines).x << "," << GetCentroid(marines).y
            << ")\n";
        */
        if (group_damaged) {
            std::cout << "Group health: " << group_health << "\n";
        }
    }
    //Linux options
#if defined(__linux__)
    //Render all layers
    Render();
#endif
}

void PathingBot::OnUnitDestroyed(const Unit* unit) {
    //Update list of marines (may be unnecessary)
    marines = Observation()->GetUnits(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_MARINE));
    //Choose and path a new leader if the leader is killed
    if (unit == leader) {
        std::cout << "\t***** Event: new leader chosen! *****" << std::endl;
        leader = SelectLeader(marines);
        Flock(this, marines, leader, goal);
        //Separate(this, marines);
    }
    else {
        //std::cout << "\tEvent: non-leader marine died" << std::endl;
    }
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

void PathingBot::OnGameEnd() {
    //Update group health
    group_health = GetGroupHealth(marines);
    std::cout << "**** Game end info: *****\n";
    std::cout << "Group health: " << group_health << "\n";

#if defined(__linux__)
    renderer::Shutdown();
#endif
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
        if (!IsNear(unit, point, POINT_RADIUS)) {
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

bool CheckGoalReached(const Unit* leader, Point2D goal) {
    if (!leader) {
        return false;
    }
    //std::cout << "\tDEBUG: leader is " << Distance2D(leader->pos, goal) << " from the goal\n";
    return(IsNear(leader, goal, GOAL_RADIUS));
}

Point2DI ConvertToPoint2DI(Point2D& p) {
    return (Point2DI{ int(p.x), int(p.y) });
}

Point2D ConvertToPoint2D(Point2DI& p) {
    return (Point2D{ float(p.x), float(p.y) });
}

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

/* ***** NOT WORKING/UNUSED FUNCTIONS***** */
//Linux rendering

//Helper rendering functions
void DrawFeatureLayer1BPP(const SC2APIProtocol::ImageData& image_data, int off_x, int off_y) {
    //assert(image_data.bits_per_pixel() == 1);
    int width = image_data.size().x();
    int height = image_data.size().y();
    sc2::renderer::Matrix1BPP(image_data.data().c_str(), width, height, off_x, off_y, kPixelDrawSize, kPixelDrawSize);
}

void DrawFeatureLayerUnits8BPP(const SC2APIProtocol::ImageData& image_data, int off_x, int off_y) {
    //assert(image_data.bits_per_pixel() == 8);
    int width = image_data.size().x();
    int height = image_data.size().y();
    sc2::renderer::Matrix8BPPPlayers(image_data.data().c_str(), width, height, off_x, off_y, kPixelDrawSize, kPixelDrawSize);
}

void DrawFeatureLayerHeightMap8BPP(const SC2APIProtocol::ImageData& image_data, int off_x, int off_y) {
    //assert(image_data.bits_per_pixel() == 8);
    int width = image_data.size().x();
    int height = image_data.size().y();
    sc2::renderer::Matrix8BPPHeightMap(image_data.data().c_str(), width, height, off_x, off_y, kPixelDrawSize, kPixelDrawSize);
}

//Main rendering function
void PathingBot::Render() {
    const SC2APIProtocol::Observation* observation = Observation()->GetRawObservation();
    const SC2APIProtocol::ObservationRender& render = observation->render_data();

    //Render layer
    const SC2APIProtocol::ImageData& minimap = render.minimap();
    renderer::ImageRGB(&minimap.data().data()[0], minimap.size().x(), minimap.size().y(), 0, std::max(kMiniMapY, kMapY) - kMiniMapY);

    const SC2APIProtocol::ImageData& map = render.map();
    renderer::ImageRGB(&map.data().data()[0], map.size().x(), map.size().y(), kMiniMapX, 0);

    ///Feature layer
    const SC2APIProtocol::FeatureLayers& m = observation->feature_layer_data().renders();
    DrawFeatureLayerUnits8BPP(m.unit_density(), 0, 0);
    DrawFeatureLayer1BPP(m.selected(), kDrawSize, 0);

    const SC2APIProtocol::FeatureLayersMinimap& mi = observation->feature_layer_data().minimap_renders();
    DrawFeatureLayerHeightMap8BPP(mi.height_map(), 0, kDrawSize);
    DrawFeatureLayer1BPP(mi.camera(), kDrawSize, kDrawSize);

    renderer::Render();
}

}
