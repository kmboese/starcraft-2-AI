#include "sc2lib/sc2_lib.h"

#include <iostream>
#include <iomanip>
#include <utility> //for pair
#include <stack>   //for stack
using namespace std;

#include "astar.h"

//void GoodbyeCruelWorld() {
//    std::cout << "Goodbye, cruel world!" << std::endl;
//}

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


#define DIST_DIAG 1.41421356237f
#define DIST_HZVT 1.0f

//typedef for pair of integers
typedef pair<int, int> Pair;

//typedef for pair<double, pair<int, int>> type
//'double' is for 'f', where f = g + h
typedef pair<double, pair<int, int>> pPair;

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

struct PositionState
{
    const sc2::GameInfo* mGameInfo;
    unsigned char* mData;  //grid
    int mWidth;     //width
    int mHeight;    //height
    int mSize;      //size
    Pair mSrc;      //source
    Pair mDst;      //destination
    posInfo* mpPosInfo; //position info array

    //open list, as set of pair of pair <f, <row, col>>, with f = g + h
    set<pPair> openList;

    //init position info array
    void InitPosInfo()
    {
        mpPosInfo = new posInfo[mSize];
        for (int i = 0; i < mSize; i++)
        {
            mpPosInfo[i].closed = false;
            mpPosInfo[i].parentX = -1;
            mpPosInfo[i].parentY = -1;
            mpPosInfo[i].f = FLT_MAX;
            mpPosInfo[i].g = FLT_MAX;
            mpPosInfo[i].h = FLT_MAX;
        }
     }

    //exit position info array
    void ExitPosInfo()
    {
        delete[] mpPosInfo;
        mpPosInfo = NULL;
    }

    //find path via A-star search algorithm
    void FindPath(const sc2::GameInfo& game_info, Pair& src, Pair& dst)
    {

        //grid, source and destination
        mGameInfo = &game_info;
        const sc2::ImageData& imd = mGameInfo->pathing_grid;
        mData = (unsigned char*)imd.data.c_str();
        mWidth = imd.width;
        mHeight = imd.height;
        mSize = mWidth * mHeight;
        mSrc = src;
        mDst = dst;

        //src out of range
        if (!IsValid(mSrc.first, mSrc.second))
        {
            PrintError("Path source is invalid");
            return;
        }

        //dst out of range
        if (!IsValid(mDst.first, mDst.second))
        {
            PrintError("Path destination is invalid");
            return;
        }

        //src or dst not available
        if (!IsClear(mSrc.first, mSrc.second) || !IsClear(mDst.first, mDst.second))
        {
            PrintError("Path source or destination not available");
            return;
        }

        //src and dst same
        if (IsDst(mSrc.first, mSrc.second))
        {
            PrintError("Path source or destination are same");
            return;
        }

        //init position info array
        InitPosInfo();

        //put src into open list, with f = g = h = 0
        int x = mSrc.first;
        int y = mSrc.second;
        UpdateOpenList(x, y, 0.0, 0.0, x, y);

        //search path
        while (!openList.empty())
        {
            //update path in all directions
            if (UpdatePath())
            {
                //exit position info array
                ExitPosInfo();
                return;
            }
        }

        //exit position info array
        ExitPosInfo();

        //failed to find path
        PrintError("Failed to find path");
    }

    //update path
    bool UpdatePath()
    {
        //get open position, remove it from open list
        pPair p = *openList.begin();
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

        //not complete
        return false;
    }

    //update direction
    bool UpdateDirection(int x, int y, double cost, int prevX, int prevY)
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

    //complete path
    bool CompletePath(int x, int y, int prevX, int prevY)
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

    //update position
    void UpdatePos(int x, int y, double cost, int prevX, int prevY)
    {
        //update position if not yet closed, and is pathable (not an abstacle)
        int pos = GetGridPos(x, y);
        if (!mpPosInfo[pos].closed && IsClear(x, y))
        {
            //new g and h
            int prevPos = GetGridPos(prevX, prevY);
            double nextG = mpPosInfo[prevPos].g + cost;
            double nextH = CalculateHValue(x, y);

            //update open linst
            UpdateOpenList(x, y, nextG, nextH, prevX, prevY);
        }
    }

    //if not in open list, add to open list
    //otherwise update in open list if new cost is better
    void UpdateOpenList(int x, int y, double nextG, double nextH, int prevX, int prevY)
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

    //calculate 'h' heuristics.
    double CalculateHValue(int row, int col)
    {
        //cost difference (distance) formula
        return ((double)sqrt((row - mDst.first)*(row - mDst.first)
            + (col - mDst.second)*(col - mDst.second)));
    }

    //whether position is in valid range
    bool IsValid(int x, int y)
    {
        if (x < mGameInfo->playable_min.x)
            return false;
        if (x > mGameInfo->playable_max.x)
            return false;
        if (y < mGameInfo->playable_min.y)
            return false;
        if (y > mGameInfo->playable_max.y)
            return false;
        return true;
    }

    //offset inot grid array
    int GetGridPos(int x, int y)
    {
        return x * mWidth + y;
    }

    //whether position is available for use (clear, not blocked)
    bool IsClear(int x, int y)
    {
        int pos = GetGridPos(x, y);
        if (mData[pos] == 0)
            return true; //clear
        return false; //obstacle
    }

    //whether position is the destination
    bool IsDst(int row, int col)
    {
        if (row == mDst.first && col == mDst.second)
            return true; //destination
        return false; //not destination
    }

    //traces resulting path
    void TracePath()
    {
        cout << "The Path is ";
        int x = mDst.first;
        int y = mDst.second;
        int pos = GetGridPos(x, y);

        stack<Pair> Path;

        while (!(mpPosInfo[pos].parentX == x && mpPosInfo[pos].parentY == y))
        {
            Path.push(make_pair(x, y));
            int temp_x = mpPosInfo[pos].parentX;
            int temp_y = mpPosInfo[pos].parentY;
            x = temp_x;
            y = temp_y;
            pos = GetGridPos(x, y);
        }

        Path.push(make_pair(x, y));
        while (!Path.empty())
        {
            pair<int, int> p = Path.top();
            Path.pop();
            cout << "-> (" << p.first << "," << p.second << ") ";
        }
        cout << std::endl;
    }

    //prints error
    void PrintError(const char* msg)
    {
        if (msg)
            cout << msg << endl;
    }
};


void FindBestPathTest3(const sc2::GameInfo& game_info)
{
    std::cout << "Find Best Path Test2" << std::endl;

    //source at left-bottom
    Pair src = make_pair(30, 30);
    //destination at left-top
    Pair dst = make_pair(60, 60);

    //path finder
    PositionState posState;

    //find path
    posState.FindPath(game_info, src, dst);
}
