# Flocking - Kevin
## Advanced pathing for units in Starcraft 2

#### Definiton: What is flocking?
Flocking is a group pathing algorithm that creates movement vectors for a set of
units based on their proximity to neighboring units, their movement vector, and
neighboring unit movement vectors. It is inspired by actual movement of
organisms in nature such as birds or bacteria, which move as a cohesive unit
and avoid collision with their neighbors. The basic terminology as it applies
to Starcraft 2 is as follows:
* Agent: a single unit
* Velocity vector: a unit's movement speed and direction of movement
* Neighborhood: a radius around a unit to look for other units
* Resultant: The movement vector obtained based on path calculations

Several calculations can be performed based on data from the game API such as
unit location, movement speed, unit size, and neighbor unit movement. There are
three main methods that are used to provide a flocking implementation:

### 1. Alignment
Alignment is a method that makes units in a group face the same direction as
other nearby units. This will result in an alignment vector that will be
applied to all units in a group. In terms of Starcraft 2, we can access units'
direction directly using the ```Unit``` object fields and apply this to a
chosen unit. We can use the ```facing``` attribute from ```sc2_unit.h``` to
get the direction each neighboring unit is facing. Then we can sum up the
movement speeds of units neighboring a given unit, based on the ```radius```
attribute of the unit. It may be necessary to set the neighborhood radius
slightly larger than that of the given unit to prevent it from bumping into
larger units, like a marine running into a siege tank, for instance.

Once all the unit movement speeds and directions have been summed, we divide
this summed vector ```v``` by the number of neighboring units, then normalize
the vector, which we can do using ```Normalize2D()```, defined in
```sc2_common.h```. We will not worry about
z-values for units for now, but such functionality could be added if we want
to have mixed groups of ground and air units in the future.
### 2. Cohesion
To implement cohesion, we can simply collect the x and y positions of all units
in our defined group, sum up their x and y values, and then divide by the total
number of units in the group to find a group centroid value. For all units
```unit```  in our group with position ```pos```, we would have:
```c++
for (auto unit : units) {
    sum_x += unit.pos.x;
    sum_y += unit.pos.y;
    unit_count++;
}
Point2D centroid{};
centroid.x = sum_x / unit_count;
centroid.y = sum_y / unit_count;
normalize(centroid); //normalize the centroid
```

### 3. Separation
The last step in our basic flocking implementation will be separation, which is 
the property that prevents units from colliding with neighboring units. To 
implement this, for each unit ```u``` we will calculate its distance from its 
neighboring units ```n``` and create a vector from this data that will 
essentially "push" ```u``` away from its neighbors as it moves to keep units 
from becoming too tightly packed together. Once again, we can use a pre-defined 
unit radius to determine which units are considered neighbors of ```u```. We can
implement it as follows:
```c++
vector<float> sep_vec{0, 0};
//Unit u already exists
for (auto neighbor : neighbors) {
    sep_vec[0] += neighbor.pos.x - u.pos.x; // x-value
    sep_vec[1] += neighbor.pos.y - u.pos.y; // y-value
}
//Flip the vector's direction to keep a unit *away* from its neighbors
sep_vec[0] *= -1;
sep_vec[0] *= -1; 
```

### Summary
These 3 methods will give us a foundation for a basic flocking implementation 
using the SC2 C++ API. This will guide how our friendly units move in formation, 
while influence maps will guide our units away from enemies, and A* will provide 
the basic routing between 2 points. The combination of all of these techniques 
should give us a group pathing system that is more robust than the default 
pathing implementation given in the standard game, and should allow us to better 
minimize the damage a group of friendly units takes when their path coincides 
with enemy units.

Some potential additions that would make our flocking more advanced:

* Z values: take into account the z-axis as well to allow for flying units to 
flock and to effectively ignore the formation of ground troops below it (since 
air units and ground units do not collide).
* Context-sensitivity: there are many situations in-game where we would want to 
alter the formation of our friendly units based on the enemy units it sees. One 
of the most fundamental examples of this is Terran marines and Zerg banelings: 
an intelligent AI would split a flock of marines/other bio units upon seeing a 
group of banelings close enough to interact with our group of friendly units and 
cause splash damage. There are potentially hundreds of such scenarios in-game, 
and it would be a large amount of work to implement even the highest priority of 
these interactions.

#### References
1. [3 Simple Rules of Flocking Behaviors](https://gamedevelopment.tutsplus.com/tutorials/3-simple-rules-of-flocking-behaviors-alignment-cohesion-and-separation--gamedev-3444)
2. [Boids](https://www.red3d.com/cwr/boids/)
