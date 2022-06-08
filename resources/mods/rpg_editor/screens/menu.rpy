init:
	style menu is textbutton:
		ground im.circle('#080')
		hover  im.circle('#0B0')
		size   25

screen menu:
	zorder 1000000
	
	key 'F1' action show_help
	
	hbox:
		xalign 1.0
		
		textbutton '?':
			style 'menu'
			color 0xFFFF00
			action show_help
		
		textbutton 'X':
			style 'menu'
			color 0xFF0000
			action start_mod('main_menu')
