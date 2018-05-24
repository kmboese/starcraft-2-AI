#include "sc2api/sc2_api.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2renderer/sc2_renderer.h"

#include "common.h"

#include <iostream>
// #include <conio.h>

#define LINUX_USE_SOFTWARE_RENDER 0
// How many workers to have built at all times
#define OPTIMAL_SCV_COUNT 14
#define OPTIMAL_BARRACKS_COUNT 6
// How close we can get to our supply cap before building more supply depots
#define SUPPLY_BUFFER 6
//How far away from their position SCV's will build structures
#define BUILD_RADIUS 7.5f
//Scale for window rendering
//16:9 scale
//#define SCALE 60
//4:3 scale
#define SCALE 240
using namespace sc2;

const int kMapX = 4*SCALE;
const int kMapY = 3*SCALE;
const int kMiniMapX = 220;
const int kMiniMapY = 200;

class RenderAgent : public Agent {
public:
    virtual void OnGameStart() final {
        renderer::Initialize("Rendered", 50, 50, kMiniMapX + kMapX, std::max(kMiniMapY, kMapY));
    }

    virtual void OnStep() final {
        uint32_t gameLoop = Observation()->GetGameLoop();

        //Periodically print game info
        if (gameLoop % 100 == 0) {
            PrintMinerals();
        }

        // moveCamera();
        TryBuildSupplyDepot();
        TryBuildBarracks();
        Render();
    }

    virtual void OnGameEnd() final {
        renderer::Shutdown();
    }

    virtual void OnUnitIdle(const Unit* unit) final {
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
            case UNIT_TYPEID::TERRAN_BARRACKS: {
                Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARINE);
                break;
            }
            case UNIT_TYPEID::TERRAN_MARINE: {
                const GameInfo& game_info = Observation()->GetGameInfo();
                Actions()->UnitCommand(unit, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations.front());
                break;
            }
            default: {
                break;
            }
        }
    }

private:
    // Currently doesn't work
    void moveCamera() {
        // const GameInfo& game_info = Observation()->GetGameInfo();

        // Point2D cameraLocation = Observation()->GetCameraPos();
        // std::cout << "Camera location: " << cameraLocation.x << " " << cameraLocation.y << std::endl;
        // Point2D newLocation(cameraLocation.x + 20, cameraLocation.y + 20);
        // // Point2D target = Observation()->GetStartLocation() + Point2D(3.0f, 3.0f);
        // Point2DI targetMinimap = ConvertWorldToMinimap(game_info, newLocation);
        // std::cout << "New Camera location: " << targetMinimap.x << " " << targetMinimap.y << std::endl;

        // ActionsFeatureLayer()->CameraMove(targetMinimap);
    }

    void PrintMinerals() {
        std::cout << "Minerals: " << Observation()->GetMinerals() << std::endl;
    }

    void Render() {
        const SC2APIProtocol::Observation* observation = Observation()->GetRawObservation();
        const SC2APIProtocol::ObservationRender& render =  observation->render_data();

        const SC2APIProtocol::ImageData& minimap = render.minimap();
        renderer::ImageRGB(&minimap.data().data()[0], minimap.size().x(), minimap.size().y(), 0, std::max(kMiniMapY, kMapY) - kMiniMapY);

        const SC2APIProtocol::ImageData& map = render.map();
        renderer::ImageRGB(&map.data().data()[0], map.size().x(), map.size().y(), kMiniMapX, 0);

        renderer::Render();
    }

    size_t CountUnitType(UNIT_TYPEID unit_type) {
        return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
    }

    bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::TERRAN_SCV) {
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
                //Skip SCV if it is already building the structure
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
        float mineral_threshold = 3.0f;
        const Unit* nearest_minerals = FindNearestMineralPatch(unit_to_build->pos);
        Point2D build_location {unit_to_build->pos.x + rx*BUILD_RADIUS, unit_to_build->pos.y + ry*BUILD_RADIUS};
        while (DistanceSquared2D(build_location, nearest_minerals->pos) < mineral_threshold) {
            rx = GetRandomScalar();
            ry = GetRandomScalar();
            build_location.x = unit_to_build->pos.x + rx*BUILD_RADIUS;
            build_location.y = unit_to_build->pos.y + ry*BUILD_RADIUS;
            nearest_minerals = FindNearestMineralPatch(unit_to_build->pos);
        }

        Actions()->UnitCommand(unit_to_build,
            ability_type_for_structure,
            Point2D(unit_to_build->pos.x + rx*BUILD_RADIUS, unit_to_build->pos.y + ry*BUILD_RADIUS));

        //Print what building we constructed
        //std::cout << "Built a " <<  << std::endl;
        return true;
    }

    bool TryBuildSupplyDepot() {
        const ObservationInterface* observation = Observation();

        // If we are not supply capped, don't build a supply depot.
        if (observation->GetFoodUsed() <= observation->GetFoodCap() - SUPPLY_BUFFER)
            return false;

        // Try and build a depot. Find a random SCV and give it the order.
        return TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT);
    }

    const Unit* FindNearestMineralPatch(const Point2D& start) {
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

    bool TryBuildBarracks() {
        const ObservationInterface* observation = Observation();

        if (CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1) {
            return false;
        }

        if (CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS) > OPTIMAL_BARRACKS_COUNT) {
            return false;
        }

        return TryBuildStructure(ABILITY_ID::BUILD_BARRACKS);
    }
};

int main(int argc, char* argv[]) {
    Coordinator coordinator;
    if (!coordinator.LoadSettings(argc, argv)) {
        std::cout << "Unable to find or parse settings." << std::endl;
        return 1;
    }

    RenderSettings settings(kMapX, kMapY, kMiniMapX, kMiniMapY);
    coordinator.SetRender(settings);
    // coordinator.SetRealtime(false);

#if defined(__linux__)
#if LINUX_USE_SOFTWARE_RENDER
    coordinator.AddCommandLine("-osmesapath /usr/lib/x86_64-linux-gnu/libOSMesa.so");
#else
    coordinator.AddCommandLine("-eglpath /usr/lib/nvidia-384/libEGL.so");
#endif
#endif

    RenderAgent bot;

    coordinator.SetParticipants({
        CreateParticipant(Race::Terran, &bot),
        CreateComputer(Race::Zerg)
    });


    // Start the game.
    coordinator.LaunchStarcraft();
    coordinator.StartGame(kMapBelShirVestigeLE);

    while (coordinator.Update()) {
        if (PollKeyPress()) {
            break;
        }
    }

    return 0;
}
