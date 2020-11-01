[Preparing]
Need screen resolution 1200x675 and more.
Editor looking for locations in <resources/images/locations/>.
Images of locations (main, free and over) must be in location directory (see documentation).
For example, main image of location <flat> must be in <resources/images/locations/flat/main.png>.

File with registration of locations (if already there) waited in <result/locations.rpy> relatively editor directory.
File with registration of objects waited in <result/objects.rpy>.
You can edit this files in text editor (with objects - need, because editor only installs, but does not register objects).


[Start]
On start shows screen <global space>, locations with view from above.

On right there is panel with locations list, you can <Add> or <Del> any location.
This list can be rewinded with key-arrows up/down.

You can move locations:
	1. Hover mouse to location,
	2. Press LMB (Left Mouse Button),
	3. Move mouse,
	4. Release LMB after end moving.
Location position relatively others need only for ease edit, it is not important in game.

Keys W, A, S and D moves camera.
Keys 9 and 0 - down and up scale.

Click on location - edit it.


[Location]
Tab <Properties> allows edit places and exits.
	Tab <Show>:
		Need to show or hide images [main, over and free] and [places, objects and exits].
	
	For adding new place:
		1. Click on <Add Place>,
		2. Type place name,
		3. Press <Ok> or <Enter>
	
	If place name is name of any location,
		the place becomes transition on it location, on place with name of current location.
	For example:
		Place <street> in location <flat> will transite to place <flat> on location <street>.
	Also such places have property <Exit Side>, that mean position of exit (red) or input (green).
		Aviable values: Up, Left, Right or Down.
	
	Property <Rotate on Enter> is responsible for character rotation on enter to location in this place.
	Usually it not need: for example, exit from right side of location-1 transite player to left side of location-2.
		In this cases, the property takes default <None> (not to rotate).
	But if need, you can set values <to_forward>, <to_left>, <to_right> or <to_back>.
	
	If place name starts with name of any object, and after stand "_pos", such object will be installed in this place.
	For example:
		bus will be installed in place "bus_pos",
		bench - in places "bench_pos-1", "bench_pos-2", ...
	
	There are buttons for <ReName Place> and <Delete Place>.
	
	Use <Add Exit> for adding new exit.
	Exits have extra properties:
		To Loc. - to what location?
		To Place - to what place of the location?
	Usually instead exits transitions makes with places with location name (see above).
	But direct creating of exit can be use, for example, for more than 1 transitions between 2 locations,
		or if need one-sided transition without aviable to return.
	
	After creating, new places and exits will selected.
	For select other - click on it on location.
	
	Changing properties x, y, xsize (width) and ysize (height) for selected place or exit:
		1. Click on button with property name to select the property,
		2. Change value with buttons +/- 1/10/100

Tab <Objects> shows list of registered objects:
	Use only for shows images and names.
	Arrows up/down rewinds this list.

Keys W, A, S, D, 9 and 0 works as on global screen.
Key Esc and button <Unselect> - pull off selection of place, exit or location (last - return to global screen).


[Ending]
All changes will auto saved in directory <result>.
Ready files can be moved to directory of your rpg-mod (game).

