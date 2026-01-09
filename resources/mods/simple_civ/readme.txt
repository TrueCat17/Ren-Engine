[Base]

This is a simplified version of Civilization.

Distinguishing features:
* creation, improvement and destruction of buildings occur immediately;
* no units;
* no roads.

All this is necessary to remove routine and make the game more dynamic.
For example, clicking on a cell and selecting "build a farm" is much faster than creating a worker, \
moving him to the desired cell, building a road along the way, and then building the farm in the same way.
Simply removing unnecessary steps - routine that take up more than half the game time.

Your main "units" are buildings like sawmills, careers, farms, factories, etc.
Each activated building grants \
${green}+1${end} \
to your force on all cells within \
${orange}5${end} \
blocks of it in all directions (square, not circle).
Each building upgrade adds another \
${green}+1${end} \
to the same area.
However, barracks function differently; see the relevant section.

Buildings can only be constructed within the boundaries of your own territory \
(the one in which you have more force than everyone else at the start of a turn).
When you remove a building, the territory remains yours even if you have zero force, \
unless your opponents are claiming it.

After the turn ends, the borders will be updated.
Your cells that are transferred to your opponent are highlighted in \
${red}red${end}.
Your opponent's cells that are transferred to you are highlighted in \
${blue}blue${end}.

This highlighting can be toggled on/off with the F6 key.
Highlighting for disabled buildings - F5.

When a cell changes hands, the building is lost, preventing it from being claimed by the invader.
This is done to prevent the attacker from reaching the opposite end of the map without making an effort, \
and to prevent the game from ending two minutes after the start of the active confrontation phase.

The simplest game strategy:
1. Establish a defensive position on "your" part of the map by building all food and stone cells;
2. Maintain the wood in the same quantity as the stone;
3. Discover colleges and build \
${orange}10-20${end} \
of them to speed up further research;
4. Discover coal (+ build careers) and cement (+ build \
${orange}1-2${end} \
factories);
5. Discover the next levels for deficit stones and food, then upgrade their buildings;
6. Once you've achieved at least \
${green}+500${end} \
stone and wood per turn, you can begin attacking by building barracks on the borders;
7. Construct buildings to further increase resource production in new territories;
8. Don't forget to remove any barracks you no longer need after moving a border, as war is VERY expensive.

Even though AI opponents are quite weak, a beginner can lose to them even on difficulty level 0 - that's normal; \
it's worth trying again.
To repeat a previous map (and AI behavior), you only need to set the previous Seed - such replays will \
help you quickly understand your mistakes and experiment with different strategies.
Of course, for this to happen, other game parameters (not just the Seed) must also remain the same.

By the way, each bot difficulty level simply gives the bots a \
${green}+5${end} \
bonus to each resource type each turn.
To observe a specific player, simply press the desired key: \
${orange}1${end}, \
${orange}2${end}, \
${orange}3${end} or \
${orange}4${end}.
This also allows for a game between multiple players on a single computer, if needed.

Good luck!


[Keyboard]

Moving the map: \
${yellow}WASD${end} \
(+CTRL to move to the end).
Change scale: \
${yellow}+${end} and ${yellow}-${end}.

The cell context menu can be accessed by LMB (Left Mouse Button), RMB, \
the Menu key (next to the Right CTRL), or the Space and Enter keys.
You can use the up and down arrow keys to select items in this menu.
To cancel, press Esc.

Hovering over most buttons reveals a keyboard shortcut that performs the same action.
Keyboard controls are generally faster and more convenient than mouse controls.
Especially when you need to unbuild 40 old barracks and build the same number of new ones along a shifted border.

Next step: \
${yellow}Shift + Enter${end}.
Repeat construction of the previous building on the selected cell: \
${yellow}R${end}.
Improve the building: \
${yellow}I${end}.
Delete building: \
${yellow}U${end}.
Turn building on/off: \
${yellow}E${end}.

It can be convenient to hold down a key to perform an action and move the mouse over the desired cells.


[Buildings]

Buildings can be divided into simple (mining) and common.
Common buildings can be built on any cells.

The first group includes:
* Farm - built on the "food" field;
* Sawmill - on the "wood" field;
* Career - on the "stone," "coal," and "metal" fields.

These buildings can ONLY be built on their own fields, and they produce as many resources as their cell contains.
So, it's silly to build a sawmill in a "wood (10)" cell if there is a "wood (19)" cell.

Building performance depending on level (2, 3, 4) relative to 1: ${building_powers}.


[Prices]

Level 2 buildings require the use of cement.
Level 3 buildings require steel.
And Level 4 buildings require even larger quantities.

${building_price}


[Support]

Buildings consume base maintenance multiplied by their level.
A functioning building also provides \
${red}-1${end} \
to food and \
${green}+n${end} \
to science (according to the performance level from section ${yellow}Buildings${end}).

Basic building support:

${building_support}


[Production]

Basic production:

${building_production}


[Barracks]

Barracks are expensive buildings to maintain (unless disabled) with a special force mechanism \
(relative to civilian buildings).

A level 1 barracks provides 8 force points on its own cell, \
with force decreasing by 2 points per cell as the distance increases.
This means that near cells (a 3x3 square, excluding the center) receive 6 force, then 4, and then 2.

For each level, barracks provide \
${green}+2${end} \
force on their own cell.
Accordingly, the range at which they exert force also increases.


[Achivements]

All achievements, except for "observer", are only awarded in a single game against 3 bots.

${achivements}
