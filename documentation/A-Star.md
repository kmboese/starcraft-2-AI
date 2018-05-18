# A* Pathfinding

## Path planning for Starcraft 2 units

### Summary

The A* search algorithm is used extensively in pathfinding, implementing a 
best-first search by searching among all possible paths and choosing the path 
with the lowest cost. It uses weighted graphs to then select a path that is 
most likely to return the solution, by minimizing the function:
$f(n) = g(n) + h(n)$
In our Starcraft 2 model, the grid that the player moves upon is the basis for 
the graph. The "nodes," represented as $n$ in the function, would be tested 
with costs in their immediate adjacency and with a heuristic for further 
"nodes." The nodes themselves are vertices on the navigation mesh, which is 
accessed in In addition, the sight radius could be used for better accuracy 
instead of simply the immediately adjacent grid space. The essential question 
of this algorithm is the matter of costs and the heuristic function.

### Cost

Marking the costs of certain grid spaces in the map terrain is one of the most 
important aspects of employing the A* algorithm, or any pathfinding algorithm.
```sc2_map_info.h``` contains grids and other values that will be helpful with 
spaces that should be entirely excluded from consideration, blocking units from
going there. ```sc2_interfaces.h``` has some functions that will be similar to 
those checking for the availability of spaces. Spaces containing obstacles or 
units are avoided by traveling units, and this will be the rudimentary system 
for cost analysis. On top of this, units need to be aware of cliffs and spaces 
to avoid at farther distances. A unit cannot traverse cliffs, holes, and other 
obstacles, so the cost of traveling to a tile containing one of these would 
be entirely ignored, as that the unit gains nothing from traveling there, so 
it would never want to go there. It would need to If an adjacent space is free, 
the cost to travel to it is 1, for example, and a space 2 tiles away may have a 
cost of 2. 

### Heuristic

Each unit has a sight radius, which can be found in ```sc2_unit.h```. Using 
what can be seen within the sight radius of the units, an estimation of 
obstacles can be made for distances outside the sight radius. The heuristic 
function will be what determines this approximation. Elements like distance 
from the edge of the map, educated guesses based on surroundings, and 
worst-case-scenario assumptions will be taken into account when creating the 
heuristic function. For example, if a unit sees a cliff in the distance, it may
choose a path further away from it, as it is likely for there to be more of 
them nearby. Additionally, if all that the unit sees are cliffs, it needs to 
find a ramp to continue moving forward. Then the A* algorithm needs to target
moving up and over. The heuristic function may differ from unit to unit, due to 
differences in sight radius. 

### Putting it together

Once the algorithm has the costs and has calculated heuristics for nearby paths,
a path will be selected based on the lowest cost. The cost, coming from terrain 
and obstacles, together with an estimated cost for more distant tiles, will 
select a path. Due to the nature of the estimations, this may not be the "best"
path, but this algorithm also does not need to find the best path. It will find
the one that is best, found the most quickly, instead of traversing every 
possible path and calculating costs of each one. Within a dynamic game, speed
is of the essence and the player benefits more from AI that is "less stupid"
over AI that is "the smartest."

#### Sources
* "Introduction to Search" lecture, McCoy ECS 170
* "A* Search Algorithm" Wikipedia
* [Amit's A* Pages] (http://theory.stanford.edu/~amitp/GameProgramming/) 
* [GameDev.net "How to do starcraft 2 pathfinding?"] (https://www.gamedev.net/forums/topic/648438-how-to-do-starcraft-2-pathfinding/)
* [TeamLiquid "The Mechannics of Starcraft 2"] (http://www.teamliquid.net/forum/starcraft-2/132171-the-mechanics-of-sc2-part-1)
* [James Anhalt's "AI Navigation" presentation] (http://www.gdcvault.com/play/1014514/AI-Navigation-It-s-Not)
