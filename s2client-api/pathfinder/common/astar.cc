#include "sc2lib/sc2_lib.h"

#include <iostream>
#include <iomanip>
#include <utility> //for pair
#include <stack>   //for stack
using namespace std;

#include "astar.h"

void DPS_Print(const char* msg)
{
    if (msg)
        cout << msg << endl;
}

void DPS_PrintImageDataValues(const sc2::ImageData& imd)
{
    if (imd.bits_per_pixel != 8)
    {
        cout << "BPS of (" << imd.bits_per_pixel << ") not printable yet" << endl;
        return;
    }
    cout << setfill('0') << uppercase;
    for (int row = 0; row < imd.height; row++)
    {
        cout << dec << setw(4) << row << ": " << hex;
        for (int col = 0; col < imd.width; col++)
        {
            int ix = (row * imd.width) + col;
            unsigned int v = (unsigned int)(unsigned char)imd.data[ix];
            cout << setw(2) << v << ".";
        }
        cout << endl;
    }
    cout << nouppercase << dec << "======================================" << endl;
}

void DPS_PrintImageData(const char* msg, const sc2::ImageData& imd, bool printData)
{
    cout << "Image Data";
    if (msg)
        cout << " (" << msg << ")";
    cout << " :)" << endl;
    cout << "Size: (" << imd.width << ", " << imd.height << ") = " << (imd.width * imd.height) << endl;
    cout << "BPS: " << imd.bits_per_pixel << endl;
    if (printData)
        DPS_PrintImageDataValues(imd);
}

void DPS_PrintGameInfo(const char* msg, const sc2::GameInfo& game_info)
{
    if (msg)
        cout << msg << ": ";
    cout << "GameInfo: " << endl;
    //! Plain text name of a map. Note that this may be different from the filename of the map.
    cout << "Map Name: [" << game_info.map_name << "]" << endl;
    //! Filename of map. Includes the ".SC2Map" file extension.
    cout << "File Name: [" << game_info.local_map_path << "]" << endl;
    //! World width of a map.
    //! World height of a map.
    cout << "World W/H of a map: (" << game_info.width << ", " << game_info.height << ")" << endl;
    //! The minimum coordinates of playable space. Points less than this are not playable.
    cout << "Min playable: (" << game_info.playable_min.x << ", " << game_info.playable_min.y << ")" << endl;
    //! The maximum coordinates of playable space. Points greater than this are not playable.
    cout << "Max playable: (" << game_info.playable_max.x << ", " << game_info.playable_max.y << ")" << endl;
    //! Positions of possible enemy starting locations.
    cout << "Enemy start locs: (" << game_info.enemy_start_locations.size() << ")" << endl;
    for (size_t i = 0; i < game_info.enemy_start_locations.size(); i++)
    {
        cout << "\t" << i << ": (" << game_info.enemy_start_locations[i].x << ", " << game_info.enemy_start_locations[i].y << ")" << endl;
    }
    cout << "Other start locs: (" << game_info.start_locations.size() << ")" << endl;
    for (size_t i = 0; i < game_info.start_locations.size(); i++)
    {
        cout << "\t" << i << ": (" << game_info.start_locations[i].x << ", " << game_info.start_locations[i].y << ")" << endl;
    }
    //! Grid showing which cells are pathable by units.
    DPS_PrintImageData("Pathable Cells", game_info.pathing_grid, true);
    //! Height map of terrain.
    DPS_PrintImageData("Terrain Height", game_info.terrain_height, true);
    //! Grid showing which cells can accept placement of structures.
    DPS_PrintImageData("Placement-Able", game_info.placement_grid, true);
    cout << "-------" << endl;
}

void DPS_PrintObservation(const char* msg, const sc2::ObservationInterface* obs)
{
    if (msg)
        cout << msg << ": ";
    cout << "Observation ...";
    cout << endl;
    const sc2::GameInfo& game_info = obs->GetGameInfo();
    DPS_PrintGameInfo(NULL, game_info);
}

void DPS_PrintUnit(const char* msg, const sc2::Unit* unit)
{
    if (msg)
        cout << msg << ": ";
    cout << "Unit " << unit->tag << " ";
    switch (unit->unit_type.ToType())
    {
    case sc2::UNIT_TYPEID::TERRAN_MARINE: //48
        cout << "TERRAN_MARINE";
        break;
    case sc2::UNIT_TYPEID::ZERG_ROACH: //110
        cout << "ZERG_ROACH";
        break;
    default:
        cout << "Type " << (int)unit->unit_type.ToType();
        break;
    }
    cout << " (" << unit->pos.x << ", " << unit->pos.y << ", " << unit->pos.z << ")";
    cout << endl;
}

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
        //InfluenceMap::getInfMap() returns *copy* of the whole map on very call, *not* the actual map
        //thus, this code will be exceptionally slow until InfluenceMap is made not to use copy constructors
        //in InfluenceMap::getInfMap() call.
        double d = pMap->getInfMap()[x][y];

        //christine says use only >= 0
        if (d >= 0.0)
            d += pMap->getInfMap()[x][y];
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
