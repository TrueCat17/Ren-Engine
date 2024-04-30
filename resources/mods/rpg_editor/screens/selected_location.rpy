init python:
	local_cam_x, local_cam_y = get_stage_width() // 2, get_stage_height() // 2
	
	local_k = absolute(1.0)
	local_k_min = absolute(0.25)
	local_k_max = absolute(8.0)
	
	local_speed = 25
	
	def get_objs_and_places():
		zoom = absolute(local_k)
		floor = math.floor
		def convert_rect(x, y, w, h, xa, ya):
			x = (x - xa * w) * zoom
			y = (y - ya * h) * zoom
			
			floor_x = floor(x)
			floor_y = floor(y)
			
			floor_w = floor(x + w * zoom) - floor_x
			floor_h = floor(y + h * zoom) - floor_y
			
			return (floor_x, floor_y), (floor_w, floor_h)
		
		objs = []
		places = []
		
		for place in selected_location.places.values():
			if '_pos' in place.name:
				obj_name = place.name[0:place.name.index('_pos')]
				if obj_name in location_objects:
					obj = location_objects[obj_name]
					main_frame = obj['animations'][None]
					obj_image = get_image_or_similar(main_frame['directory'] + main_frame['main_image'] + '.' + location_object_ext)
					
					x = place.x + place.xsize / 2
					y = place.y + place.ysize / 2
					w, h = get_image_size(obj_image)
					
					pos, size = convert_rect(x, y, w, h, 0.5, 1.0)
					objs.append((obj_image, pos, size))
			
			if place.to_location_name is None:
				image = im.Rect('#0B0')
			else:
				to_loc = rpg_locations.get(place.to_location_name, None)
				if to_loc and place.to_place_name in to_loc.places:
					if place.exit_side:
						x, y, w, h = place.get_rect(of_exit = False)
						ex, ey, ew, eh = place.get_rect(of_exit = True)
						image = im.Composite((place.xsize, place.ysize),
									         (x  - place.x, y  - place.y), im.Rect('#FF0',  w,  h),
									         (ex - place.x, ey - place.y), im.Rect('#B00', ew, eh))
					else:
						image = im.Rect('#B00')
				else:
					image = im.Rect('#F00')
			
			pos, size = convert_rect(place.x, place.y, max(place.xsize, 3), max(place.ysize, 3), 0, 0)
			places.append((place, image, pos, size))
		
		objs.sort(key = lambda image_pos_size: image_pos_size[1][1])
		places.sort(key = lambda place_and_others: place_and_others[0].xsize * place_and_others[0].ysize, reverse = True)
		
		return objs, places
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('selected_location')


screen selected_location:
	image im.rect('#444'):
		size 1.0
	
	key 'w' action 'local_cam_y -= local_speed'
	key 'a' action 'local_cam_x -= local_speed'
	key 's' action 'local_cam_y += local_speed'
	key 'd' action 'local_cam_x += local_speed'
	
	for key in '-_':
		key key action SetVariable('local_k', max(local_k - local_k_min, local_k_min))
	for key in '+=':
		key key action SetVariable('local_k', min(local_k + local_k_min, local_k_max))
	
	python:
		selected_location = rpg_locations[selected_location_name]
		
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
			null:
				$ objs, places = get_objs_and_places()
				
				for obj_image, pos, size in objs:
					image obj_image:
						pos  pos
						size size
				
				for place, image, pos, size in places:
					button:
						ground image
						action 'selected_place_name = place.name'
						
						pos  pos
						size size
						
						alpha 0.5
	
	use location_props

