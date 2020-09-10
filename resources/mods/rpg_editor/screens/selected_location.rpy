init python:
	local_cam_x, local_cam_y = get_stage_width() / 2, get_stage_height() / 2
	
	local_k = 1.0
	local_speed = 25

screen selected_location:
	image im.rect('#444'):
		size (1.0, 1.0)
	
	key 'w' action AddVariable('local_cam_y', -local_speed)
	key 'a' action AddVariable('local_cam_x', -local_speed)
	key 's' action AddVariable('local_cam_y', +local_speed)
	key 'd' action AddVariable('local_cam_x', +local_speed)
	
	key '9' action SetVariable('local_k', max(local_k - 0.25, 0.25))
	key '0' action SetVariable('local_k', min(local_k + 0.25, 8.00))
	
	python:
		selected_location = locations[selected_location_name]
		
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
			python:
				objs = []
				places = []
				
				for place_name in selected_location.places:
					place = selected_location.places[place_name]
					
					if '_pos' in place_name:
						obj_name = place_name[0:place_name.index('_pos')]
						if location_objects.has_key(obj_name):
							obj = location_objects[obj_name]
							main_frame = obj['animations'][None]
							obj_image = get_image_or_similar(main_frame['directory'] + main_frame['main_image'] + '.' + location_object_ext)
							
							x = place.x + place.xsize / 2
							y = place.y + place.ysize / 2
							objs.append((obj_image, x, y))
					
					if place.side_exit is None:
						image = im.Rect('#0B0')
					else:
						x, y, w, h, exit_x, exit_y, exit_w, exit_h = get_place_coords(place)
						image = im.Composite((place.xsize, place.ysize),
								             (   x  ,    y  ), im.Rect('#0B0', w, h),
								             (exit_x, exit_y), im.Rect('#B00', exit_w, exit_h))
					
					places.append((place, image))
				
				objs.sort(key = lambda (obj_image, x, y): y)
			
			for obj_image, x, y in objs:
				image obj_image:
					pos (int(x * local_k), int(y * local_k))
					size (int(get_image_width(obj_image) * local_k), int(get_image_height(obj_image) * local_k))
					anchor (0.5, 1.0)
			
			for place, image in places:
				button:
					ground image
					action [SetVariable('selected_place_name', place.name), SetVariable('selected_exit_num', None)]
					
					pos  (int(place.x * local_k), int(place.y * local_k))
					size (max(int(place.xsize * local_k), 3), max(int(place.ysize * local_k), 3))
					
					alpha 0.5
		
		if not persistent.hide_exits:
			for i in xrange(len(selected_location.exits)):
				python:
					exit = selected_location.exits[i]
					real_to = locations.has_key(exit.to_location_name) and locations[exit.to_location_name].places.has_key(exit.to_place_name)
					image = im.Rect('#BB0' if real_to else '#440')
				
				button:
					ground image
					action [SetVariable('selected_exit_num', i), SetVariable('selected_place_name', None)]
					
					pos  (int(exit.x * local_k),     int(exit.y * local_k))
					size (int(exit.xsize * local_k), int(exit.ysize * local_k))
					
					alpha 0.5
	
	use location_props

