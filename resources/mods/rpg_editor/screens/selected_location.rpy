init:
	style transparent_btn is button:
		ground im.rect('#0000')
		hover  im.rect('#FFF8')
		corner_sizes 0
		alpha 0.5

init python:
	local_cam_x, local_cam_y = get_stage_width() // 2, get_stage_height() // 2
	
	local_k = absolute(1.0)
	local_k_min = absolute(0.25)
	local_k_max = absolute(8.0)
	
	local_speed = 25
	
	selected_location_bg = im.rect('#444')
	
	place_bg = im.rect('#0B0')
	exit_place_side_bg = im.rect('#FF0')
	exit_exit_side_bg = im.rect('#B00')
	exit_wrong_bg = im.rect('#F00')
	
	
	calced_objs = []
	calced_places = []
	
	def on_place_changed():
		set_save_locations()
		update_objs_and_places()
	
	def update_objs_and_places():
		zoom = absolute(local_k)
		floor = math.floor
		def convert_rect(x, y, w, h, xa, ya):
			x = (x - xa * w) * zoom
			y = (y - ya * h) * zoom
			
			floor_x = floor(x)
			floor_y = floor(y)
			
			floor_w = floor(x + w * zoom) - floor_x
			floor_h = floor(y + h * zoom) - floor_y
			
			return floor_x, floor_y, floor_w, floor_h
		
		calced_objs.clear()
		calced_places.clear()
		
		for place_name, place in selected_location.places.items():
			i = place_name.find('_pos')
			if i != -1:
				obj_name = place_name[:i]
				obj = location_objects.get(obj_name)
				if obj:
					main_frame = obj.animations[None]
					image = get_image_or_similar(main_frame.directory + main_frame.main_image)
					
					x = place.x + place.xsize / 2
					y = place.y + place.ysize / 2
					w, h = get_image_size(image)
					
					x, y, w, h = convert_rect(x, y, w, h, 0.5, 1.0)
					calced_objs.append((image, x, y, w, h))
			
			image2 = None
			
			if place.to_location_name is None:
				image = place_bg
			else:
				to_loc = rpg_locations.get(place.to_location_name, None)
				if to_loc and place.to_place_name in to_loc.places:
					if place.exit_side:
						x, y, w, h = place.get_rect(of_exit = False)
						ex, ey, ew, eh = place.get_rect(of_exit = True)
						
						image = exit_place_side_bg
						x, y, w, h = convert_rect(x, y, w, h, 0, 0)
						
						image2 = exit_exit_side_bg
						ex, ey, ew, eh = convert_rect(ex, ey, ew, eh, 0, 0)
					
					else:
						image = exit_exit_side_bg
				else:
					image = exit_wrong_bg
			
			images = []
			
			# c - common
			cx, cy, cw, ch = convert_rect(place.x, place.y, place.xsize, place.ysize, 0, 0)
			cw = max(cw, min_place_size)
			ch = max(ch, min_place_size)
			
			if not image2:
				x, y, w, h = 0, 0, cw, ch
			else:
				x -= cx
				y -= cy
			images.append((image, x, y, w, h))
			
			if image2:
				images.append((image2, ex - cx, ey - cy, ew, eh))
			
			calced_places.append((place, (cx, cy), (cw, ch), images))
		
		calced_objs.sort(key = lambda image_x_y_w_h: image_x_y_w_h[2])
		calced_places.sort(key = lambda place_and_others: place_and_others[0].xsize * place_and_others[0].ysize, reverse = True)
	
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('selected_location')


screen selected_location:
	image selected_location_bg:
		size 1.0
	
	key 'W' action 'local_cam_y -= local_speed'
	key 'A' action 'local_cam_x -= local_speed'
	key 'S' action 'local_cam_y += local_speed'
	key 'D' action 'local_cam_x += local_speed'
	
	for key in '-_':
		key key action 'local_k = max(local_k - local_k_min, local_k_min); update_objs_and_places()'
	for key in '+=':
		key key action 'local_k = min(local_k + local_k_min, local_k_max); update_objs_and_places()'
	
	python:
		x, y = int(-local_cam_x * local_k), int(-local_cam_y * local_k)
		w, h = int(selected_location.xsize * local_k), int(selected_location.ysize * local_k)
		sw, sh = get_stage_width() - props_width, get_stage_height()
		
		if w < sw:
			local_cam_x = (w - sw) / 2 / local_k
		else:
			if x > 0:
				local_cam_x = 0
			elif x + w < sw:
				local_cam_x = (w - sw) / local_k
		
		if h < sh:
			local_cam_y = (h - sh) / 2 / local_k
		else:
			if y > 0:
				local_cam_y = 0
			elif y + h < sh:
				local_cam_y = (h - sh) / local_k
		
		x, y = int(-local_cam_x * local_k), int(-local_cam_y * local_k)
		selected_location_x, selected_location_y = x, y
	
	null:
		pos (x, y)
		
		if not persistent.hide_main:
			image selected_location.main():
				size (w, h)
		if not persistent.hide_over and selected_location.over():
			image selected_location.over():
				size (w, h)
		if not persistent.hide_free and selected_location.free():
			image selected_location.free():
				size (w, h)
		
		if not persistent.hide_places:
			for image, x, y, w, h in calced_objs:
				image image:
					xpos x
					ypos y
					xsize w
					ysize h
			
			for place, pos, size, images in calced_places:
				button:
					style 'transparent_btn'
					pos  pos
					size size
					action 'selected_place_name = place.name'
					
					for image, x, y, w, h in images:
						image image:
							xpos x
							ypos y
							xsize w
							ysize h
	
	use location_props
