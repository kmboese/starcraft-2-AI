#include "sc2api/sc2_api.h"

//void GoodbyeCruelWorld();
namespace sc2
{
    void FindBestPathTest(const GameInfo& game_info, Point2DI& src, Point2DI& dst, std::vector<Point2DI>& outPath);
    void PrintBestPath(std::vector<Point2DI>& outPath);

} //namespace sc2


#include "sc2api/sc2_unit.h"
void DPS_Print(const char* msg);
void DPS_PrintObservation(const char* msg, const sc2::ObservationInterface* obs);
void DPS_PrintUnit(const char* msg, const sc2::Unit* unit);

