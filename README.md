# Starcraft 2 AI Project: AI Gore
A Starcraft 2 AI bot created with the Blizzard s2Client API, written in C++.

# Overview
Our bot uses a combined implementation of the A* pathing algorithm, flocking algorithm, and influence maps to create an AI that can move along an efficient path to a defined goal point, while avoiding enemies to minimize damage taken along the way. This is an improvement over the default move command in Starcraft 2, which does only basic unit separation and does not avoid enemies. We used the Blizzard s2Client, which is a wrapper for the Starcraft 2 protocol buffer, to facilitate the design of our agent.

# Building and Running the Project
We targeted Windows 7/10 as the build environment for our project, so we recommend building on that platform for the best experience. However, it is possible to compile and run this code on Linux, but the game interface on Linux uses the Blizzard OpenGL development library for rendering, and is very basic. 

 To build the project, you can follow Blizzard's instructions on the s2client-api Github, as our process is the same:
 * [Building on Windows](https://github.com/Blizzard/s2client-api/blob/master/docs/building.md#windows)
 * [Building on Linux](https://github.com/Blizzard/s2client-api/blob/master/docs/building.md#linux)

To run the project on Windows, navigate to `<build_directory>\bin` to find the compiled executables. Our AI executable is titled `project.exe`, while the rest are Blizzard's example executables. Note that in order for the executable to run, you need Starcraft 2 installed on your system. If the system doesn't detect your executable, you can pass the path to the executable as follows: `project.exe -e <path_to_executable>`. Note that the proper executable is not in the root install of the Starcraft 2 directory, but is rather located at `<starcraft_2_install>\Versions\BaseXXXXX\SC2_x64.exe`.

# Examples of the Bot in Action
* [Marines avoiding roaches](https://www.youtube.com)

# CS Concepts Involved
* Algorithms
    * A*: a common path-finding algorithm for agents in games. This is essentially a modified version of Dijkstra's algorithm that considers the distance from the goal as well as the cost to traverse terrain in generating the shortest path for a unit to move to a goal. Inputs are the grid conversion of the game map, and output is a set of grid points as the shortest path.
    * Flocking: a method of keeping units separated from each other using the centroid of a group of units and the distance between units to modify their direction of movement. Uses game data such as unit radius and relative unit positions to modify movement.
* Data Structures
    * Game grid: to speed up computation, we converted the game map, which has granularity down to the pixel layer, into an integer grid, such that a 100x100 map in-game would translate to a 100x100 2D array of points. This allows for A* and influence maps to be calculated at a much lower cost, while still allowing for relatively good granularity of movement.
    * Influence maps: a way to represent the influence units exert over a region of a map. In our case, we used the attack range and position of enemy Roaches as influence on the map that A* would take into consideration in its heuristic. As a result, our marines would avoid the attack radius of enemy units whenever possible. We updated the influence maps at a regular frequency whenever enemy Roaches moved, as their influence is linked to their position on the map.
* Object-Oriented Design
    * The Blizzard API is heavily class and object based. We used concepts like inheritance in the construction of our bot, as well as polymorphism to over-ride behavior on in-game events, such as game start, internal game ticks, and unit deaths.
    * We kept the implementation of our algorithms separate as much as possible, so we could develop and connect the algorithms to the game code independently. This allowed for faster development and meant we did not have to wait for a certain module to be made to make progress on another part of the bot. 

	
# References
1. [Mechanics of Influence Mapping](http://aigamedev.com/open/tutorial/influence-map-mechanics/)
2. [Kiting in RTS Games Using Influence Maps](http://nova.wolfwork.com/papers/Kiting_RTS_Influence_Maps.pdf)
3. [Influence Maps and Objective Functions in Starcraft](https://arxiv.org/pdf/1803.02943.pdf)
4. [Python Testing Tools Taxonomy](https://pythonhosted.org/testing/)
5. [Boids (Flocking)](https://www.red3d.com/cwr/boids/)
6. [Game School Gems Influence Maps](http://gameschoolgems.blogspot.com/2009/12/influence-maps-i.html)
7. [Flocking Tutorial Videos](https://www.phstudios.com/flocking-ai-series/)