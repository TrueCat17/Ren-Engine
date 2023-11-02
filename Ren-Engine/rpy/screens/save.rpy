init python:
	hotkeys.disable_key_on_screens['ESCAPE'].append('save')

screen save:
	zorder 10001
	modal  True
	save   False
	
	image gui.bg('save_bg'):
		size 1.0
		
		if gui.enable_title:
			text _('Save'):
				style style.save_menu_title or style.menu_title
		
		use slots('save')

