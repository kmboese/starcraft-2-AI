#include "sc2api/sc2_api.h"
#include <iostream>

#include "sc2renderer/sc2_renderer.h"


#define LINUX_USE_SOFTWARE_RENDER 0
// How many workers to have built at all times
#define OPTIMAL_SCV_COUNT 14
#define OPTIMAL_BARRACKS_COUNT 6
// How close we can get to our supply cap before building more supply depots
#define SUPPLY_BUFFER 6
//How far away from their position SCV's will build structures
#define BUILD_RADIUS 15.0f

namespace sc2 {
class PathingBot : public Agent {
public:
    virtual void OnGameStart() final;
    virtual void OnStep() final;
    virtual void OnUnitIdle(const Unit* unit) final;
    virtual void OnGameEnd() final;

private:
    // Currently doesn't work
    void MoveCamera();
    //Output the current number of minerals the player has
    void PrintMinerals();
    void Render();
    size_t CountUnitType(UNIT_TYPEID unit_type);

    bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::TERRAN_SCV);
    bool TryBuildSupplyDepot();
    const Unit* FindNearestMineralPatch(const Point2D& start);
    Units* SelectMarines();
    bool TryBuildBarracks();
};
Point2DI ConvertWorldToMinimap(const GameInfo& game_info, const Point2D& world);
}