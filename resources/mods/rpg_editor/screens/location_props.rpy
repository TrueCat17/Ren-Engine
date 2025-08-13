init python:
	input_allow_symbols = alphabet + alphabet.upper() + numbers + '_-+'
	
	props_width = 300
	
	show_objects = False
	objects_indent = 5
	objects_btn_size = 70
	objects_image_size = 64
	objects_start_btn = 0
	
	selected_place_name = None
	selected_place_prop = 'x'
	
	min_place_size = 4
	
	
	def unselect():
		global selected_place_name
		if selected_place_name:
			selected_place_name = None
		else:
			hide_screen('selected_location')
			show_screen('all_locations')
	
	def check_place(name):
		if name in selected_location.places:
			notification.out(_('Place <%s> already exists'), name)
			return False
		return True
	
	def add_place(name):
		if not check_place(name):
			return
		
		global selected_place_name
		selected_place_name = name
		
		w, h = 100, 100
		x = ((get_stage_width() - props_width - w) / 2 - selected_location_x) / local_k
		y = ((get_stage_height() - h) / 2 - selected_location_y) / local_k
		
		register_place(selected_location.name, name, int(x), int(y), w, h)
		
		if name in rpg_locations:
			place = selected_location.places[name]
			place.to_location_name = name
			place.to_place_name = selected_location.name
		
		on_place_changed()
	
	def rename_place(name):
		global selected_place_name
		if selected_place_name == name or not check_place(name):
			return
		
		place = selected_location.places[name] = selected_location.places[selected_place_name]
		
		del selected_location.places[selected_place_name]
		selected_place_name = place.name = name
		
		if name in rpg_locations:
			place = selected_location.places[name]
			place.to_location_name = name
			place.to_place_name = selected_location.name
		
		on_place_changed()
	
	def del_place():
		global selected_place_name
		del selected_location.places[selected_place_name]
		selected_place_name = None
		on_place_changed()
	
	
	def rename_to_loc(to_location_name):
		place = selected_location.places[selected_place_name]
		place.to_location_name = to_location_name or None
		on_place_changed()
	def rename_to_place(to_place_name):
		place = selected_location.places[selected_place_name]
		place.to_place_name = to_place_name
		on_place_changed()
	
	
	def change_place_prop(d):
		place = selected_location.places[selected_place_name]
		
		if selected_place_prop == 'x':
			place.x = in_bounds(place.x + d, -700, selected_location.xsize + 700)
		elif selected_place_prop == 'y':
			place.y = in_bounds(place.y + d, -700, selected_location.ysize + 700)
		elif selected_place_prop == 'xsize':
			place.xsize = in_bounds(place.xsize + d, min_place_size, in_bounds(selected_location.xsize - place.x, 1, 700))
		else: # ysize
			place.ysize = in_bounds(place.ysize + d, min_place_size, in_bounds(selected_location.ysize - place.y, 1, 700))
		
		on_place_changed()


screen location_props:
	key 'ESCAPE' action unselect
	
	image location_props_bg:
		xalign 1.0
		xsize props_width
		ysize 1.0
		
		text (selected_location.name + ((',\n' + selected_place_name) if selected_place_name else '')):
			style 'selected_name'
			ypos get_stage_height() - objects_indent
		
		vbox:
			xalign 0.5
			ypos objects_indent
			spacing objects_indent
			
			textbutton _('Unselect ' + ('place' if selected_place_name else 'location')):
				style 'unselect_btn'
				action unselect
			
			hbox:
				xalign 0.5
				spacing 10
				
				textbutton _('Properties'):
					style 'rpg_editor_btn'
					selected not show_objects
					size (120, 25)
					text_size 22
					action 'show_objects = False'
				textbutton _('Objects'):
					style 'rpg_editor_btn'
					selected show_objects
					size (120, 25)
					text_size 22
					action 'show_objects = True'
			
			
			if show_objects:
				$ allow_arrows()
				key 'UP'   action 'objects_start_btn -= 1'
				key 'DOWN' action 'objects_start_btn += 1'
				
				python:
					names = sorted(location_objects.keys())
					
					height = get_stage_height()
					height -= objects_indent * 3 + style.unselect_btn.get_current('ysize') + style.rpg_editor_btn.get_current('ysize')
					height -= objects_indent * 2 + style.selected_name.get_current('text_size') * 2
					count_btns = int(height - objects_indent) // (objects_btn_size + objects_indent)
					objects_start_btn = in_bounds(objects_start_btn, 0, max(0, len(names) - count_btns))
					
					names = names[objects_start_btn:objects_start_btn + count_btns]
				
				for name in names:
					image obj_bg:
						size (props_width - 50, objects_btn_size)
						
						python:
							obj = location_objects[name]
							main_frame = obj.animations[None]
							obj_image = get_image_or_similar(main_frame.directory + main_frame.main_image)
							
							w, h = get_image_size(obj_image)
							k = objects_image_size / max(w, h)
							w, h = int(w * k), int(h * k)
						
						image obj_image:
							anchor 0.5
							pos objects_btn_size // 2
							size (w, h)
						
						text name:
							color '#444'
							text_size 20
							xpos objects_btn_size
							xsize props_width - 50 - objects_btn_size
							yalign 0.5
			
			
			else:
				image location_props_toggle_bg:
					xalign 0.5
					size (210, 60)
					
					text (_('Show') + ':'):
						color 0
						text_size 20
						pos 3
					
					hbox:
						spacing 4
						xalign 0.5
						ypos 22
						
						vbox:
							spacing 4
							
							button:
								style 'bool_btn'
								action ToggleVariable('persistent.hide_main')
								
								image (checkbox_no if persistent.hide_main else checkbox_yes) style 'bool_btn_checkbox'
								text 'Main' style 'bool_btn_text'
							
							button:
								style 'bool_btn'
								alpha 1 if selected_location.over() else 0
								action ToggleVariable('persistent.hide_over')
								
								image (checkbox_no if persistent.hide_over else checkbox_yes) style 'bool_btn_checkbox'
								text 'Over' style 'bool_btn_text'
						
						vbox:
							spacing 4
							
							button:
								style 'bool_btn'
								alpha 1 if selected_location.free() else 0
								action ToggleVariable('persistent.hide_free')
								
								image (checkbox_no if persistent.hide_free else checkbox_yes) style 'bool_btn_checkbox'
								text 'Free' style 'bool_btn_text'
							
							button:
								style 'bool_btn'
								alpha 1 if selected_location.places else 0
								action ToggleVariable('persistent.hide_places')
								
								image (checkbox_no if persistent.hide_places else checkbox_yes) style 'bool_btn_checkbox'
								text (_('Places') + ' (' + str(len(selected_location.places)) + ')') style 'bool_btn_text'
				
				if selected_place_name is None:
					textbutton (_('Add') + ' ' + _('Place')):
						style 'place_btn'
						action input.ask_str(add_place, 'New place name', allow = input_allow_symbols)
				else:
					$ place = selected_location.places[selected_place_name]
					
					textbutton (_('Rename') + ' ' + _('Place')):
						style 'place_btn'
						action input.ask_str(rename_place, 'Renaming the place', selected_place_name, allow = input_allow_symbols)
					textbutton (_('Delete') + ' ' + _('Place')):
						style 'place_btn'
						action del_place
					
					$ place_error = place.to_location_name and place.to_location_name not in rpg_locations
					textbutton (_('To location') + ': ' + (place.to_location_name or '')):
						style 'place_error_btn' if place_error else 'place_btn'
						action input.ask_str(rename_to_loc, 'To location', place.to_location_name or '', allow = input_allow_symbols)
					
					
					if place.to_location_name:
						vbox:
							spacing 3
							xalign 0.5
							
							$ place_error = place.to_location_name not in rpg_locations or place.to_place_name not in rpg_locations[place.to_location_name].places
							textbutton (_('To place') + ': ' + (place.to_place_name or '')):
								style 'place_error_btn' if place_error else 'place_btn'
								action input.ask_str(rename_to_place, 'To place', place.to_place_name or '', allow = input_allow_symbols)
							
							null ysize 1
							
							text (_('Exit side') + ':'):
								color '#00F'
								text_size 22
								xalign 0.5
							
							textbutton _('Up'):
								style 'rotate_btn'
								xalign 0.5
								selected place.exit_side == 'up'
								action 'place.exit_side = "up"; on_place_changed()'
							
							hbox:
								spacing 3
								xalign 0.5
								
								textbutton _('Left'):
									style 'rotate_btn'
									selected place.exit_side == 'left'
									action 'place.exit_side = "left"; on_place_changed()'
								textbutton _('None'):
									style 'rotate_btn'
									selected place.exit_side is None
									action 'place.exit_side = None; on_place_changed()'
								textbutton _('Right'):
									style 'rotate_btn'
									selected place.exit_side == 'right'
									action 'place.exit_side = "right"; on_place_changed()'
							
							textbutton _('Down'):
								style 'rotate_btn'
								xalign 0.5
								selected place.exit_side == 'down'
								action 'place.exit_side = "down"; on_place_changed()'
							
							null ysize 1
							
							text (_('Rotate on exit') + ':'):
								color '#F00'
								text_size 22
								xalign 0.5
							
							textbutton _('To forward'):
								style 'rotate_btn'
								xalign 0.5
								selected place.to_side == to_forward
								action 'place.to_side = to_forward; on_place_changed()'
							
							hbox:
								spacing 3
								xalign 0.5
								
								textbutton _('To left'):
									style 'rotate_btn'
									selected place.to_side == to_left
									action 'place.to_side = to_left; on_place_changed()'
								textbutton _('None'):
									style 'rotate_btn'
									selected place.to_side is None
									action 'place.to_side = None; on_place_changed()'
								textbutton _('To right'):
									style 'rotate_btn'
									selected place.to_side == to_right
									action 'place.to_side = to_right; on_place_changed()'
							
							textbutton _('To back'):
								style 'rotate_btn'
								xalign 0.5
								selected place.to_side == to_back
								action 'place.to_side = to_back; on_place_changed()'
					
					
					null ysize 10
					
					hbox:
						xalign 0.5
						spacing 5
						
						vbox:
							spacing 3
							
							for prop in ('x', 'y', 'xsize', 'ysize'):
								textbutton (prop + ': ' + str(place[prop])):
									style 'prop_btn'
									selected selected_place_prop == prop
									action 'selected_place_prop = prop'
						
						vbox:
							spacing 5
							yalign 0.5
							
							for d in (1, 10, 100):
								hbox:
									spacing 3
									
									textbutton ('-' + str(d)):
										style 'change_prop_btn'
										action change_place_prop(-d)
									textbutton ('+' + str(d)):
										style 'change_prop_btn'
										action change_place_prop(+d)
