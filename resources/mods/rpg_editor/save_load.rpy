init -1000 python:
	path_to_locations = 'images/locations/'
	
	preview_path = '../var/rpg_editor/cache/'
	os.makedirs(preview_path, exist_ok = True)
	
	file_with_locations = 'mods/rpg_editor/results/locations.rpy'
	file_with_objects   = 'mods/rpg_editor/results/objects.rpy'
	
	
	location_times = Object()
	
	def get_location_main(name):
		d = path_to_locations + name + '/'
		for f in os.listdir(d):
			if f.startswith('main.'):
				return d + f
		return None
	
	def set_location_times():
		for name in os.listdir(preview_path):
			name = name[:name.rfind('.')]
			if not os.path.exists(path_to_locations + name): continue
			
			main = get_location_main(name)
			if not main: continue
			
			file_time = os.path.getctime(main)
			preview_time = os.path.getctime(preview_path + name + '.png')
			
			location_times[name] = file_time, preview_time
	
	set_location_times()


init python:
	def register_new_locations():
		locations = []
		for name in os.listdir(path_to_locations):
			if name.startswith('_'): continue
			
			main = get_location_main(name)
			if main:
				locations.append((name, main))
		
		locations.sort()
		
		for name, main in locations:
			if name not in rpg_locations:
				register_location(name, path_to_locations + name, False, get_image_width(main), get_image_height(main))
			
			if not os.path.exists(preview_path + name + '.png'):
				make_preview(name)
			
			file_time, preview_time = location_times[name]
			if preview_time < file_time:
				w, h = get_image_size(main)
				location = rpg_locations[name]
				
				if location.xsize != w or location.ysize != h:
					location.xsize, location.ysize = w, h
					set_save_locations()
				
				make_preview(name)
	
	def get_preview(name):
		return preview_path + name + '.png?' + str(location_times[name][1])
	
	def make_preview(name):
		location = rpg_locations[name]
		w, h = location.xsize, location.ysize
		
		if location.over():
			image = im.composite((w, h), (0, 0), location.main(), (0, 0), location.over())
		else:
			image = location.main()
		
		to_save = preview_path + name + '.png'
		im.save(image, to_save, 128 * w / max(w, h), 128 * h / max(w, h))
		
		location_times[name] = [
			os.path.getctime(get_location_main(name)),
			os.path.getctime(to_save)
		]
	
	def save():
		place_indent = ' ' * (len('location') - len('place'))
		
		def get_place_to(place):
			if place.exit_side:
				exit_side = '"' + place.exit_side + '"'
			else:
				exit_side = 'None'
			
			to_loc = place.to_location_name or ''
			to_place = place.to_place_name or ''
			res = '%s, "%s", "%s"' % (exit_side, to_loc, to_place)
			if place.to_side is not None:
				to_side = ('to_back', 'to_left', 'to_right', 'to_forward')[place.to_side]
				res += ', ' + to_side
			return ', to=[' + res + ']'
		
		
		tmp = ['init python:', '\t']
		location_names = sorted(rpg_locations.keys())
		for location_name in location_names:
			location = rpg_locations[location_name]
			if location.x is None: continue
			
			params = '"%s", "%s", %s, %i, %i' % (location_name, location.directory, location.is_room, location.xsize, location.ysize)
			tmp.append('\tregister_location(' + params + ')')
			
			places = sorted(location.places.keys())
			for place_name in places:
				place = location.places[place_name]
				if place.to_location_name: continue
				
				params = '"%s", "%s", %i, %i, %i, %i' % (location_name, place_name, place.x, place.y, place.xsize, place.ysize)
				tmp.append('\tregister_place(' + place_indent + params + ')')
			
			for place_name in places:
				place = location.places[place_name]
				if not place.to_location_name: continue
				
				params = '"%s", "%s", %i, %i, %i, %i' % (location_name, place_name, place.x, place.y, place.xsize, place.ysize)
				tmp.append('\tregister_place(' + place_indent + params + get_place_to(place) + ')')
			
			tmp.append('\t')
		
		tmp += ['\t', '\t']
		
		for location_name in location_names:
			location = rpg_locations[location_name]
			if location.x is None:
				continue
			
			tmp.append('\trpg_locations["%s"].x, rpg_locations["%s"].y = %i, %i' % (location_name, location_name, location.x, location.y))
		
		with open(file_with_locations, 'wb') as f:
			for s in tmp:
				f.write((s + '\n').encode('utf-8'))
		
		
		
		if not location_objects:
			return
		
		with open(file_with_objects, 'rb') as f:
			tmp = f.read().decode('utf-8').split('\n')
		
		for i in range(len(tmp) - 1, -1, -1):
			if not tmp[i].startswith('\tregister_location_object'): continue
			
			s = 0
			while i < len(tmp):
				line = tmp[i]
				i += 1
				s += line.count('(') - line.count(')')
				if s == 0:
					break
			break
		tmp = [(s.rstrip() or '\t') for s in tmp[:i]]
		tmp.append('\t')
		
		for location_name in location_names:
			location = rpg_locations[location_name]
			
			places = sorted(location.places.keys())
			for place_name in places:
				i = place_name.find('_pos')
				if i == -1: continue
				
				obj_name = place_name[:i]
				if obj_name not in location_objects: continue
				
				params = '"%s", "%s", "%s"' % (location_name, place_name, obj_name)
				tmp.append('\tadd_location_object(' + params + ')')
		
		with open(file_with_objects, 'wb') as f:
			for s in tmp:
				f.write((s + '\n').encode('utf-8'))
