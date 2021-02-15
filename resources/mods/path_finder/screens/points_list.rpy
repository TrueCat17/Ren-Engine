init python:
	init_checkboxes()
	
	out_text = ''
	def set_out_text(text):
		global out_text
		out_text = text
	
	points_left = False
	points_width = 250
	
	points_indent = 10
	points_btn_height = 50
	
	brute_force = False
	
	max_count_points = 5
	points = []
	def add_point():
		if len(points) == max_count_points:
			return
		
		x, y = get_local_mouse()
		x, y = int((x - draw_location.x) / location_zoom), int((y - draw_location.y) / location_zoom)
		
		if draw_location.free():
			color = get_image_pixel(draw_location.free(), x, y)
			alpha = color & 255
			if alpha != 255:
				return
		for obj in draw_location.objects:
			if not isinstance(obj, RpgLocation) and obj.free:
				obj_x = int(obj.x + obj.xoffset - get_absolute(obj.xanchor, obj.xsize))
				obj_y = int(obj.y + obj.yoffset - get_absolute(obj.yanchor, obj.ysize))
				if x < obj_x or y < obj_y or x >= obj_x + obj.xsize or y >= obj_y + obj.ysize: continue
				
				color = get_image_pixel(im.crop(obj.free(), obj.crop), x - obj_x, y - obj_y)
				alpha = color & 255
				if alpha != 0:
					return
		
		points.append((draw_location.name, {'x': x, 'y': y}))
	
	def swap_points(index1, index2):
		if index1 >= 0 and index2 < len(points):
			points[index1], points[index2] = points[index2], points[index1]
	
	def start_moving():
		if len(points) > 1:
			location, place = points[0]
			
			global draw_location
			draw_location = None
			
			set_location(location, place)
			cam_to(me, 0)
			show_screen('selected_location')
			
			st = time.time()
			if not me.move_to_place(points, 1, brute_force=brute_force):
				me.move_to_place(None)
				stop_moving()
				set_out_text('Path not found')
				return
			dtime_ms = (time.time() - st) * 1000
			set_out_text(_('Spent') + ': ' + str(round(dtime_ms, 2)) + ' ms')
			
			if has_screen('all_locations'):
				hide_screen('all_locations')
	
	def stop_moving():
		me.paths = None
		if me.location:
			hide_character(me)
			global cam_object
			cam_object = {'x': me.x, 'y': me.y}
		else:
			restore_locations_coords()

screen points_list:
	zorder 10
	alpha (0.5 if me.paths else 1)
	
	key 'P' action make_screenshot
	
	image im.rect('#FFF'):
		xalign (0.0 if points_left else 1.0)
		yalign 0.5
		size (points_width, 0.7)
		
		vbox:
			ypos points_indent
			spacing points_indent
			
			textbutton _('To right' if points_left else 'To left'):
				xalign 0.5
				text_size 20
				action SetVariable('points_left', not points_left)
			
			text ('Points aviable: %s / %s' % (max_count_points - len(points), max_count_points)):
				color 0
				text_align 'center'
				xsize points_width
			
			$ i = 0
			while i < len(points):
				python:
					point = points[i]
					location_name = point[0]
					x, y = point[1]['x'], point[1]['y']
				
				image im.rect('#F80'):
					xpos points_indent
					yanchor 0.5
					size (points_width - 2 * points_indent, points_btn_height)
					
					text ("%s: (%s, %s)" % (location_name, x, y)):
						align (0.5, 0.2)
						text_size 20
						color 0x404040
					
					hbox:
						align (0.5, 0.8)
						
						textbutton ('/\\'):
							size (16, 16)
							text_size 14
							action swap_points(i - 1, i)
						textbutton ('\\/'):
							size (16, 16)
							text_size 14
							action swap_points(i, i + 1)
						textbutton ('X'):
							size (16, 16)
							text_size 14
							color 0xFF0000
							action [SetVariable('points', points[:i] + points[i+1:]), SetVariable('i', i - 1)]
				$ i += 1
		
		vbox:
			align (0.5, 0.95)
			spacing points_indent
			
			hbox:
				xalign 0.5
				spacing points_indent
				
				button:
					size (20, 20)
					
					ground (checkbox_yes if brute_force else checkbox_no)
					action SetVariable('brute_force', not brute_force)
				
				text 'brute_force':
					color 0
					yalign 0.5
					text_size 20
			
			textbutton _('Stop' if me.paths else 'Start'):
				xalign 0.5
				action stop_moving if me.paths else start_moving
			
			text out_text:
				color 0
				xalign 0.5

