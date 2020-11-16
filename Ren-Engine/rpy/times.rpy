init -1001 python:
	
	times = {}
	
	def change_time(name):
		times['next_name'] = name
		
		if not has_screen('location') or not times.has_key('current_name') or not cur_location:
			set_time_direct()
		else:
			place = {
				'x': me.x,
				'y': me.y,
				'width': 0,
				'height': 0,
			}
			set_location(cur_location.name, place)
	
	def make_time(name, **kwargs):
		globals()[name + '_time'] = Function(change_time, name)
		
		times[name] = {
			'sprite':   kwargs.get('sprite',   (255, 255, 255)),
			'location': kwargs.get('location', (255, 255, 255))
		}
	
	def set_time_direct():
		name = times['current_name'] = times['next_name']
		times['next_name'] = None
		
		global sprite_time_rgb, location_time_rgb
		sprite_time_rgb   = times[name]['sprite']
		location_time_rgb = times[name]['location']
	
	
	make_time('day') # def day_time
	make_time('night',  sprite=(160, 200, 210), location=(140, 180, 210)) # night_time
	make_time('sunset', sprite=(240, 210, 255), location=(240, 210, 255)) # sunset_time
	
	day_time()
	
