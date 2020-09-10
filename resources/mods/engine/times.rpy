init -1001 python:
	
	times = {}
	def make_time(name, **kwargs):
		def func():
			times['next_name'] = name
			
			if not has_screen('location') or not times.has_key('current_name') or not cur_location:
				set_time_direct()
			else:
				place = {
					'x': me.x,
					'y': me.y,
					'width': 0,
					'height': 0
				}
				set_location(cur_location.name, place)
		
		times[name] = {
			'text':     kwargs.get('text',     'The ' + name + ' is coming'),
			'sprite':   kwargs.get('sprite',   (255, 255, 255)),
			'location': kwargs.get('location', (255, 255, 255))
		}
		globals()[name + '_time'] = func
	
	def set_time_direct():
		name = times['current_name'] = times['next_name']
		times['next_name'] = None
		
		global sprite_time_rgb, location_time_rgb
		sprite_time_rgb   = times[name]['sprite']
		location_time_rgb = times[name]['location']
	
	def set_time_text(name, text):
		times[name]['text'] = text
	
	make_time('day') # def day_time
	
	day_time()

