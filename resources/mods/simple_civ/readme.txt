[Main]

It's simplified version of Civilization.
Features: units move along the roads instantly,
the creation and destruction of buildings occurs immediately.

At the moment, there are no AI opponents, hostilities, or an ultimate goal.
However, you can set this goal yourself. For example:
* Mastering the entire map,
* Unlocking all technologies (possibly, up to turn #100).

At first, however, you need to carefully monitor the balance of resources,
and pay special attention to the extraction of stone.
Further, one should not forget about the discovery of technology and
the construction of educational institutions.

Perhaps the first few games will be lost due to the process of mastering the game - this is normal, just try again.
Good luck!


[Keyboard]

Map moving - WASD (+CTRL to moving to the end).
Zoom changing: + и -.
Unit selection on current cell: left and right arrows.
The context menu can be called up with the menu key (next to the right CTRL),
but not only from click to unit on right panel.
Select items of the menu: up and down arrows.
Next step: Shift+Enter.

Hovering over most of the buttons reveals a button on the keyboard that does the same.
Typically, keyboard controls are faster and more comfortable than mouse controls.


[Buildings]

Buildings can be divided into simple / mining and general buildings.
General buildings can be built on any squares.

The first group includes:
  Farm - built on the "food" field,
  Sawmill - on the "wood" field,
  Career - on the fields "stone", "coal" and "metal".
These buildings can be built ONLY on their own fields, and
they extract the amount of resources that are on their cell.
Therefore, it is foolish to build a sawmill
in the cell "forest (10)" if there is a "forest (19)".

Each storage must be connected to the "main" (first) storage for its work.
Each building, except for "districts", requires a storage nearby at least the same level as itself.

Workers from districts also can work only near the house.
The maximum number of workers working in a building is equal to the level of that building.
Building performance depending on the number of workers (2, 3, 4)
with respect to 1 worker: ${building_powers}.


[Prices]

Level 2 buildings require the use of cement.
3rd level - steel.
And on the 4th - also in large numbers.

${building_cost}


[Support]

[{color=0x00FF00}{outlinecolor=0}Storage{/outlinecolor}{/color}] and [{color=0x00FF00}{outlinecolor=0}District{/outlinecolor}{/color}] consume base content multiplied by their level.
The rest of the buildings instead of the level use the number of residents currently working.
For example, if 3 workers are sent to the sawmill, but there is only a level 2 storage nearby,
then only 2 workers will work, and the base maintenance of the building will be multiplied by 2, not 3.

Basic building support:

${support_cost}


[Production]

Base production:

${building_production}


[Units]

Workers are needed to carry out daily work.
They move into districts after their construction.
Each sitizen give +1 to science.
Each worker - also +1 (and take 1 food unit).
Each worker of College - also +4.

Builders are needed for the construction of roads and buildings.
They can train a new builder in district.
Builders don't require separate resources for their basic upkeep.

Soldiers are needed to protect existing and conquer new territories.
Training of new soldiers takes place in the barracks.
Currently absent.
