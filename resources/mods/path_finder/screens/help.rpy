init python:
	help_text = ''
	help_groups = []
	
	help_inited = False
	def init_help():
		global help_inited
		help_inited = True
		
		f = open(_('mods/path_finder/readme.txt'))
		content = f.read().strip().replace('\t', '  ')
		f.close()
		
		groups = content.split('\n\n\n')
		for group in groups:
			new_str_pos = group.index('\n')
			
			name = group[0:new_str_pos]
			text = group[new_str_pos+1:]
			
			help_groups.append((name, text))
		
		global help_text
		if help_groups:
			help_text = help_groups[0][1]
	
	def show_help():
		if not help_inited:
			init_help()
		show_screen('help')
	
	def close_help():
		hide_screen('help')
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('help')


screen help:
	zorder 1000001
	modal True
	
	key 'ESCAPE' action close_help
	
	button:
		ground im.rect('#000')
		hover  im.rect('#000')
		size  1.0
		alpha 0.3
		mouse False
		action close_help
	
	image im.rect('#000'):
		align (0.5, 0.5)
		xsize int(get_stage_width() * 0.8 + 10)
		ysize int(get_stage_height() * 0.8 + 10)
	
	image im.rect('#FFF'):
		align (0.5, 0.5)
		size (0.8, 0.8)
		
		vbox:
			ypos 15
			xsize 0.8
			spacing 15
			
			hbox:
				xalign 0.5
				ypos 10
				spacing 10
				
				for name, text in help_groups:
					textbutton name:
						action SetVariable('help_text', text)
				
			text help_text:
				xpos 0.05
				ypos 0.05
				xsize 0.7
				color 0x000000
				font 'Consola'
