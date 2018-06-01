#include "astar.h"

#include <iostream>
#include <string>
#include <utility> //for pair
#include <stack>   //for stack

#include "sc2lib/sc2_lib.h"

void GoodbyeCruelWorld() {
    std::cout << "Goodbye, cruel world!" << std::endl;
}

using namespace std;

#define ROW 9
#define COL 10

//typedef for pair of integers
typedef pair<int, int> Pair;

//typedef for pair<double, pair<int, int>> type
//'double' is for 'f', where f = g + h
typedef pair<double, pair<int, int>> pPair;


//position state info
struct posInfo
{
    //parent indices
    int parentRow;
    int parentCol;
    //values, f = g + h
    double f;
    double g;
    double h;
};


//prints error
void PrintError(const char* msg)
{
    if (msg)
        cout << msg << endl;
}

//whether position is in valid range
bool isValid(int row, int col)
{
    return (row >= 0) && (row < ROW) && (col >= 0) && (col < COL);
}


//whether position is available for use (clear, not blocked)
bool isClear(int grid[][COL], int row, int col)
{
    if (grid[row][col] == 1)
        return true; //clear
    return false; //obstacle
}


//whether position is the destination
bool isDst(int row, int col, Pair dst)
{
    if (row == dst.first && col == dst.second)
        return true; //destination
    return false; //not destination
}


//calculate 'h' heuristics.
double calculateHValue(int row, int col, Pair dst)
{
    //cost difference (distance) formula
    return ((double)sqrt((row - dst.first)*(row - dst.first)
        + (col - dst.second)*(col - dst.second)));
}

//traces resulting path
void tracePath(posInfo posData[][COL], Pair dst)
{
    cout << "The Path is ";
    int row = dst.first;
    int col = dst.second;

    stack<Pair> Path;

    while (!(posData[row][col].parentRow == row && posData[row][col].parentCol == col))
    {
        Path.push(make_pair(row, col));
        int temp_row = posData[row][col].parentRow;
        int temp_col = posData[row][col].parentCol;
        row = temp_row;
        col = temp_col;
    }

    Path.push(make_pair(row, col));
    while (!Path.empty())
    {
        pair<int, int> p = Path.top();
        Path.pop();
        cout << "-> (" << p.first << "," << p.second << ") ";
    }
    cout << std::endl;
}

//find best path via A-star search algorithm
void FindBestPath(int graph[][COL], Pair src, Pair dst)
{
    cout << "Find Best Path Run" << endl;

    //src out of range
    if (!isValid(src.first, src.second))
    {
        PrintError("Path source is invalid");
        return;
    }

    //dst out of range
    if (!isValid(dst.first, dst.second))
    {
        PrintError("Path destination is invalid");
        return;
    }

    //src or dst not available
    if (!isClear(graph, src.first, src.second) || !isClear(graph, dst.first, dst.second))
    {
        PrintError("Path source or destination not available");
        return;
    }

    //src and dst same
    if (isDst(src.first, src.second, dst))
    {
        PrintError("Path source or destination are same");
        return;
    }

    //list of processed (closed) positions
    //boolean 2D array, with all positions initially set to false
    bool closedList[ROW][COL];
    memset(closedList, false, sizeof(closedList));

    //2D position info array
    posInfo posData[ROW][COL];

    int i;
    int j;

    //init position info array
    for (i = 0; i<ROW; i++)
    {
        for (j = 0; j<COL; j++)
        {
            posData[i][j].f = FLT_MAX;
            posData[i][j].g = FLT_MAX;
            posData[i][j].h = FLT_MAX;
            posData[i][j].parentRow = -1;
            posData[i][j].parentCol = -1;
        }
    }

    //init info for src position
    i = src.first;
    j = src.second;
    posData[i][j].f = 0.0;
    posData[i][j].g = 0.0;
    posData[i][j].h = 0.0;
    posData[i][j].parentRow = i;
    posData[i][j].parentCol = j;

    //open list, as set of pair of pair <f, <row, col>>, with f = g + h
    set<pPair> openList;

    //put src into open list, with f = 0
    openList.insert(make_pair(0.0, make_pair(i, j)));

    //whether best path found
    bool pathFound = false;

    //search
    while (!openList.empty())
    {
        //get open position, remove it from open list
        pPair p = *openList.begin();
        openList.erase(openList.begin());

        //mark removed position as processed (closed)
        i = p.second.first;
        j = p.second.second;
        closedList[i][j] = true;

        //8 possible successors for the removed position are possible
        //local storage for f, g, h of these successors
        double fNew;
        double gNew;
        double hNew;

        //north, if valid
        if (isValid(i - 1, j))
        {
            //dst reached
            if (isDst(i - 1, j, dst))
            {
                //save parent info
                posData[i - 1][j].parentRow = i;
                posData[i - 1][j].parentCol = j;
                tracePath(posData, dst);
                pathFound = true;
                return;
            }

            //skip if already processed, or if unavailable
            else if (!closedList[i - 1][j] && isClear(graph, i - 1, j))
            {
                //new f, g, h
                gNew = posData[i][j].g + 1.0;
                hNew = calculateHValue(i - 1, j, dst);
                fNew = gNew + hNew;

                //if not in open list, add to open list
                //otherwise update in open list if new cost is better
                if (posData[i - 1][j].f == FLT_MAX || posData[i - 1][j].f > fNew)
                {
                    //insert 
                    openList.insert(make_pair(fNew, make_pair(i - 1, j)));

                    //update info
                    posData[i - 1][j].f = fNew;
                    posData[i - 1][j].g = gNew;
                    posData[i - 1][j].h = hNew;
                    posData[i - 1][j].parentRow = i;
                    posData[i - 1][j].parentCol = j;
                }
            }
        }

        //south, if valid
        if (isValid(i + 1, j))
        {
            //dst reached
            if (isDst(i + 1, j, dst))
            {
                //save parent info
                posData[i + 1][j].parentRow = i;
                posData[i + 1][j].parentCol = j;
                tracePath(posData, dst);
                pathFound = true;
                return;
            }

            //skip if already processed, or if unavailable
            else if (!closedList[i + 1][j] && isClear(graph, i + 1, j))
            {
                //new f, g, h
                gNew = posData[i][j].g + 1.0;
                hNew = calculateHValue(i + 1, j, dst);
                fNew = gNew + hNew;

                //if not in open list, add to open list
                //otherwise update in open list if new cost is better
                if (posData[i + 1][j].f == FLT_MAX || posData[i + 1][j].f > fNew)
                {
                    //insert 
                    openList.insert(make_pair(fNew, make_pair(i + 1, j)));

                    //update info
                    posData[i + 1][j].f = fNew;
                    posData[i + 1][j].g = gNew;
                    posData[i + 1][j].h = hNew;
                    posData[i + 1][j].parentRow = i;
                    posData[i + 1][j].parentCol = j;
                }
            }
        }

        //east, if valid
        if (isValid(i, j + 1))
        {
            //dst reached
            if (isDst(i, j + 1, dst))
            {
                //save parent info
                posData[i][j + 1].parentRow = i;
                posData[i][j + 1].parentCol = j;
                tracePath(posData, dst);
                pathFound = true;
                return;
            }

            //skip if already processed, or if unavailable
            else if (!closedList[i][j + 1] && isClear(graph, i, j + 1))
            {
                //new f, g, h
                gNew = posData[i][j].g + 1.0;
                hNew = calculateHValue(i, j + 1, dst);
                fNew = gNew + hNew;

                //if not in open list, add to open list
                //otherwise update in open list if new cost is better
                if (posData[i][j + 1].f == FLT_MAX || posData[i][j + 1].f > fNew)
                {
                    //insert 
                    openList.insert(make_pair(fNew, make_pair(i, j + 1)));

                    //update info
                    posData[i][j + 1].f = fNew;
                    posData[i][j + 1].g = gNew;
                    posData[i][j + 1].h = hNew;
                    posData[i][j + 1].parentRow = i;
                    posData[i][j + 1].parentCol = j;
                }
            }
        }

        //west, if valid
        if (isValid(i, j - 1))
        {
            //dst reached
            if (isDst(i, j - 1, dst))
            {
                //save parent info
                posData[i][j - 1].parentRow = i;
                posData[i][j - 1].parentCol = j;
                tracePath(posData, dst);
                pathFound = true;
                return;
            }

            //skip if already processed, or if unavailable
            else if (!closedList[i][j - 1] && isClear(graph, i, j - 1))
            {
                //new f, g, h
                gNew = posData[i][j].g + 1.0;
                hNew = calculateHValue(i, j - 1, dst);
                fNew = gNew + hNew;

                //if not in open list, add to open list
                //otherwise update in open list if new cost is better
                if (posData[i][j - 1].f == FLT_MAX || posData[i][j - 1].f > fNew)
                {
                    //insert 
                    openList.insert(make_pair(fNew, make_pair(i, j - 1)));

                    //update info
                    posData[i][j - 1].f = fNew;
                    posData[i][j - 1].g = gNew;
                    posData[i][j - 1].h = hNew;
                    posData[i][j - 1].parentRow = i;
                    posData[i][j - 1].parentCol = j;
                }
            }
        }

        //north-east, if valid
        if (isValid(i - 1, j + 1))
        {
            //dst reached
            if (isDst(i - 1, j + 1, dst))
            {
                //save parent info
                posData[i - 1][j + 1].parentRow = i;
                posData[i - 1][j + 1].parentCol = j;
                tracePath(posData, dst);
                pathFound = true;
                return;
            }

            //skip if already processed, or if unavailable
            else if (!closedList[i - 1][j + 1] && isClear(graph, i - 1, j + 1))
            {
                //new f, g, h
                gNew = posData[i][j].g + 1.414;
                hNew = calculateHValue(i - 1, j + 1, dst);
                fNew = gNew + hNew;

                //if not in open list, add to open list
                //otherwise update in open list if new cost is better
                if (posData[i - 1][j + 1].f == FLT_MAX || posData[i - 1][j + 1].f > fNew)
                {
                    //insert 
                    openList.insert(make_pair(fNew, make_pair(i - 1, j + 1)));

                    //update info
                    posData[i - 1][j + 1].f = fNew;
                    posData[i - 1][j + 1].g = gNew;
                    posData[i - 1][j + 1].h = hNew;
                    posData[i - 1][j + 1].parentRow = i;
                    posData[i - 1][j + 1].parentCol = j;
                }
            }
        }

        //north-west, if valid
        if (isValid(i - 1, j - 1))
        {
            //dst reached
            if (isDst(i - 1, j - 1, dst))
            {
                //save parent info
                posData[i - 1][j - 1].parentRow = i;
                posData[i - 1][j - 1].parentCol = j;
                tracePath(posData, dst);
                pathFound = true;
                return;
            }

            //skip if already processed, or if unavailable
            else if (!closedList[i - 1][j - 1] && isClear(graph, i - 1, j - 1))
            {
                //new f, g, h
                gNew = posData[i][j].g + 1.414;
                hNew = calculateHValue(i - 1, j - 1, dst);
                fNew = gNew + hNew;

                //if not in open list, add to open list
                //otherwise update in open list if new cost is better
                if (posData[i - 1][j - 1].f == FLT_MAX || posData[i - 1][j - 1].f > fNew)
                {
                    //insert 
                    openList.insert(make_pair(fNew, make_pair(i - 1, j - 1)));

                    //update info
                    posData[i - 1][j - 1].f = fNew;
                    posData[i - 1][j - 1].g = gNew;
                    posData[i - 1][j - 1].h = hNew;
                    posData[i - 1][j - 1].parentRow = i;
                    posData[i - 1][j - 1].parentCol = j;
                }
            }
        }

        //south-east, if valid
        if (isValid(i + 1, j + 1))
        {
            //dst reached
            if (isDst(i + 1, j + 1, dst))
            {
                //save parent info
                posData[i + 1][j + 1].parentRow = i;
                posData[i + 1][j + 1].parentCol = j;
                tracePath(posData, dst);
                pathFound = true;
                return;
            }

            //skip if already processed, or if unavailable
            else if (!closedList[i + 1][j + 1] && isClear(graph, i + 1, j + 1))
            {
                //new f, g, h
                gNew = posData[i][j].g + 1.414;
                hNew = calculateHValue(i + 1, j + 1, dst);
                fNew = gNew + hNew;

                //if not in open list, add to open list
                //otherwise update in open list if new cost is better
                if (posData[i + 1][j + 1].f == FLT_MAX || posData[i + 1][j + 1].f > fNew)
                {
                    //insert 
                    openList.insert(make_pair(fNew, make_pair(i + 1, j + 1)));

                    //update info
                    posData[i + 1][j + 1].f = fNew;
                    posData[i + 1][j + 1].g = gNew;
                    posData[i + 1][j + 1].h = hNew;
                    posData[i + 1][j + 1].parentRow = i;
                    posData[i + 1][j + 1].parentCol = j;
                }
            }
        }

        //south-west, if valid
        if (isValid(i + 1, j - 1))
        {
            //dst reached
            if (isDst(i + 1, j - 1, dst))
            {
                //save parent info
                posData[i + 1][j - 1].parentRow = i;
                posData[i + 1][j - 1].parentCol = j;
                tracePath(posData, dst);
                pathFound = true;
                return;
            }

            //skip if already processed, or if unavailable
            else if (!closedList[i + 1][j - 1] && isClear(graph, i + 1, j - 1))
            {
                //new f, g, h
                gNew = posData[i][j].g + 1.414;
                hNew = calculateHValue(i + 1, j - 1, dst);
                fNew = gNew + hNew;

                //if not in open list, add to open list
                //otherwise update in open list if new cost is better
                if (posData[i + 1][j - 1].f == FLT_MAX || posData[i + 1][j - 1].f > fNew)
                {
                    //insert 
                    openList.insert(make_pair(fNew, make_pair(i + 1, j - 1)));

                    //update info
                    posData[i + 1][j - 1].f = fNew;
                    posData[i + 1][j - 1].g = gNew;
                    posData[i + 1][j - 1].h = hNew;
                    posData[i + 1][j - 1].parentRow = i;
                    posData[i + 1][j - 1].parentCol = j;
                }
            }
        }
    }

    //failed to find path
    if (!pathFound)
        PrintError("Failed to find path");
}

//test FindBestPath
void FindBestPathTest()
{
    std::cout << "Find Best Path Test" << std::endl;

    //1 = clear, no obstacle
    //0 = closed, obstacle
    int graph[ROW][COL] =
    {
        { 1, 0, 1, 1, 1, 1, 0, 1, 1, 1 },
        { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1 },
        { 1, 1, 1, 0, 1, 1, 0, 1, 0, 1 },
        { 0, 0, 1, 0, 1, 0, 0, 0, 0, 1 },
        { 1, 1, 1, 0, 1, 1, 1, 0, 1, 0 },
        { 1, 0, 1, 1, 1, 1, 0, 1, 0, 0 },
        { 1, 0, 0, 0, 0, 1, 0, 0, 0, 1 },
        { 1, 0, 1, 1, 1, 1, 0, 1, 1, 1 },
        { 1, 1, 1, 0, 0, 0, 1, 0, 0, 1 }
    };

    //source at left-bottom
    Pair src = make_pair(8, 0);
    //destination at left-top
    Pair dest = make_pair(0, 0);

    //find path
    FindBestPath(graph, src, dest);
}
