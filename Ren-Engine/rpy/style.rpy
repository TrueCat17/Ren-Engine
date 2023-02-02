init -1002 python:
	
	def style__get_default_hover(hover, ground, brightness = 0.1):
		if hover:
			return hover
		return im.MatrixColor(ground, im.matrix.brightness(brightness))
	
	build_object('style')
	
	
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
				if attr == 'properties':
					for k, v in value.iteritems():
						Object.__setattr__(self, k, v)
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


init -1001:
	style key:
		first_delay 0.333
		delay       0.01
	
	style default:
		pos 0
		anchor 0
		size 1.0
		zoom 1.0
		spacing 0
		crop (0.0, 0.0, 1.0, 1.0)
		rotate 0
		alpha 1
		clipping False
		skip_mouse False
	
	style screen:
		modal False
		save  True
		zorder 0
		ignore_modal False
	
	style vbox:
		size 0
	
	style hbox is vbox
	style null is vbox
	
	style image:
		size 100
	
	style text:
		size -1
		text_size 20
		color 0xFFFFFF
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
		text_align  'center' # left | center | right
		text_valign 'center' #  top | center | bottom
		size (175, 25)
		text_size 15
		ground 'images/gui/std/btn/ground.png'
		hover  'images/gui/std/btn/hover.png'
	
	style button:
		mouse True
		size (175, 25)
		ground style.textbutton.ground
		hover  style.textbutton.hover
	
	style hotspot:
		mouse True
	
	style imagemap:
		ground ''
		hover  ''
