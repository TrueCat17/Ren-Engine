init -1001 python:
	
	class Style(Object):
		complex_props = ('pos', 'anchor', 'size', 'zoom', 'align', 'xalign', 'yalign')
		
		def __init__(self, parent = None, properties = None):
			Object.__init__(self, parent)
			if properties:
				if type(properties) is not dict:
					properties = dict(properties)
				for name, value in properties.iteritems():
					self.__setattr__(name, value)
		
		def has_key(self, attr):
			if attr in self.complex_props:
				props = get_prop_names(attr)
				for prop in props:
					if not self.has_key(prop):
						return False
				return True
			return Object.has_key(self, attr)
		def has_attr(self, attr):
			return self.has_key(attr)
		
		def get(self, attr, default_value = None):
			if attr in self.complex_props:
				if attr == 'xalign':
					return Object.get(self, 'xpos', default_value)
				if attr == 'yalign':
					return Object.get(self, 'ypos', default_value)
				
				props = get_prop_names(attr)
				return tuple(self.get(prop, default_value) for prop in props)
			return Object.get(self, attr, default_value)
		
		def __getattr__(self, attr):
			if attr in self.complex_props:
				if attr == 'xalign':
					return Object.__getattr__(self, 'xpos')
				if attr == 'yalign':
					return Object.__getattr__(self, 'ypos')
				
				props = get_prop_names(attr)
				return tuple(self.__getattr__(prop) for prop in props)
			return Object.__getattr__(self, attr)
		
		def __setattr__(self, attr, value):
			if attr in self.complex_props:
				props = get_prop_names(attr)
				if type(value) in (tuple, list):
					if len(value) != len(props):
						out_msg('Style.__setattr__', 'As <value> got %s with len %s, but expected len %s\nAttr: %s; value: %s' % (type(value), len(value), len(props), attr, value))
						return
					values = value
				else:
					values = [value] * len(props)
				for index, prop in enumerate(props):
					self.__setattr__(prop, values[index])
			else:
				Object.__setattr__(self, attr, value)
		
		def __delattr__(self, attr):
			if attr in self.complex_props:
				props = get_prop_names(attr)
				for prop in props:
					self.__delattr__(prop)
			else:
				if self.has_key(attr):
					Object.__delattr__(self, attr)
	
	
	
	style = Object()
	
	style.key = Style()
	style.key.first_delay = 0.333
	style.key.delay = 0.01
	
	style.default = Style()
	style.default.pos = 0
	style.default.anchor = 0
	style.default.size = 1.0
	style.default.zoom = 1.0
	style.default.spacing = 0
	style.default.crop = (0.0, 0.0, 1.0, 1.0)
	style.default.rotate = 0
	style.default.alpha = 1
	style.default.clipping = False
	
	style.screen = Style(style.default)
	style.screen.modal = False
	style.screen.zorder = 0
	
	style.vbox = Style(style.default)
	style.vbox.size = 0
	
	style.hbox = Style(style.vbox)
	style.null = Style(style.vbox)
	
	style.image = Style(style.default)
	style.image.size = 100
	
	style.text = Style(style.default)
	style.text.size = -1
	style.text.text_size = 20
	style.text.color = 0xFFFFFF
	style.text.outlinecolor = None
	style.text.font = 'Calibri'
	style.text.text_align = 'left' 				# left | center | right
	style.text.text_valign = 'top' 				#  top | center | bottom
	style.text.bold = False
	style.text.italic = False
	style.text.underline = False
	style.text.strikethrough = False
	
	style.textbutton = Style(style.text)
	style.textbutton.mouse = True
	style.textbutton.text_align = 'center'		# left | center | right
	style.textbutton.text_valign = 'center'		#  top | center | bottom
	style.textbutton.size = (175, 25)
	style.textbutton.text_size = 15
	style.textbutton.ground = 'images/gui/std/btn/usual.png'
	style.textbutton.hover = 'images/gui/std/btn/hover.png'
	
	style.button = Style(style.default)
	style.button.mouse = True
	style.button.size = (175, 25)
	style.button.ground = style.textbutton.ground
	style.button.hover = style.textbutton.hover
	
	style.hotspot = Style(style.default)
	style.hotspot.mouse = True
	
	style.imagemap = Style(style.default)
	style.imagemap.ground = ''
	style.imagemap.hover = ''
