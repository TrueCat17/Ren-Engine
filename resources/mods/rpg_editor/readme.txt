[Preparing]
Need screen resolution 1200x675 and more.
Editor looking for locations in <resources/images/locations/>.
Images of locations (main, free and over) must be in location directory (see documentation).
For example, main image of location <flat> must be in <resources/images/locations/flat/main.png>.

File with registration of locations (if already there) waited in <result/locations.rpy> relatively editor directory.
File with registration of objects waited in <result/objects.rpy>.
You can edit this files in text editor (with objects - need, because editor only installs, but does not register objects).


[Start]
First, a global space is displayed, something like a "view from above".

On the right is a panel with a list of locations, you can <Add> or <Del> any location.
This list can be rewinded with key-arrows up/down.

You can move locations:
	1. Hover mouse to location,
	2. Press LMB (Left Mouse Button),
	3. Move mouse,
	4. Release LMB after end moving.
Location position relatively others need only for ease edit, it is not important in game.

Keys \
{color=#F00}W{/color}, \
{color=#F00}A{/color}, \
{color=#F00}S{/color} and \
{color=#F00}D{/color} moves camera.
Keys \
{color=#F00}-{/color} and \
{color=#F00}+{/color} - down and up scale.

Click on location - edit it.


[Location]
Tab <Properties> allows edit places.
	Panel <Show>:
		Need to show or hide images [main, over and free] and [places and objects].
	
	For adding new place:
		1. Click on <Add Place>,
		2. Type place name,
		3. Press <Ok> or <Enter>
	
	If place's property "To Location" defined, then this place is exit to defined location.
	In this case, also need define property "To Place" in the location.
	
	Also such places have properties:
		<Exit Side>, that define position of exit (red) or input (yellow).
		Available values: Up, Left, Right, Down and None (all place is exit).
		
		<Rotate on Enter> is responsible for character rotation after transition.
		Usually it not need: for example, exit from right side of location-1 transite player to left side of location-2.
			In this cases, the property takes default <None> (not to rotate).
		But if need, you can set values <To forward>, <To left>, <To right> or <To back>.
	
	If place name starts with name of any object, and after stands "_pos", then such object will be installed in this place.
	For example:
		bus will be installed in place "bus_pos",
		bench - in places "bench_pos-1", "bench_pos-2", ...
	
	There are buttons for <ReName Place> and <Delete Place>.
	
	After creating new place will selected.
	For select other - click on it on location.
	
	Changing properties x, y, xsize (width) and ysize (height) for selected place:
		1. Click on button with property name to select the property,
		2. Change value with buttons +/- 1/10/100.

Tab <Objects> shows list of registered objects:
	Use only for shows images and names.
	Arrows up/down rewinds this list.

Keys \
{color=#F00}W{/color}, \
{color=#F00}A{/color}, \
{color=#F00}S{/color}, \
{color=#F00}D{/color}, \
{color=#F00}-{/color} and \
{color=#F00}+{/color} works as on global screen.
Key Esc and button <Unselect> - pull off selection of place or location (last - return to global screen).


[Ending]
All changes will auto saved in directory <result>.
Ready files can be moved to directory of your rpg-mod (game).
