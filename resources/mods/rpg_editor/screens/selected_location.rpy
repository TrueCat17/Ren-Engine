init python:
	local_cam_x, local_cam_y = get_stage_width() / 2, get_stage_height() / 2
	
	local_k = absolute(1.0)
	local_k_min = absolute(0.25)
	local_k_max = absolute(8.0)
	
	local_speed = 25
	
	def get_objs_and_places():
		objs = []
		places = []
		
		for place in selected_location.places.itervalues():
			if '_pos' in place.name:
				obj_name = place.name[0:place.name.index('_pos')]
				if location_objects.has_key(obj_name):
					obj = location_objects[obj_name]
					main_frame = obj['animations'][None]
					obj_image = get_image_or_similar(main_frame['directory'] + main_frame['main_image'] + '.' + location_object_ext)
					
					x = place.x + place.xsize / 2
					y = place.y + place.ysize / 2
					objs.append((obj_image, x, y))
			
			if place.to_location_name is None:
				image = im.Rect('#0B0')
			else:
				to_loc = rpg_locations.get(place.to_location_name, None)
				if to_loc and to_loc.places.has_key(place.to_place_name):
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
			
			places.append((place, image))
		
		objs.sort(key = lambda (obj_image, x, y): y)
		
		return objs, places
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('selected_location')

screen selected_location:
	image im.rect('#444'):
		size 1.0
	
	key 'w' action AddVariable('local_cam_y', -local_speed)
	key 'a' action AddVariable('local_cam_x', -local_speed)
	key 's' action AddVariable('local_cam_y', +local_speed)
	key 'd' action AddVariable('local_cam_x', +local_speed)
	
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
		if persistent.show_free and selected_location.free():
			image selected_location.free():
				size (w, h)
		
		if not persistent.hide_places:
			null:
				zoom local_k
				
				$ objs, places = get_objs_and_places()
				
				for obj_image, x, y in objs:
					image obj_image:
						pos (x, y)
						size get_image_size(obj_image)
						anchor (0.5, 1.0)
				
				for place, image in places:
					button:
						ground image
						action SetVariable('selected_place_name', place.name)
						
						pos  (place.x, place.y)
						size (max(place.xsize, 3), max(place.ysize, 3))
						
						alpha 0.5
	
	use location_props

