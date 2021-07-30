init -1000 python:
	locations_path = 'images/locations/'

	preview_path = '../var/rpg_editor/cache/'
	if not os.path.exists(preview_path):
		os.makedirs(preview_path)
	
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
			if name not in rpg_locations:
				register_location(name, path, False, get_image_width(main), get_image_height(main))
			
			if not os.path.exists(preview_path + name + '.png'):
				make_preview(name)
			
			file_time, preview_time = locations_times[name]			
			if preview_time < file_time:
				w, h = get_image_size(main)
				location = rpg_locations[name]
				
				if location.xsize != w or location.ysize != h:
					location.xsize, location.ysize = w, h
					set_save_locations()
				
				make_preview(name)
	
	def get_preview(name):
		return preview_path + name + '.png?' + str(locations_times[name][1])
	
	def make_preview(name):
		location = rpg_locations[name]
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
		
		def get_place_to(place):
			if place.exit_side:
				exit_side = '"' + place.exit_side + '"'
			else:
				exit_side = "None"
			
			to_loc = place.to_location_name or ''
			to_place = place.to_place_name or ''
			res = '%s, "%s", "%s"' % (exit_side, to_loc, to_place)
			if place.to_side is not None:
				to_side = ['to_back', 'to_left', 'to_right', 'to_forward'][place.to_side]
				res += ', ' + to_side
			return ', to=[' + res + ']'
		
		
		tmp = ['init python:', '\t']
		location_names = rpg_locations.keys()
		location_names.sort()
		for location_name in location_names:
			location = rpg_locations[location_name]
			if location.x is None:
				continue
			
			location_name = '"' + location_name + '"'
			path = '"' + location.directory + '"'
			is_room = str(location.is_room)
			w, h = str(location.xsize), str(location.ysize)
			
			tmp.append('\tregister_location(' + ', '.join([location_name, path, is_room, w, h]) + ')')
			
			places = location.places.keys()
			places.sort()
			for place_name in places:
				place = location.places[place_name]
				if place.to_location_name:
					continue
				
				place_name = '"' + place_name + '"'
				place_args = [location_name, place_name] + map(str, [place.x, place.y, place.xsize, place.ysize])
				tmp.append('\tregister_place(' + place_indent + ', '.join(place_args) + ')')
			
			for place_name in places:
				place = location.places[place_name]
				if not place.to_location_name:
					continue
				
				place_name = '"' + place_name + '"'
				place_args = [location_name, place_name] + map(str, [place.x, place.y, place.xsize, place.ysize])
				tmp.append('\tregister_place(' + place_indent + ', '.join(place_args) + get_place_to(place) + ')')
			
			tmp.append('\t')
		
		tmp += ['\t', '\t']
		
		for location_name in location_names:
			location = rpg_locations[location_name]
			if location.x is None:
				continue
			
			x, y = str(int(location.x)), str(int(location.y))
			tmp.append('\trpg_locations["' + location_name + '"].x, rpg_locations["' + location_name + '"].y = ' + x + ', ' + y)
		
		tmp.append('')
		
		locations_file = open(locations_file_path, 'wb')
		locations_file.writelines(map(lambda s: s + '\n', tmp))
		locations_file.close()
		
		
		
		if not location_objects:
			return
		
		location_objects_file = open(location_objects_file_path, 'rb')
		tmp = location_objects_file.readlines()
		location_objects_file.close()
		for i in xrange(len(tmp) - 1, -1, -1):
			if not tmp[i].startswith('\tregister_location_object'): continue
			
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
		for location_name in location_names:
			location = rpg_locations[location_name]
			
			added = False
			places = location.places.keys()
			places.sort()
			for place_name in places:
				if '_pos' not in place_name: continue
				
				obj_name = place_name[0:place_name.index('_pos')]
				if obj_name not in obj_names: continue
				
				added = True
				args = map(lambda s: '"' + s + '"', [location_name, place_name, obj_name])
				tmp.append('\tadd_location_object(' + ', '.join(args) + ')')
			
			if added:
				tmp.append('\t')
		
		location_objects_file = open(location_objects_file_path, 'wb')
		location_objects_file.writelines(map(lambda s: s + '\n', tmp))

