#include "sc2lib/sc2_lib.h"

#include <iostream>
#include <iomanip>
#include <utility> //for pair
#include <stack>   //for stack
using namespace std;

#include "astar.h"

//namespace
namespace sc2
{

#define DIST_DIAG 1.41421356237f    //min diagonal distance cost
#define DIST_HZVT 1.0f              //min horz/vert distance cost

AStarPathFinder::AStarPathFinder(const GameInfo& game_info, bool canMoveDiag)
    : mGameInfo(game_info), mImd(game_info.pathing_grid), mCanMoveDiag(canMoveDiag), mError(NoError)
{
    mWidth = mImd.width;
    mHeight = mImd.height;
    mSize = mWidth * mHeight;
    mpPosInfo = new posInfo[mSize];
}

AStarPathFinder::~AStarPathFinder()
{
    delete[] mpPosInfo;
    mpPosInfo = NULL;
}

bool AStarPathFinder::FindPath(Point2DI& src, Point2DI& dst, vector<Point2DI>& outPath)
{
    //input
    mData = (unsigned char*)mImd.data.c_str();
    mSrc = src;
    mDst = dst;

    //output
    mpPath = &outPath;
    mpPath->clear();

    //validate src and dst
    if (!IsValid(mSrc.x, mSrc.y)) return SetError(NotPlayableSrc); //src not playable
    if (!IsValid(mDst.x, mDst.y)) return SetError(NotPlayableDst); //dst not playable
    if (!IsClear(mSrc.x, mSrc.y)) return SetError(NotPathableSrc); //src not pathable
    if (!IsClear(mDst.x, mDst.y)) return SetError(NotPathableDst); //dst not pathable
    if (IsDst(mSrc.x, mSrc.y)) return SetError(SameSrcAndDst); //same src and dst
    SetError(NoError); //no error

    //reset position info array
    for (int i = 0; i < mSize; i++)
    {
        mpPosInfo[i].closed = false;
        mpPosInfo[i].parentX = -1;
        mpPosInfo[i].parentY = -1;
        mpPosInfo[i].f = FLT_MAX;
        mpPosInfo[i].g = FLT_MAX;
        mpPosInfo[i].h = FLT_MAX;
    }

    //put src into open list, with f = g = h = 0
    UpdateOpenList(mSrc.x, mSrc.y, 0.0, 0.0, mSrc.x, mSrc.y);

    //search path
    while (!openList.empty())
    {
        //update path in all directions
        if (UpdatePath())
        {
            //path found
            return true;
        }
    }

    //failed to find path
    return SetError(NoPath);
}

bool AStarPathFinder::UpdatePath()
{
    //get open position, remove it from open list
    openPos p = *openList.begin();
    openList.erase(openList.begin());

    //current position
    int x = p.second.first;
    int y = p.second.second;

    //mark removed position as processed (closed)
    int pos = GetGridPos(x, y);
    mpPosInfo[pos].closed = true;

    //8 possible successors for the removed position are possible

    //update north direction
    if (UpdateDirection(x - 1, y, DIST_HZVT, x, y))
        return true;

    //update south direction
    if (UpdateDirection(x + 1, y, DIST_HZVT, x, y))
        return true;

    //update east direction
    if (UpdateDirection(x, y + 1, DIST_HZVT, x, y))
        return true;

    //update west direction
    if (UpdateDirection(x, y - 1, DIST_HZVT, x, y))
        return true;

    //if diag move allowed
    if (mCanMoveDiag)
    {
        //update north-east direction
        if (UpdateDirection(x - 1, y + 1, DIST_DIAG, x, y))
            return true;

        //update north-west direction
        if (UpdateDirection(x - 1, y - 1, DIST_DIAG, x, y))
            return true;

        //update south-east direction
        if (UpdateDirection(x + 1, y + 1, DIST_DIAG, x, y))
            return true;

        //update south-west direction
        if (UpdateDirection(x + 1, y - 1, DIST_DIAG, x, y))
            return true;
    }

    //not complete
    return false;
}

bool AStarPathFinder::UpdateDirection(int x, int y, double cost, int prevX, int prevY)
{
    //position in range
    if (IsValid(x, y))
    {
        //done if path complete
        if (CompletePath(x, y, prevX, prevY))
            return true;

        //update position
        UpdatePos(x, y, cost, prevX, prevY);
    }
    return false; //not completed
}

bool AStarPathFinder::CompletePath(int x, int y, int prevX, int prevY)
{
    //dst not reached
    if (!IsDst(x, y))
        return false;

    //dst reached - save parent info and trace the path
    int pos = GetGridPos(x, y);
    mpPosInfo[pos].parentX = prevX;
    mpPosInfo[pos].parentY = prevY;
    TracePath();
    return true;
}

void AStarPathFinder::UpdatePos(int x, int y, double cost, int prevX, int prevY)
{
    //update position if not yet closed, and is pathable (not an abstacle)
    int pos = GetGridPos(x, y);
    if (!mpPosInfo[pos].closed && IsClear(x, y))
    {
        //new g and h
        int prevPos = GetGridPos(prevX, prevY);
        double totalInfluence = GetInfluence(x, y);
        cost += totalInfluence;
        double nextG = mpPosInfo[prevPos].g + cost;
        double nextH = CalcHeuristic(x, y);

        //update open linst
        UpdateOpenList(x, y, nextG, nextH, prevX, prevY);
    }
}

void AStarPathFinder::UpdateOpenList(int x, int y, double nextG, double nextH, int prevX, int prevY)
{
    //new f = g + h
    double nextF = nextG + nextH;

    //if not in open list, add to open list
    //otherwise update in open list if new cost is better
    int pos = GetGridPos(x, y);
    if (mpPosInfo[pos].f == FLT_MAX || mpPosInfo[pos].f > nextF)
    {
        //insert into open list
        openList.insert(make_pair(nextF, make_pair(x, y)));

        //update info
        mpPosInfo[pos].f = nextF;
        mpPosInfo[pos].g = nextG;
        mpPosInfo[pos].h = nextH;
        mpPosInfo[pos].parentX = prevX;
        mpPosInfo[pos].parentY = prevY;
    }
}

double AStarPathFinder::CalcHeuristic(int x, int y)
{
    //cost difference (distance) formula
    int diffX = x - mDst.x;
    int diffY = y - mDst.y;
    return (double)sqrt(diffX * diffX + diffY * diffY);
}

void AStarPathFinder::TracePath()
{
    stack<Point2DI> path;

    int x = mDst.x;
    int y = mDst.y;
    int pos = GetGridPos(x, y);

    while (!(mpPosInfo[pos].parentX == x && mpPosInfo[pos].parentY == y))
    {
        path.push(Point2DI(x, y));
        int temp_x = mpPosInfo[pos].parentX;
        int temp_y = mpPosInfo[pos].parentY;
        x = temp_x;
        y = temp_y;
        pos = GetGridPos(x, y);
    }
    path.push(Point2DI(x, y));

    while (!path.empty())
    {
        Point2DI pt = path.top();
        path.pop();
        mpPath->push_back(pt);
    }
}

//gets error string for specifed error
const char* AStarPathFinder::GetErrorString(AStarPathError error)
{
    switch (error)
    {
    case NoError:        return "No Error";
    case NoPath:         return "No Path Found";
    case SameSrcAndDst:  return "Same Source and Destination";
    case NotPlayableSrc: return "Source Not Playable";
    case NotPlayableDst: return "Destination Not Playable";
    case NotPathableSrc: return "Source Not Pathable";
    case NotPathableDst: return "Destination Not Pathable";
    case InvalidInfMapWidth: return "Invalid Influence Map Width"; 
    case InvalidInfMapHeight: return "Invalid Influence Map Height";
    }
    return "Undefined Error";
}

double AStarPathFinder::GetInfluence(int x, int y)
{
    double inflTotal = 0.0;
    for (vector<InfluenceMap*>::iterator it = mInfMapList.begin(); it != mInfMapList.end(); ++it)
    {
        InfluenceMap* pMap = *it;
        double d = pMap->getInfMap()[y][x];

        //Use only >= 0
        if (d >= 0.0)
            inflTotal += d;
    }
    return inflTotal;
}

//adds influence map
bool AStarPathFinder::AddInfluenceMap(InfluenceMap* pInfMap)
{
    //no map specified
    if (!pInfMap)
        return false;

    //width of influence map does not match width of pathable map
    if (pInfMap->getNumCols() != mWidth)
        return SetError(InvalidInfMapWidth);

    //height of influence map does not match height of pathable map
    if (pInfMap->getNumRows() != mHeight)
        return SetError(InvalidInfMapHeight);

    mInfMapList.push_back(pInfMap);
    return true;
}

//prints best path
void PrintBestPath(vector<Point2DI>& outPath)
{
    cout << "The path: ";
    if (outPath.empty())
    {
        cout << "EMPTY";
    }
    else
    {
        for (vector<Point2DI>::iterator it = outPath.begin(); it != outPath.end(); ++it)
        {
            Point2DI& pt = *it;
            cout << "-> (" << pt.x << "," << pt.y << ") ";
        }
    }
    cout << endl;
}

} //namespace sc2
