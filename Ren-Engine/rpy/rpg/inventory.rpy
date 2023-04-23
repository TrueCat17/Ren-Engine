init -1010 python:
	
	def inventory__set_size(size, obj = None):
		obj = obj or me
		if isinstance(obj, Object) and obj.inventory is None:
			obj.inventory = []
		inv = obj if type(obj) is list else obj.inventory
		
		old_size = len(inv)
		if size > old_size:
			for i in range(size - old_size):
				inv.append(['', 0])
		else:
			inventory.reduce_size(old_size - size, inv)
	
	
	def inventory__reduce_size(dsize, inv):
		new_size = len(inv) - dsize
		
		# for overflowed elements
		for index in range(dsize):
			obj_name, obj_count = inv[new_size + index]
			if not obj_name: continue
			
			loc_obj = location_objects[obj_name]
			max_count = loc_obj['max_in_inventory_cell']
			
			# move to prev cells
			for i in range(new_size):
				if not inv[i][0]:
					inv[i] = inv[new_size + index]
					obj_count = 0
				elif inv[i][0] == obj_name:
					d = min(max_count - inv[i][1], obj_count)
					if d > 0:
						inv[i][1] += d
						obj_count -= d
				
				if obj_count == 0: break
			if obj_count == 0: continue
			
			# remove_to_location
			if loc_obj['remove_to_location']:
				r = inventory.throw_radius
				for i in range(obj_count):
					dx, dy = random.randint(-r, r), random.randint(-r, r)
					add_location_object(cur_location.name, {'x': me.x + dx, 'y': me.y + dy}, obj_name)
		
		inv[-dsize:] = []
	
	
	def inventory__add(obj_name, count = 1, obj = None):
		obj = obj or me
		inv = obj if type(obj) is list else obj.inventory
		
		loc_obj = location_objects.get(obj_name, None)
		if not loc_obj:
			out_msg('inventory.add', 'Object <' + str(obj_name) + '> not registered')
			return count
		max_count = loc_obj['max_in_inventory_cell']
		if max_count == 0:
			return count
		
		for step in (1, 2): # 1 - add to existed stacks, 2 - to empty cells
			for element in inv:
				if step == 1 and element[0] != obj_name: continue
				if step == 2 and element[0]: continue
				
				if not element[0]:
					element[:] = [obj_name, 0]
				
				d = min(max_count - element[1], count)
				if d > 0:
					element[1] += d
					count -= d
					if count == 0:
						return 0
		return count
	
	def inventory__has(obj_name, count = 1, obj = None):
		obj = obj or me
		inv = obj if type(obj) is list else obj.inventory
		
		if obj_name not in location_objects:
			out_msg('inventory.has', 'Object <' + str(obj_name) + '> not registered')
			return False
		
		t = 0
		for element in inv:
			if element[0] == obj_name:
				t += element[1]
				if t >= count:
					return True
		return False
	
	def inventory__remove(obj_name, count, obj = None):
		obj = obj or me
		inv = obj if type(obj) is list else obj.inventory
		
		if obj_name not in location_objects:
			out_msg('inventory.remove', 'Object <' + str(obj_name) + '> not registered')
			return count
		
		for element in inv:
			if element[0] != obj_name: continue
			
			if element[1] > count:
				element[1] -= count
				count = 0
			else:
				count -= element[1]
				element[:] = ['', 0]
			
			if count == 0:
				break
		
		return count
	
	def inventory__change(old, new, show_on_fail):
		old_len, new_len = len(old), len(new)
		
		for i in range(min(old_len, new_len)):
			if not new[i][0] and old[i][0]:
				new[i], old[i] = old[i], new[i]
		
		auto_failed = False
		for i in range(old_len):
			obj_name, obj_count = old[i]
			if not obj_name: continue
			
			max_count = location_objects[obj_name]['max_in_inventory_cell']
			for j in range(new_len):
				obj_name_new, obj_count_new = new[j]
				if not obj_name_new:
					new[j], old[i] = old[i], new[j]
					obj_count = 0
					break
				
				if obj_name_new == obj_name:
					d = min(max_count - obj_count_new, obj_count)
					if d > 0:
						new[j][1] += d
						old[i][1] -= d
						obj_count -= d
						
						if obj_count == 0:
							old[i][0] = ''
							break
			
			if obj_count:
				auto_failed = True
		
		if auto_failed and show_on_fail:
			inventory.show(new, old)
	
	
	build_object('inventory')
	
	inventory.throw_radius = 3
	
	inventory.dress_sizes = {
		'default': 10,
	}
