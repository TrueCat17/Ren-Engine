init python:
	props_width = 300
	
	show_objects_list = False
	objects_indent = 10
	objects_btn_height = 70
	objects_start_btn = 0
	
	selected_exit_num = None
	selected_place_name = None
	selected_place_prop = 'x'
	
	selected_prop     = im.Composite((100, 20),
	                                 (0, 0), im.Rect('#000', 100, 20),
	                                 (3, 3), im.Rect('#FFF',  94, 14))
	not_selected_prop = im.Composite((100, 20),
	                                 (0, 0), im.Rect('#888', 100, 20),
	                                 (3, 3), im.Rect('#FFF',  94, 14))
	
	style.prop_btn = Style(style.textbutton)
	style.prop_btn.xanchor = 0.5
	style.prop_btn.xpos    = 0.5
	style.prop_btn.xsize = 220
	style.prop_btn.ysize = 26
	style.prop_btn.text_size = 20
	
	style.rotate_btn = Style(style.textbutton)
	style.rotate_btn.xsize = 80
	style.rotate_btn.ysize = 22
	style.rotate_btn.text_size = 16
	style.rotate_btn.color = 0
	
	
	def unselect():
		global selected_place_name, selected_exit_num
		if selected_place_name or selected_exit_num is not None:
			selected_place_name = selected_exit_num = None
		else:
			hide_screen('selected_location')
			show_screen('all_locations')
	
	def check_place(name):
		if selected_location.places.has_key(name):
			out_msg('Place <' + name + '> already exists')
			return False
		return True
	
	def add_place(name):
		if not check_place(name):
			return
		
		global selected_place_name, selected_exit_num
		selected_place_name = name
		selected_exit_num = None
		
		w, h = 100, 100
		x = ((get_stage_width() - props_width - w) / 2 - selected_location_x) / local_k
		y = ((get_stage_height() - h) / 2 - selected_location_y) / local_k
		
		register_place(selected_location.name, name, int(x), int(y), w, h)
		if locations.has_key(name):
			selected_location.places[name].side_exit = 'down'
		
		set_save_locations()
	
	def rename_place(name):
		global selected_place_name
		if selected_place_name == name or not check_place(name):
			return
		
		place = selected_location.places[name] = selected_location.places[selected_place_name]
		
		del selected_location.places[selected_place_name]
		selected_place_name = place.name = name
		
		if locations.has_key(name):
			set_exit_side()
			if place.side_exit is None:
				place.side_exit = 'down'
		elif place.has_key('side_exit'):
			del place.side_exit
		set_save_locations()
	
	def del_place():
		global selected_place_name
		del selected_location.places[selected_place_name]
		selected_place_name = None
		
		set_save_locations()
	
	def add_exit():
		global selected_exit_num, selected_place_name
		selected_exit_num = len(selected_location.exits)
		selected_place_name = None
		
		w, h = 100, 100
		x = ((get_stage_width() - props_width - w) / 2 - selected_location_x) / local_k
		y = ((get_stage_height() - h) / 2 - selected_location_y) / local_k
		
		register_exit(selected_location.name, '%to_location_name%', '%to_place_name%', int(x), int(y), w, h)
		set_save_locations()
	
	def del_exit():
		global selected_exit_num
		selected_location.exits = selected_location.exits[0:selected_exit_num] + selected_location.exits[selected_exit_num+1:]
		selected_exit_num = None
		
		set_save_locations()
	
	def rename_to_loc(to_location_name):
		exit = selected_location.exits[selected_exit_num]
		exit.to_location_name = to_location_name
	def rename_to_place(to_place_name):
		exit = selected_location.exits[selected_exit_num]
		exit.to_place_name = to_place_name
	
	
	def change_place_prop(d):
		is_place = selected_exit_num is None
		place = selected_location.places[selected_place_name] if is_place else selected_location.exits[selected_exit_num]
		
		if selected_place_prop == 'x':
			place.x = in_bounds(place.x + d, -700, selected_location.xsize + 700)
		elif selected_place_prop == 'y':
			place.y = in_bounds(place.y + d, -700, selected_location.ysize + 700)
		elif selected_place_prop == 'xsize':
			place.xsize = in_bounds(place.xsize + d, 2, in_bounds(selected_location.xsize - place.x, 1, 700))
		else: # ysize
			place.ysize = in_bounds(place.ysize + d, 2, in_bounds(selected_location.ysize - place.y, 1, 700))
		
		if is_place:
			set_exit_side()
		set_save_locations()
	
	def set_exit_side():
		if not locations.has_key(selected_place_name):
			return
		
		place = selected_location.places[selected_place_name]
		
		cx, cy = place.x + place.xsize / 2, place.y + place.ysize / 2
		acx, acy = selected_location.xsize - cx, selected_location.ysize - cy
		
		dists = (cx, cy, acx, acy)
		m = min(dists)
		if m < 100:
			place.side_exit = ('left', 'up', 'right', 'down')[dists.index(m)]
			set_save_locations()
	
	def get_place_coords(place):
		x, y, w, h = 0, 0, place.xsize, place.ysize
		side_exit = place.side_exit
		
		if side_exit in ('left', 'right'):
			exit_y, exit_w, exit_h = 0, w / 2, h
			w -= exit_w
			if side_exit == 'left':
				exit_x, x = 0, exit_w
			else:
				exit_x = w
		else:
			exit_x, exit_w, exit_h = 0, w, h / 2
			h -= exit_h
			if side_exit == 'up':
				exit_y, y = 0, exit_h
			else:
				exit_y = h
		return x, y, w, h, exit_x, exit_y, exit_w, exit_h


screen location_props:
	key 'ESCAPE' action unselect
	
	if show_objects_list:
		key 'UP'   action AddVariable('objects_start_btn', -1)
		key 'DOWN' action AddVariable('objects_start_btn', +1)
	
	image im.Rect('#DDD'):
		xalign 1.0
		size (props_width, 1.0)
		
		text (selected_location.name + ((',\n' + selected_place_name) if selected_place_name else '')):
			color 0x202020
			text_size 25
			text_align 'center'
			align (0.5, 0.99)
		
		vbox:
			xalign 0.5
			ypos 10
			spacing 10
			
			textbutton ('Unselect (' + ('place' if selected_place_name else 'location' if selected_exit_num is None else 'exit') + ')'):
				xalign 0.5
				size (200, 25)
				text_size 20
				action unselect
			
			hbox:
				xalign 0.5
				spacing 20
				
				textbutton 'Props':
					size (100, 25)
					text_size 20
					action SetVariable('show_objects_list', False)
				textbutton 'Objects':
					size (100, 25)
					text_size 20
					action SetVariable('show_objects_list', True)
		
		
		if show_objects_list:
			python:
				names = location_objects.keys()
				names.sort()
				
				objects_count_btns = (get_stage_height() - objects_indent - 150) / (objects_btn_height + objects_indent)
				objects_start_btn = in_bounds(objects_start_btn, 0, max(0, len(names) - objects_count_btns))
				
				names = names[objects_start_btn:objects_start_btn + objects_count_btns]
			
			vbox:
				xalign 0.5
				ypos (100 + objects_indent)
				spacing objects_indent
				
				for name in names:
					image im.Rect('#B80'):
						size (props_width - 50, objects_btn_height)
						
						python:
							obj = location_objects[name]
							main_frame = obj['animations'][None]
							obj_image = get_image_or_similar(main_frame['directory'] + main_frame['main_image'] + '.' + location_object_ext)
							
							w, h = get_image_size(obj_image)
							k = 64.0 / max(w, h)
							w, h = int(w * k), int(h * k)
							image = im.Scale(image, w, h)
						
						image obj_image:
							anchor (0.5, 0.5)
							pos (objects_btn_height / 2, objects_btn_height / 2)
							size (w, h)
						
						text name:
							color 0x404040
							text_size 20
							xpos objects_btn_height + objects_indent
							yalign 0.5
		
		
		else:
			vbox:
				pos (30, 80)
				spacing 10
				
				image im.Rect('#FFF'):
					size (100, 130)
					
					text 'Show:':
						color 0x00FF00
						text_size 20
						xalign 0.1
						ypos 5
					
					vbox:
						align (0.4, 0.5)
						spacing 3
						
						null ysize 20
						
						hbox:
							button:
								size (16, 16)
								
								ground (checkbox_no if persistent.hide_main else checkbox_yes)
								action SetDict(persistent, 'hide_main', not persistent.hide_main)
							null xsize 10
							text 'Main':
								color 0
								yalign 0.5
								text_size 14
						
						if selected_location.over():
							hbox:
								button:
									size (16, 16)
									
									ground (checkbox_no if persistent.hide_over else checkbox_yes)
									action SetDict(persistent, 'hide_over', not persistent.hide_over)
								null xsize 10
								text 'Over':
									color 0
									yalign 0.5
									text_size 14
						
						if selected_location.free():
							hbox:
								button:
									size (16, 16)
									
									ground (checkbox_yes if persistent.show_free else checkbox_no)
									action SetDict(persistent, 'show_free', not persistent.show_free)
								null xsize 10
								text 'Free':
									color 0
									yalign 0.5
									text_size 14
						
						if selected_location.places:
							hbox:
								button:
									size (16, 16)
									
									ground (checkbox_no if persistent.hide_places else checkbox_yes)
									action SetDict(persistent, 'hide_places', not persistent.hide_places)
								null xsize 10
								text ('Places (' + str(len(selected_location.places)) + ')'):
									color 0
									yalign 0.5
									text_size 14
						
						if selected_location.exits:
							hbox:
								button:
									size (16, 16)
									
									ground (checkbox_no if persistent.hide_exits else checkbox_yes)
									action SetDict(persistent, 'hide_exits', not persistent.hide_exits)
								null xsize 10
								text ('Exits (' + str(len(selected_location.exits)) + ')'):
									color 0
									yalign 0.5
									text_size 14
			
			vbox:
				spacing 10
				xalign 0.5
				ypos 0.30
				
				textbutton 'Add Place':
					style prop_btn
					action ask_str(add_place)
				textbutton 'Add Exit':
					style prop_btn
					action add_exit
				
				if selected_place_name or selected_exit_num is not None:
					python:
						is_place = selected_exit_num is None
						place = selected_location.places[selected_place_name] if is_place else selected_location.exits[selected_exit_num]
					
					if is_place:
						textbutton 'ReName Place':
							style prop_btn
							action ask_str(rename_place, selected_place_name)
						textbutton 'Delete Place':
							style prop_btn
							action del_place
					else:
						textbutton ('To Loc: ' + place.to_location_name):
							style prop_btn
							text_size 15
							action ask_str(rename_to_loc, place.to_location_name)
						textbutton ('To Place: ' + place.to_place_name):
							style prop_btn
							text_size 15
							action ask_str(rename_to_place, place.to_place_name)
						textbutton 'Delete Exit':
							style prop_btn
							action del_exit
					
					hbox:
						xalign 0.5
						spacing 5
						
						vbox:
							spacing 5
							
							for prop in 'x y xsize ysize'.split(' '):
								textbutton (prop + ': ' + str(place[prop])):
									ground (selected_prop if selected_place_prop == prop else not_selected_prop)
									
									xalign 0.5
									size (100, 22)
									text_size 16
									color 0
									action SetVariable('selected_place_prop', prop)
						
						vbox:
							spacing 10
							yalign 0.5
							
							for d in (1, 10, 100):
								hbox:
									spacing 5
									
									textbutton ('-' + str(d)):
										size (50, 20)
										text_size 15
										action change_place_prop(-d)
									textbutton ('+' + str(d)):
										size (50, 20)
										text_size 15
										action change_place_prop(+d)
					
					if locations.has_key(selected_place_name):
						vbox:
							xalign 0.5
							spacing 5
							
							text '{b}Exit Side:{/b}':
								color 0xFF0000
								text_size 18
							
							textbutton 'Up':
								style rotate_btn
								ground (selected_prop if place.side_exit == 'up' else not_selected_prop)
								xalign 0.5
								action [SetDict(place, 'side_exit', 'up'), set_save_locations]
							
							hbox:
								spacing 5
								xalign 0.5
								
								textbutton 'Left':
									style rotate_btn
									ground (selected_prop if place.side_exit == 'left' else not_selected_prop)
									action [SetDict(place, 'side_exit', 'left'), set_save_locations]
								textbutton 'Right':
									style rotate_btn
									ground (selected_prop if place.side_exit == 'right' else not_selected_prop)
									action [SetDict(place, 'side_exit', 'right'), set_save_locations]
							
							textbutton 'Down':
								style rotate_btn
								ground (selected_prop if place.side_exit == 'down' else not_selected_prop)
								xalign 0.5
								action [SetDict(place, 'side_exit', 'down'), set_save_locations]
					
					if is_place:
						vbox:
							xalign 0.5
							spacing 5
							
							text '{b}Rotate on Enter:':
								color 0x0000FF
								text_size 18
							
							textbutton 'to_forward':
								style rotate_btn
								ground (selected_prop if place.to_side == to_forward else not_selected_prop)
								xalign 0.5
								action [SetDict(place, 'to_side', to_forward), set_save_locations]
							
							hbox:
								spacing 5
								xalign 0.5
								
								textbutton 'to_left':
									style rotate_btn
									ground (selected_prop if place.to_side == to_left else not_selected_prop)
									action [SetDict(place, 'to_side', to_left), set_save_locations]
								textbutton 'None':
									style rotate_btn
									ground (selected_prop if place.to_side == None else not_selected_prop)
									action [SetDict(place, 'to_side', None), set_save_locations]
								textbutton 'to_right':
									style rotate_btn
									ground (selected_prop if place.to_side == to_right else not_selected_prop)
									action [SetDict(place, 'to_side', to_right), set_save_locations]
							
							textbutton 'to_back':
								style rotate_btn
								ground (selected_prop if place.to_side == to_back else not_selected_prop)
								xalign 0.5
								action [SetDict(place, 'to_side', to_back), set_save_locations]

