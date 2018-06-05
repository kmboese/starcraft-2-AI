#include "sc2api/sc2_api.h"

//void GoodbyeCruelWorld();
namespace sc2
{
void PrintBestPath(std::vector<Point2DI>& outPath);

//AStar Path Finder Error Codes 
enum AStarPathError
{
    NoError = 0,
    NoPath,
    SameSrcAndDst,
    NotPlayableSrc,
    NotPlayableDst,
    NotPathableSrc,
    NotPathableDst,
};

//typedef for pair<double, pair<int, int>> type
//'double' is for 'f', where f = g + h
//'pair<int, int>' is the coordinates of the pos the 'f' is for.
typedef std::pair<double, std::pair<int, int>> openPos;

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

//AStar Path Finder
class AStarPathFinder
{
public:

    AStarPathFinder(const GameInfo& game_info, bool canMoveDiag);
    virtual ~AStarPathFinder();

    //find path via A-star search algorithm
    //returns 'true' if path is found, 'false' otherwise
    //if path is found, 'outPath' is filled with all points along that path,
    //including source and destination points;
    //if path is not found, 'outPath' is set to 'empty'.
    bool FindPath(Point2DI& src, Point2DI& dst, std::vector<Point2DI>& outPath);

    //option to allow for diagonal moves
    bool GetCanMoveDiag() const {return mCanMoveDiag;}
    void SetCanMoveDiag(bool canMoveDiag) {mCanMoveDiag = canMoveDiag;}

    //fills the specified point with map dimensions
    //-- some points may not be playable
    //-- and some playable points may by not pathable
    void GetMapDimensions(Point2DI& pt) {pt.x = mWidth; pt.y = mHeight;}

    //fills the specified points with minimum and maximum playing locations
    void GetPlayableRange(Point2DI& ptMin, Point2DI& ptMax)
    {
        ptMin.x = (int)mGameInfo.playable_min.x;
        ptMin.y = (int)mGameInfo.playable_min.y;
        ptMax.x = (int)mGameInfo.playable_max.x;
        ptMax.y = (int)mGameInfo.playable_max.y;
    }

    //whether the point is within map dimensions, regardless of whether playable
    bool IsInMap(const Point2DI& pt)
    {
        if (pt.x < 0 || pt.x >= mWidth) return false;
        if (pt.y < 0 || pt.y >= mHeight) return false;
        return true;
    }

    //whether the point is playable, regardless of whether pathable or not
    bool IsPlayable(const Point2DI& pt) {return IsValid(pt.x, pt.y);}

    //whether the point is pathable - i.e. playable point that is not part of obstacle
    bool IsPathable(const Point2DI& pt) {return (IsValid(pt.x, pt.y) && IsClear(pt.x, pt.y));}

    //gets last error
    AStarPathError GetError() const {return mError;}

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
        if (x < mGameInfo.playable_min.x || x > mGameInfo.playable_max.x) return false;
        if (y < mGameInfo.playable_min.y || y > mGameInfo.playable_max.y) return false;
        return true;
    }

    //offset inot grid array
    int GetGridPos(int x, int y) const {return y * mWidth + x;}

    //whether position is available for use (clear, not blocked)
    bool IsClear(int x, int y) const {int pos = GetGridPos(x, y); return (mData[pos] == 0);}

    //whether position is the destination
    bool IsDst(int x, int y) const {return (x == mDst.x && y == mDst.y);}

    //traces resulting path
    void TracePath();

    //sets last error
    bool SetError(AStarPathError error) {mError = error; return false;}

public:

    //gets error string for specifed error
    static const char* GetErrorString(AStarPathError error);

protected:

    bool mCanMoveDiag;
    AStarPathError mError;         //last error, if any
    const GameInfo& mGameInfo;     //game info
    const sc2::ImageData& mImd;    //map image data;
    unsigned char* mData;          //grid
    int mWidth;                    //width
    int mHeight;                   //height
    int mSize;                     //size
    Point2DI mSrc;                 //source
    Point2DI mDst;                 //destination
    posInfo* mpPosInfo;            //position info array
    std::vector<Point2DI>* mpPath; //path on output
    std::set<openPos> openList;    //open list, set of current 'openPos'
};

} //namespace sc2


#include "sc2api/sc2_unit.h"
void DPS_Print(const char* msg);
void DPS_PrintObservation(const char* msg, const sc2::ObservationInterface* obs);
void DPS_PrintUnit(const char* msg, const sc2::Unit* unit);

