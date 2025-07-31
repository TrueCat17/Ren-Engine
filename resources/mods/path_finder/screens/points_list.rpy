init python:
	out_text = ''
	def set_out_text(text):
		global out_text
		out_text = text
	
	points_list_bg = im.rect('#FFF')
	point_bg = im.rect('#F80')
	
	points_left = False
	points_width = 250
	
	points_indent = 8
	points_btn_height = 54
	
	brute_force = False
	
	max_count_points = 5
	points = []
	def add_point():
		if len(points) == max_count_points:
			return
		
		x, y = get_local_mouse()
		x = int((x - draw_location.x) / location_zoom)
		y = int((y - draw_location.y) / location_zoom)
		
		if draw_location.free():
			color = get_image_pixel(draw_location.free(), x, y)
			alpha = color & 0xFF
			if alpha != 0xFF:
				return
		
		for obj in draw_location.objects:
			if isinstance(obj, RpgLocation): continue
			
			obj_free = obj.free()
			if obj_free is None:
				continue
			
			w, h = get_image_size(obj_free)
			obj_x = int(obj.x + obj.xoffset - get_absolute(obj.xanchor, w))
			obj_y = int(obj.y + obj.yoffset - get_absolute(obj.yanchor, h))
			if x < obj_x or y < obj_y or x >= obj_x + w or y >= obj_y + h: continue
			
			if obj.crop:
				obj_free = im.crop(obj_free, obj.crop)
			
			color = get_image_pixel(obj_free, x - obj_x, y - obj_y)
			alpha = color & 0xFF
			if alpha != 0:
				return
		
		points.append((draw_location.name, { 'x': x, 'y': y }))
	
	def swap_points(index1, index2):
		if index1 >= 0 and index2 < len(points):
			points[index1], points[index2] = points[index2], points[index1]
	
	def start_moving():
		if len(points) <= 1:
			return
		
		location, place = points[0]
		
		global draw_location
		draw_location = None
		
		set_location(location, place)
		cam_to(me, 0)
		show_screen('selected_location')
		
		st = time.time()
		if not me.move_to_places(points, run = True, brute_force = brute_force):
			me.move_to_place(None)
			stop_moving()
			set_out_text('Path not found')
			return
		
		dtime_ms = (time.time() - st) * 1000
		set_out_text('%s: %.2f ms' % (_('Spent'), dtime_ms))
		
		if has_screen('all_locations'):
			hide_screen('all_locations')
	
	def stop_moving():
		me.paths = None
		prev_loc = me.location
		if prev_loc:
			hide_character(me)
			prev_loc.cam_object = { 'x': me.x, 'y': me.y }
		else:
			restore_locations_coords()

screen points_list:
	zorder 10
	alpha 0.5 if me.paths else 1
	
	image points_list_bg:
		xalign 0.0 if points_left else 1.0
		yalign 0.5
		xsize points_width
		ysize 0.8
		
		vbox:
			ypos    points_indent
			spacing points_indent
			
			textbutton _('To right' if points_left else 'To left'):
				style 'path_finder_to_side_btn'
				action 'points_left = not points_left'
			
			text (_('Points available: %s / %s') % (max_count_points - len(points), max_count_points)):
				color 0
				text_align 'center'
				xsize points_width
			
			$ i = 0
			while i < len(points):
				python:
					point = points[i]
					location_name = point[0]
					x, y = point[1]['x'], point[1]['y']
				
				image point_bg:
					xpos points_indent
					size (points_width - 2 * points_indent, points_btn_height)
					
					vbox:
						spacing 2
						align 0.5
						
						text ('%s: (%s, %s)' % (location_name, x, y)):
							xalign 0.5
							text_size 20
							color '#333'
						
						hbox:
							xalign 0.5
							spacing 2
							
							$ btn_params = (
								('↑', '#FFF', 'swap_points(i - 1, i)'),
								('↓', '#FFF', 'swap_points(i, i + 1)'),
								('X', '#F00', 'points.pop(i); i -= 1'),
							)
							for text, color, action in btn_params:
								textbutton text:
									style 'path_finder_point_btn'
									color  color
									action action
				
				$ i += 1
		
		vbox:
			align (0.5, 1.0)
			spacing points_indent
			
			button:
				style 'bool_button'
				xalign 0.5
				xsize style.path_finder_btn.get_current('xsize')
				ysize 25
				action 'brute_force = not brute_force'
				
				hbox:
					spacing points_indent
					align 0.5
					
					image (path_finder_checkbox_yes if brute_force else path_finder_checkbox_no):
						size 20
						corner_sizes -1
						yalign 0.5
					
					text 'brute_force':
						color 0
						text_size 20
						yalign 0.5
			
			textbutton _('Stop' if me.paths else 'Start'):
				style 'path_finder_btn'
				action stop_moving if me.paths else start_moving
			
			text (out_text or ' '):
				color 0
				xalign 0.5
			
			null ysize 1
