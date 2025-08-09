init -1002 python:
	
	def style__get_default_hover(hover, ground, brightness = 0.1):
		if hover:
			return hover
		return im.matrix_color(ground, im.matrix.brightness(brightness))
	
	build_object('style')
	
	
	screen_lang_complex_props = ('pos', 'anchor', 'size', 'size_min', 'size_max', 'zoom', 'align', 'xalign', 'yalign')
	
	class Style(Object):
		def __init__(self, parent = None, properties = None):
			Object.__init__(self, parent)
			if properties:
				if type(properties) is not dict:
					properties = dict(properties)
				for name, value in properties.items():
					self.__setattr__(name, value)
		
		def __contains__(self, key):
			if key in screen_lang_complex_props:
				props = get_prop_names(key)
				for prop in props:
					if prop not in self:
						return False
				return True
			return Object.__contains__(self, key)
		
		def get(self, attr, default_value = None):
			if attr in screen_lang_complex_props:
				if attr == 'xalign':
					return Object.get(self, 'xpos', default_value)
				if attr == 'yalign':
					return Object.get(self, 'ypos', default_value)
				
				props = get_prop_names(attr)
				return tuple(self.get(prop, default_value) for prop in props)
			return Object.get(self, attr, default_value)
		
		def __getattr__(self, attr):
			if attr in screen_lang_complex_props:
				if attr == 'xalign':
					return Object.__getattr__(self, 'xpos')
				if attr == 'yalign':
					return Object.__getattr__(self, 'ypos')
				
				props = get_prop_names(attr)
				return tuple(self.__getattr__(prop) for prop in props)
			return Object.__getattr__(self, attr)
		
		def __setattr__(self, attr, value):
			if attr in screen_lang_complex_props:
				props = get_prop_names(attr)
				if type(value) in (tuple, list):
					if len(value) != len(props):
						msg = (
							'As <value> got %s with len %s, but expected len %s\n'
							'Attr: %s; value: %s'
						)
						params = type(value), len(value), len(props), attr, value
						out_msg('Style.__setattr__', msg, *params)
						return
					values = value
				else:
					values = [value] * len(props)
				for index, prop in enumerate(props):
					self.__setattr__(prop, values[index])
			else:
				if attr == 'properties':
					for k, v in value.items():
						Object.__setattr__(self, k, v)
				else:
					Object.__setattr__(self, attr, value)
		
		def __delattr__(self, attr):
			if attr in screen_lang_complex_props:
				props = get_prop_names(attr)
				for prop in props:
					self.__delattr__(prop)
			else:
				if attr in self:
					Object.__delattr__(self, attr)
		
		def get_current(self, prop, relative = None):
			now = get_game_time()
			if self[prop + '_last_update'] != now:
				self[prop + '_last_update'] = now
				
				if relative is None:
					if prop.startswith('x'):
						relative = get_stage_width()
					else:
						relative = get_stage_height()
						if prop not in ('text_size', 'hover_text_size') and not prop.startswith('y'):
							out_msg('Style.get_current', 'Set <relative> param for unusual prop <%s>', prop)
				
				value = get_absolute(self[prop], relative)
				value_min = self[prop + '_min']
				if value_min is not None and value_min > 0:
					value = max(value, get_absolute(value_min, relative))
				value_max = self[prop + '_max']
				if value_max is not None and value_max > 0:
					value = min(value, get_absolute(value_max, relative))
				
				self['cur_' + prop] = value
			return self['cur_' + prop]
		
		def get_resized_image(self, prop, width, height):
			key = ('cur_' + prop, width, height)
			key_last_update = (prop + '_last_update', width, height)
			now = get_game_time()
			if self[key_last_update] != now:
				self[key_last_update] = now
				
				xsize = width  or self.get_current('xsize', get_stage_width())
				ysize = height or self.get_current('ysize', get_stage_height())
				
				self[key] = im.scale_without_borders(self[prop], xsize, ysize, self.corner_sizes)
			return self[key]
		
		def get_ground(self, width = None, height = None):
			return self.get_resized_image('ground', width, height)
		def get_hover(self, width = None, height = None):
			return self.get_resized_image('hover', width, height)


init -1001:
	style default:
		pos 0
		anchor 0
		size 1.0
		size_min -1
		size_max -1
		zoom 1.0
		spacing 0
		spacing_min -1
		spacing_max -1
		crop (0.0, 0.0, 1.0, 1.0)
		rotate 0
		alpha 1
		clipping False
		skip_mouse False
		corner_sizes 0
	
	style key:
		first_delay 0.333
		delay       0.01
	
	style screen:
		modal False
		save  True
		zorder 0
		ignore_modal False
	
	style vbox is screen:
		size 0
	style hbox is vbox
	
	style null:
		size 0
	
	style image:
		size 100
	
	style text:
		size -1
		text_size 20
		text_size_min -1
		text_size_max -1
		color '#FFF'
		outlinecolor None
		font 'Calibri'
		text_align 'left'    # left | center | right
		text_valign 'top'    #  top | center | bottom
		bold          False
		italic        False
		underline     False
		strikethrough False
	
	style textbutton is text:
		mouse True
		selected False
		text_align  'center' # left | center | right
		text_valign 'center' #  top | center | bottom
		size (175, 25)
		text_size 15
		ground 'images/gui/std/btn/ground.png'
		hover  'images/gui/std/btn/hover.png'
		corner_sizes -1
	
	style button:
		mouse True
		selected False
		size (175, 25)
		ground style.textbutton.ground
		hover  style.textbutton.hover
		corner_sizes -1
	
	style hotspot:
		mouse True
		selected False
	
	style imagemap:
		ground ''
		hover  ''
