init python:
	location_list_width = 300
	
	location_list_indent = 10
	location_list_btn_height = 90
	location_list_start_btn = 0
	
	def change_using_location(location):
		if location.x is None:
			location.x = location.x or common_k * (get_stage_width() - location_list_width) / 2
			location.y = location.y or common_k * get_stage_height() / 2
		else:
			location.x = None
		
		set_save_locations()

screen location_list:
	$ allow_arrows()
	key 'UP'   action 'location_list_start_btn -= 1'
	key 'DOWN' action 'location_list_start_btn += 1'
	
	image location_list_bg:
		xalign 1.0
		xsize location_list_width
		ysize 1.0
		
		vbox:
			xalign 0.5
			ypos location_list_indent
			spacing location_list_indent
			
			python:
				names = sorted(rpg_locations.keys())
				
				count_btns = (get_stage_height() - location_list_indent) // (location_list_btn_height + location_list_indent)
				location_list_start_btn = in_bounds(location_list_start_btn, 0, max(0, len(names) - count_btns))
				
				names = names[location_list_start_btn:location_list_start_btn + count_btns]
			
			for name in names:
				$ location = rpg_locations[name]
				
				image (used_location_bg if location.x is not None else unused_location_bg):
					size (location_list_width - 50, location_list_btn_height)
					
					hbox:
						xpos location_list_indent
						yalign 0.5
						spacing location_list_indent
						
						null:
							size 64
							
							$ preview = get_preview(name)
							image preview:
								align 0.5
								size get_image_size(preview)
								zoom 0.5
						
						vbox:
							yalign 0.5
							spacing 5
							
							text ('%s\n(%sx%s)' % (name, location.xsize, location.ysize)):
								color '#444'
								text_size 20
							
							textbutton _('Add' if location.x is None else 'Delete'):
								style 'add_location_btn' if location.x is None else 'del_location_btn'
								action change_using_location(location)
