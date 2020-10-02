init -1001 python:
	Style = Object # class
	
	
	# Important!
	# In style.* necessarily:
	#   use x- and y- property separately
	#   do not use *align
	
	style = Object()
	
	style.key = Style()
	style.key.first_delay = 0.333
	style.key.delay = 0.01
	
	style.default = Style()
	style.default.xpos = 0
	style.default.ypos = 0
	style.default.xanchor = 0
	style.default.yanchor = 0
	style.default.xsize = 1.0
	style.default.ysize = 1.0
	style.default.xzoom = 1.0
	style.default.yzoom = 1.0
	style.default.spacing = 0
	style.default.crop = (0.0, 0.0, 1.0, 1.0)
	style.default.rotate = 0
	style.default.alpha = 1
	style.default.clipping = False
	
	style.screen = Style(style.default)
	style.screen.modal = False
	style.screen.zorder = 0
	
	style.vbox = Style(style.default)
	style.vbox.xsize = 0
	style.vbox.ysize = 0
	
	style.hbox = Style(style.vbox)
	style.null = Style(style.vbox)
	
	style.image = Style(style.default)
	style.image.xsize = 100
	style.image.ysize = 100
	
	style.text = Style(style.default)
	style.text.xsize = -1
	style.text.ysize = -1
	style.text.text_size = 20
	style.text.color = 0xFFFFFF
	style.text.outlinecolor = None
	style.text.font = 'Calibri'
	style.text.text_align = 'left' 				# left | center | right
	style.text.text_valign = 'top' 				# top  | center |  down
	style.text.bold = False
	style.text.italic = False
	style.text.underline = False
	style.text.strikethrough = False
	
	style.textbutton = Style(style.text)
	style.textbutton.mouse = True
	style.textbutton.text_align = 'center'		# left | center | right
	style.textbutton.text_valign = 'center'		# top  | center |  down
	style.textbutton.xsize = 175
	style.textbutton.ysize = 25
	style.textbutton.text_size = 15
	style.textbutton.ground = 'images/gui/std/btn/usual.png'
	style.textbutton.hover = 'images/gui/std/btn/hover.png'
	
	style.button = Style(style.default)
	style.button.mouse = True
	style.button.xsize = 175
	style.button.ysize = 25
	style.button.ground = style.textbutton.ground
	style.button.hover = style.textbutton.hover
	
	style.hotspot = Style(style.default)
	style.hotspot.mouse = True
	
	style.imagemap = Style(style.default)
	style.imagemap.ground = ''
	style.imagemap.hover = ''
