init -1000 python:
	locations_path = 'images/locations/'

	preview_path = 'mods/rpg_editor/cache/'
	if not os.path.exists(preview_path):
		os.mkdir(preview_path)
	
	locations_file_path = 'mods/rpg_editor/results/locations.rpy'
	location_objects_file_path = 'mods/rpg_editor/results/objects.rpy'
	
	
	locations_times = Object()
	
	for name in os.listdir(preview_path):
		name = name[:name.rfind('.')]
		if not os.path.exists(locations_path + name):
			continue
		
		file_time = os.path.getctime(locations_path + name + '/main.' + location_ext)
		preview_time = os.path.getctime(preview_path + name + '.png')
		
		locations_times[name] = file_time, preview_time


init python:
	def register_new_locations():
		ext = location_ext
		
		locations_dirs = [i for i in os.listdir(locations_path) if not i.startswith('_') and os.path.exists(locations_path + i + '/main.' + ext)]
		locations_dirs.sort()
		
		for name in locations_dirs:
			path = locations_path + name
			main = path + '/main.' + ext
			if name not in locations:
				register_location(name, path, False, get_image_width(main), get_image_height(main))
				set_save_locations()
			
			if not os.path.exists(preview_path + name + '.png'):
				make_preview(name)
			
			file_time, preview_time = locations_times[name]			
			if preview_time < file_time:
				w, h = get_image_size(main)
				location = locations[name]
				
				if location.xsize != w or location.ysize != h:
					location.xsize, location.ysize = w, h
					set_save_locations()
				
				make_preview(name)
	
	def get_preview(name):
		return preview_path + name + '.png?' + str(locations_times[name][1])
	
	def make_preview(name):
		location = locations[name]
		w, h = location.xsize, location.ysize
		
		if location.over():
			image = im.Composite((w, h), (0, 0), location.main(), (0, 0), location.over())
		else:
			image = location.main()
		
		to_save = preview_path + name + '.png'
		im.save(image, to_save, 128 * w / max(w, h), 128 * h / max(w, h))
		
		locations_times[name] = [
			os.path.getctime(locations_path + name + '/main.' + location_ext),
			os.path.getctime(to_save)
		]
	
	def save():
		place_indent = ' ' * (len('location') - len('place'))
		exit_indent = ' ' * (len('location') - len('exit'))
		
		def get_to_side(place):
			if place.to_side is None:
				return ''
			return ', ' + ['to_back', 'to_left', 'to_right', 'to_forward'][place.to_side]
		
		
		tmp = ['init python:', '\t']
		location_names = locations.keys()
		location_names.sort()
		for location_name in location_names:
			location = locations[location_name]
			
			location_name = '"' + location_name + '"'
			path = '"' + location.directory + '"'
			is_room = str(location.is_room)
			w, h = str(location.xsize), str(location.ysize)
			
			tmp.append('\tregister_location(' + ', '.join([location_name, path, is_room, w, h]) + ')')
			
			places = location.places.keys()
			places.sort()
			for place_name in places:
				place = location.places[place_name]
				if place.side_exit is not None:
					continue
				
				place_name = '"' + place_name + '"'
				
				to_side = get_to_side(place)
				place_args = [location_name, place_name] + map(str, [place.x, place.y, place.xsize, place.ysize])
				tmp.append('\tregister_place(' + place_indent + ', '.join(place_args) + to_side + ')')
			
			for place_name in places:
				place = location.places[place_name]
				if place.side_exit is None:
					continue
				
				place_name = '"' + place_name + '"'
				
				px, py = place.x, place.y
				x, y, w, h, exit_x, exit_y, exit_w, exit_h = get_place_coords(place)
				to_side = get_to_side(place)
				
				x += px
				y += py
				exit_x += px
				exit_y += py
				
				in_place_indent = ' ' * (len(', ') + len(location_name))
				place_args = [location_name, place_name + in_place_indent] + map(str, [x, y, w, h])
				exit_args  = [location_name, place_name, location_name   ] + map(str, [exit_x, exit_y, exit_w, exit_h])
				tmp.append('\tregister_place(' + place_indent + ', '.join(place_args) + to_side + ')')
				tmp.append('\tregister_exit(' +  exit_indent  + ', '.join(exit_args)  + ')')
			
			for exit in location.exits:
				to_location_name = '"' + exit.to_location_name + '"'
				to_place_name = '"' + exit.to_place_name + '"'
				x, y = str(exit.x), str(exit.y)
				w, h = str(exit.xsize), str(exit.ysize)
				
				tmp.append('\tregister_exit(' + ', '.join([location_name, to_location_name, to_place_name, x, y, w, h]) + ')')
			
			tmp.append('\t')
		
		tmp += ['\t', '\t']
		
		for location_name in location_names:
			location = locations[location_name]
			if not location.using:
				x = y = "None"
			else:
				x, y = str(int(location.x)), str(int(location.y))
			tmp.append('\tlocations["' + location_name + '"].x, locations["' + location_name + '"].y = ' + x + ', ' + y)
		
		tmp.append('')
		
		locations_file = open(locations_file_path, 'wb')
		locations_file.writelines(map(lambda s: s + '\n', tmp))
		locations_file.close()
		
		
		
		location_objects_file = open(location_objects_file_path, 'r')
		tmp = location_objects_file.readlines()
		location_objects_file.close()
		for i in xrange(len(tmp) - 1, -1, -1):
			if tmp[i].startswith('\tregister_location_object'):
				s = 0
				while i < len(tmp):
					line = tmp[i]
					i += 1
					s += line.count('(') - line.count(')')
					if s == 0:
						break
				break
		tmp = map(lambda s: '\t' if s == '\t\n' else s.rstrip(), tmp[0:i])
		tmp.append('\t')
		
		obj_names = location_objects.keys()
		location_names = locations.keys()
		location_names.sort()
		for location_name in location_names:
			location = locations[location_name]
			
			added = False
			places = location.places.keys()
			places.sort()
			for place_name in places:
				if '_pos' in place_name:
					obj_name = place_name[0:place_name.index('_pos')]
					if obj_name in obj_names:
						added = True
						args = map(lambda s: '"' + s + '"', [location_name, place_name, obj_name])
						tmp.append('\tadd_location_object(' + ', '.join(args) + ')')
			
			if added:
				tmp.append('\t')
		
		location_objects_file = open(location_objects_file_path, 'wb')
		location_objects_file.writelines(map(lambda s: s + '\n', tmp))

