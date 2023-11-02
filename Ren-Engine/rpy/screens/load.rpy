init python:
	hotkeys.disable_key_on_screens['ESCAPE'].append('load')

screen load:
	zorder 10001
	modal  True
	
	image gui.bg('load_bg'):
		size 1.0
		
		if gui.enable_title:
			text _('Load'):
				style style.load_menu_title or style.menu_title
		
		use slots('load')

