#include "sc2api/sc2_api.h"

//void GoodbyeCruelWorld();
namespace sc2
{
void PrintBestPath(std::vector<Point2DI>& outPath);

#define DIST_DIAG 1.41421356237f
#define DIST_HZVT 1.0f

//typedef for pair of integers
typedef std::pair<int, int> Pair;

//typedef for pair<double, pair<int, int>> type
//'double' is for 'f', where f = g + h
typedef std::pair<double, std::pair<int, int>> pPair;

//position state info
struct posInfo
{
    //whether closed (processed) position
    bool closed;
    //parent position
    int parentX;
    int parentY;
    //values, f = g + h
    double f;
    double g;
    double h;
};

class AStarPathFinder
{
public:

    AStarPathFinder(const GameInfo& game_info);
    virtual ~AStarPathFinder();

    //find path via A-star search algorithm
    bool FindPath(Point2DI& src, Point2DI& dst, std::vector<Point2DI>& outPath);

protected:

    //update path
    bool UpdatePath();

    //update direction
    bool UpdateDirection(int x, int y, double cost, int prevX, int prevY);

    //complete path
    bool CompletePath(int x, int y, int prevX, int prevY);

    //update position
    void UpdatePos(int x, int y, double cost, int prevX, int prevY);

    //if not in open list, add to open list
    //otherwise update in open list if new cost is better
    void UpdateOpenList(int x, int y, double nextG, double nextH, int prevX, int prevY);

    //calculate 'h' heuristics.
    double CalcHeuristic(int x, int y);

    //whether position is in valid range
    bool IsValid(int x, int y) const
    {
        if (x < mGameInfo.playable_min.x || x > mGameInfo.playable_max.x)
            return false;
        if (y < mGameInfo.playable_min.y || y > mGameInfo.playable_max.y)
            return false;
        return true;
    }

    //offset inot grid array
    int GetGridPos(int x, int y) const
    {
        return x * mWidth + y;
    }

    //whether position is available for use (clear, not blocked)
    bool IsClear(int x, int y) const
    {
        int pos = GetGridPos(x, y);
        return (mData[pos] == 0);
    }

    //whether position is the destination
    bool IsDst(int x, int y) const
    {
        return (x == mDst.first && y == mDst.second);
    }

    //traces resulting path
    void TracePath();

    //prints error
    void PrintError(const char* msg);

protected:

    const GameInfo& mGameInfo;     //game info
    const sc2::ImageData& mImd;    //map image data;
    unsigned char* mData;          //grid
    int mWidth;                    //width
    int mHeight;                   //height
    int mSize;                     //size
    Pair mSrc;                     //source
    Pair mDst;                     //destination
    posInfo* mpPosInfo;            //position info array
    std::vector<Point2DI>* mpPath; //path on output

                                   //open list, as set of pair of pair <f, <row, col>>, with f = g + h
    std::set<pPair> openList;
};

} //namespace sc2


#include "sc2api/sc2_unit.h"
void DPS_Print(const char* msg);
void DPS_PrintObservation(const char* msg, const sc2::ObservationInterface* obs);
void DPS_PrintUnit(const char* msg, const sc2::Unit* unit);

