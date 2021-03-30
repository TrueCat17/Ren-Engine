init -100 python:
	
	inventory_size = 0
	inventory = []
	def set_inventory_size(count):
		global inventory_size, inventory
		if inventory_size < count:
			inventory += [None] * (count - inventory_size)
		else:
			reduce_inventory_size(inventory_size - count)
		inventory_size = count
		
		global inventory_row, inventory_full_rows, inventory_last_row
		inventory_row = min(10, inventory_size)
		inventory_full_rows = int(inventory_size / inventory_row)
		inventory_last_row = inventory_size % inventory_row
		
		global inventory_xsize, inventory_ysize
		inventory_xsize = (inventory_cell_size + inventory_cell_spacing) * inventory_row + inventory_cell_spacing
		inventory_ysize = (inventory_cell_size + inventory_cell_spacing) * (inventory_full_rows + (1 if inventory_last_row else 0)) + inventory_cell_spacing + 100
	
	set_inventory_size(30)
	
	
	def reduce_inventory_size(reduce_count):
		global inventory
		new_size = inventory_size - reduce_count
		
		# for overflowed elements
		for index in xrange(reduce_count):
			element = inventory[new_size + index]
			if element is None: continue
			
			obj_name, count = element
			obj = location_objects[obj_name]
			max_count = obj['max_in_inventory_cell']
			
			# move to prev cells
			for i in xrange(size):
				if inventory[i] is None:
					inventory[i] = inventory[index]
					count = 0
				
				while max_count != inventory[i][1] and count != 0:
					count -= 1
					inventory[i][1] += 1
				
				if count == 0: break
			if count == 0: continue
			
			# remove_to_location
			if obj['remove_to_location']:
				for i in xrange(count):
					dx, dy = random.randint(-3, 3), random.randint(-3, 3)
					add_location_object(cur_location.name, {'x': me.x + dx, 'y': me.y + dy}, obj_name)
		
		inventory = inventory[:-reduce_count]
	
	
	def add_to_inventory(obj_name, count):
		obj = location_objects.get(obj_name, None)
		if not obj:
			out_msg('add_to_inventory', 'Object <' + obj_name + '> not registered')
			return count
		
		while count > 0:
			index = -1
			for i in xrange(inventory_size):
				element = inventory[i]
				if element is not None and element[0] == obj_name and element[1] < obj['max_in_inventory_cell']:
					index = i
					break
			else:
				for i in xrange(inventory_size):
					if inventory[i] is None:
						index = i
						break
			
			if index == -1:
				break
			
			if inventory[index] is None:
				inventory[index] = [obj_name, 0]
			
			element = inventory[index]
			d = min(obj['max_in_inventory_cell'] - element[1], count)
			
			element[1] += d
			count -= d
		
		return count
	
	def has_in_inventory(obj_name, count = 1):
		if not location_objects.has_key(obj_name):
			out_msg('has_in_inventory', 'Object <' + obj_name + '> not registered')
			return False
		
		t = 0
		for element in inventory:
			if element and element[0] == obj_name:
				t += element[1]
				if t >= count:
					return True
		return False
	
	def remove_from_inventory(obj_name, count):
		obj = location_objects.get(obj_name, None)
		if not obj:
			out_msg('remove_from_inventory', 'Object <' + obj_name + '> not registered')
			return count
		
		while count > 0:
			index = -1
			for i in xrange(inventory_size):
				element = inventory[i]
				if element and element[0] == obj_name:
					index = i
					break
			else:
				break
			
			element = inventory[index]
			if element[1] > count:
				element[1] -= count
			else:
				count -= element[1]
				inventory[index] = None
		
		return count
		
	
