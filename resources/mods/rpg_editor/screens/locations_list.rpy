init python:
	init_checkboxes()
	
	locations_width = 300
	
	locations_indent = 10
	locations_btn_height = 70
	locations_start_btn = 0
	
	def change_using_location(location):
		location.using = not location.using
		if location.using:
			location.x = location.x or k * get_stage_width() / 3
			location.y = location.y or k * get_stage_height() / 2
		
		set_save_locations()

screen locations_list:
	key 'UP'   action AddVariable('locations_start_btn', -1)
	key 'DOWN' action AddVariable('locations_start_btn', +1)
	
	image im.Rect('#FFF'):
		xalign 1.0
		size (locations_width, 1.0)
		
		vbox:
			xalign 0.5
			ypos locations_indent
			spacing locations_indent
			
			python:
				names = locations.keys()
				names.sort()
				
				locations_count_btns = (get_stage_height() - locations_indent) / (locations_btn_height + locations_indent)
				locations_start_btn = in_bounds(locations_start_btn, 0, max(0, len(names) - locations_count_btns))
				
				names = names[locations_start_btn:locations_start_btn + locations_count_btns]
			
			for name in names:
				python:
					location = locations[name]
					color = '#0F0' if location.using else '#F80'
					preview = get_preview(name)
					size = '\n(' + str(location.xsize) + 'x' + str(location.ysize) + ')'
				
				image im.Rect(color):
					size (locations_width - 50, locations_btn_height)
					
					image preview:
						anchor (0.5, 0.5)
						pos (locations_btn_height / 2, locations_btn_height / 2)
						size (get_image_width(preview) / 2, get_image_height(preview) / 2)
					
					text (name + size):
						color 0x404040
						text_size 20
						xpos locations_btn_height + locations_indent
						yalign 0.5
					
					textbutton ('Del' if location.using else 'Add'):
						size (50, 25)
						text_size 20
						align (0.95, 0.75)
						action change_using_location(location)

