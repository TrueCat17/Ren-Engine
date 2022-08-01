init python:
	hotkeys.disable_key_on_screens['ESCAPE'].append('save')

screen save:
	zorder 10001
	modal  True
	save   False
	
	image gui.bg('save_bg'):
		size 1.0
		
		text _('Save'):
			align (0.5, 0.02)
			
			font         gui.title_text_font
			color        gui.get_int('title_text_color')
			outlinecolor gui.get_int('title_text_outlinecolor')
			text_size gui.get_int('title_text_size')
		
		use slots('save')
		
		textbutton _('Return'):
			ground gui.bg('button_ground')
			hover  gui.bg('button_ground')
			xsize gui.get_int('button_width')
			ysize gui.get_int('button_height')
			font         gui.button_text_font
			color        gui.get_int('button_text_color')
			outlinecolor gui.get_int('button_text_outlinecolor')
			text_size   gui.get_int('button_text_size')
			text_align  gui.button_text_xalign
			
			align (0.95, 0.95)
			action HideMenu('save')
	
	key 'ESCAPE' action HideMenu('save')

