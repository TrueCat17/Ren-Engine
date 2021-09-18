screen map:
	image im.rect('#222'):
		size 1.0
	
	$ sc_map.ctrl = False
	$ sc_map.shift = False
	key 'LEFT CTRL'  action SetDict(sc_map, 'ctrl', True) first_delay 0
	key 'RIGHT CTRL' action SetDict(sc_map, 'ctrl', True) first_delay 0
	key 'LEFT SHIFT'  action SetDict(sc_map, 'shift', True) first_delay 0
	key 'RIGHT SHIFT' action SetDict(sc_map, 'shift', True) first_delay 0
	
	key 'w' action sc_map.move( 0, -1)
	key 'a' action sc_map.move(-1,  0)
	key 's' action sc_map.move( 0, +1)
	key 'd' action sc_map.move(+1,  0)
	
	key '-' action sc_map.add_zoom(-1)
	key '=' action sc_map.add_zoom(+1)
	
	key 'F5' action SetDict(sc_map, 'mark_unloaded', not sc_map.mark_unloaded)
	
	for obj in sc_map.to_draw():
		if obj.has_key('image'):
			image obj['image']:
				pos  (obj['xpos'], obj['ypos'])
				size  obj['size']
				alpha obj.get('alpha', 1)
		else:
			text obj['text']:
				pos  (obj['xpos'], obj['ypos'])
				bold  True
				color 0xFFFFFF
				outlinecolor 0
				xanchor 0.5
					
	
	button:
		ground im.rect('#000')
		hover  im.rect('#000')
		size   1.0
		mouse  False
		alpha  0.01
		action control.on_click
	
	$ sc_map.ctrl = False
