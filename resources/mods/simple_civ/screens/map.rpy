screen map:
	image sc_map.bg:
		size 1.0
	
	key 'W' action sc_map.move( 0, -1)
	key 'A' action sc_map.move(-1,  0)
	key 'S' action sc_map.move( 0, +1)
	key 'D' action sc_map.move(+1,  0)
	
	key '-' action sc_map.add_zoom(-1)
	key '=' action sc_map.add_zoom(+1)
	
	key 'F5' action ToggleVariable('sc_map.mark_unloaded')
	
	for obj in sc_map.to_draw():
		if 'image' in obj:
			image obj['image']:
				xpos obj['xpos']
				ypos obj['ypos']
				size  obj['size']
				alpha obj.get('alpha', 1)
		else:
			text obj['text']:
				xpos obj['xpos']
				ypos obj['ypos']
				bold  True
				color '#FFF'
				outlinecolor 0
				xanchor 0.5
	
	button:
		ground black_bg
		hover  black_bg
		size   1.0
		alpha  0.01
		mouse  False
		action control.on_click
