init python:
	style.menu = Style(style.textbutton)
	style.menu.ground = im.circle('#080')
	style.menu.hover  = im.circle('#0B0')
	style.menu.xsize = style.menu.ysize = 25

screen menu:
	zorder 1000000
	
	key 'F1' action show_help
	
	hbox:
		xalign 1.0
		
		textbutton '?':
			style menu
			color 0xFFFF00
			action show_help
		
		textbutton 'X':
			style menu
			color 0xFF0000
			action start_mod('main_menu')
