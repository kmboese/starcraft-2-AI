# Flocking
## Advanced pathing for units in Starcraft 2

#### Definiton: What is flocking?
Flocking is a group pathing algorithm that creates movement vectors for a set
of units based on their proximity to neighboring units, their movement vector,
and neighboring unit movement vectors. It is inspired by actual movement of organisms in nature such as birds or bacteria, which move as a cohesive unit and avoid collision with their neighbors. The basic terminology as it applies to
Starcraft 2 is as follows:
* Agent: a single unit
* Velocity vector: a unit's movement speed and direction of movement
* Neighborhood: a radius around a unit to look for other units
* Resultant: The movement vector obtained based on path calculations

Several calculations can be performed based on data from the game API such as
unit location, movement speed, unit size, and neighbor unit movement. There are
three main methods that are used to provide a flocking implementation:

### 1. Alignment
Alignment is a method that makes units in a group face the same direction as other nearby units. This will result in an alignment vector that will be applied to all units in a group. To calculate this, we need to know how many neighbors are within a unit's neighborhood, and we need to know the direction each neighboring unit is facing. We can use the ```facing``` attribute from ```sc2_unit.h``` to get the direction each neighboring unit is facing. Then we can sum up the movement speeds of units neighboring a given unit, based on the ```radius``` attribute of the unit. It may be necessary to set the neighborhood radius slightly larger than that of the given unit to prevent it from bumping into larger units, like a marine running into a siege tank, for instance.

Once all the unit movement speeds and directions have been summed, we divide this summed vector ```v``` by the number of neighboring units, then normalize the vector, which we can do using ```Normalize2D(v.x)``` and ```Normalize2D(v.y)```, defined in ```sc2_common.h```. We will not worry about z-values for units for now, but such functionality could be added if we want to have mixed groups of ground and air units in the future.

### 2. Cohesion

### 3. Separation
