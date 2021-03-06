# Influence Maps

## Introduction

Influence maps give an agent a sense of spatial awareness of its environment. In
terms of the StarCraft II environment, an influence map can give our bot an
awareness of the positions of other characters in the game.  With influence
maps, we can give our agent advantages such as identifying the location of
enemies and determining safe locations that our agent can migrate to if it
chooses. As stated in our proposal, our agent will have areas in the game
environment that it needs to avoid in order to survive, such as enemy units with
a range of fire and areas under enemy control.We will use influence maps in
conjunction with an A* path finding algorithm to help our agent move towards its
destination.

Influence maps in general have the ability to give our AI three different types
of information: a summary of which agents control certain territories,
statistics on the previous state of an area (but only for states a short period
ago), and information that could help make future predictions of what an enemy’s
next move would be and how it would change its influence. For this project, we
will be focusing on giving our AI a summary of its surroundings.

## Implementation

### Overall Layout

Influence maps are typically divided into sections; in our implementation, we
will be using a grid where we assign influence values to each cell. These values
can mean “danger” for enemies, or strength for our bot. We will use a small
64x64 grid based on the visual map of what the agent can see. This map will be
updated every couple of seconds. Moreover, we will have multiple maps, one for
our agent and other maps for the enemy. We can also implement aerial maps for
future extensions.

### Influence Values and Initialization

Influence maps require influence sources. For our AI, our agent and each
enemy are influence sources. (We will ignore events such as grenade explosions
as it complicates the system.) Each source will have a center point where the
influence source is stationed. The influence will decay the further away the
cell is from the center point, therefore an influence source will have its
influence radiate out to a certain distance. We will propagate these influence
values over time.

At the beginning of our system, each cell in an influence map will be
initialized to some absurd number (e.g. negative 9999). Once a unit is created
in the game, its surrounding cells will be initialized to zeros. The radius of a
unit will be determined on the unit itself. For example, enemy units' radii will
be based on their attack radius. Allies of our units will have positive
influence values, with the minimum value being 0 and the maximum value being 1
(the location of the agent). Enemies of our units will have negative influence
values, with the minimum being 0 and the maximum being negative 1.

Influence sources can also layer on top of other influences (i.e. influences are
additive). For our system, enemies will have additive influences with respect to
each other, and our AI bot’s units will have additive influences with respect to
each other. For any given influence map, when agents collide, their values are
added to create an even stronger influence. Therefore agents closer together
will have a stronger influence than agents further apart. Thus, units that
overlap can have value greater than 1. As an example, if agent A has a cell with
value 0.4 and agent B has a cell with value of 0.9, then the new value of that
cell is 1.3.

### Propagation: Updating Influence Values

Updating influence values is based on three parameters: momentum, decay, and
update frequency. Momentum is dependent on how much the updated value is biased
towards the existing value versus the new value. In the chapter by Dave Mark and
the article on the mechanics of influence mapping, linear interpolation is used
to blend the existing value with the new value. The blended value is controlled
by a momentum constant between 0.0 and 1.0. A higher momentum value is biased
towards the existing value, whereas a lower value is biased towards the new
value. We can change this constant during the actual implementation process and
determine which factor creates better results.

The second parameter, decay, influences how quickly an influence should decay
with distance. In the "Mechanics of Influence Mapping" article, decay is
determined by an exponential function. The final parameter, update frequency, is
how often we will update the influence maps in frames per second.

To propagate the influences, we will iterate through each cell in the map,
getting each connection that the cell is connected to, and creating a new
influence value based on this formula (the exponential function mentioned
above):

```C++
calcInf = inf[cell.neighbor] * exponentialFunc(-cell.distance * decay);
inf = max(calcInf, maxInf);
inf[cell] = lerp(inf[cell], inf, momentum);
```

As noted earlier, the influence of the cell (inf[cell]) will have a blurred
value using the linear interpolation of the previous cell's value, the
calculated influence value, and a momentum constant.

## Sources

* [The Mechanics of Influence Mapping][1]
* [Modular Tactical Influence Maps by Dave Mark][2]
* [Game School Gems][3]

[1]: http://aigamedev.com/open/tutorial/influence-map-mechanics/
[2]:http://www.gameaipro.com/GameAIPro2/GameAIPro2_Chapter30_Modular_Tactical_Influence_Maps.pdf
[3]: http://gameschoolgems.blogspot.com/2009/12/influence-maps-i.html