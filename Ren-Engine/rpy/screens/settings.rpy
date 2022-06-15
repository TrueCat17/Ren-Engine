init -1 python:
	settings_background = gui + 'menu/main/back.png'
	
	settings_usual_btn = style.textbutton.ground
	settings_selected_btn = im.matrix_color(settings_usual_btn, im.matrix.contrast(2.0))
	
	k = get_from_hard_config("window_w_div_h", float)
	settings_resolutions = tuple((i, int(i/k)) for i in (640, 960, 1200, 1366, 1920))
	
	settings_langs = renpy.known_languages()
	
	settings_show_mods = False
	settings_tab = 'Screen'
	
	settings_autosave_times = (0.5, 1, 2, 3, 5, 7, 10, 15, 0)
	def settings_prev_autosave_time():
		autosave = config.autosave / 60.0
		if autosave <= 0:
			config.autosave = int(settings_autosave_times[-2] * 60)
		else:
			i = 0
			while i < len(settings_autosave_times) - 1 and autosave > settings_autosave_times[i]:
				i += 1
			config.autosave = int(settings_autosave_times[max(i - 1, 0)] * 60)
	def settings_next_autosave_time():
		autosave = config.autosave / 60.0
		if autosave > 0:
			i = 0
			while i < len(settings_autosave_times) - 1 and autosave > settings_autosave_times[i]:
				i += 1
			config.autosave = int(settings_autosave_times[i + 1] * 60)
	
	
	def settings_add_text_cps(d):
		show_all_text = config.text_cps > 100000
		text_cps = in_bounds((config.text_cps % 100000) + d, 20, 220)
		config.text_cps = (100000 if show_all_text else 0) + text_cps
	
	def settings_set_text_cps_on(v):
		config.text_cps = (100000 if v else 0) + (config.text_cps % 100000)


screen settings:
	zorder 10001
	modal  True
	
	if not has_screen('pause') and not has_screen('choose_menu'):
		use hotkeys
	
	image settings_background:
		size (1.0, 1.0)
	
	text ('{outlinecolor=0}' + _('Settings')):
		align (0.5, 0.02)
		
		color 0xFFFFFF
		text_size get_stage_height() / 10
	
	if not checkboxes_inited:
		$ init_checkboxes()
	
	
	if settings_show_mods:
		vbox:
			align 0.5
			spacing 10
			
			for name, dir_name in get_mods():
				textbutton name:
					xalign 0.5
					action start_mod(dir_name)
	else:
		$ settings_menu_size = 150
		$ settings_menu_xpos = int(get_stage_width() * 0.05)
		vbox:
			anchor (0, 0.5)
			pos (settings_menu_xpos, 0.5)
			spacing 10
			
			textbutton _('Screen')    xsize settings_menu_size action SetVariable('settings_tab', 'Screen')
			textbutton _('Sounds')    xsize settings_menu_size action SetVariable('settings_tab', 'Sounds')
			textbutton _('Other')     xsize settings_menu_size action SetVariable('settings_tab', 'Other')
			if len(settings_langs) > 1:
				textbutton 'Language' xsize settings_menu_size action SetVariable('settings_tab', 'Language')  # without translation!
		
		
		vbox:
			xpos settings_menu_xpos + settings_menu_size
			xsize get_stage_width() - (settings_menu_xpos + settings_menu_size)
			yalign 0.5
			spacing 10
			
			
			if settings_tab == 'Screen':
				hbox:
					xalign 0.5
					spacing 15
					
					$ is_fullscreen = get_from_hard_config('window_fullscreen', bool)
					button:
						ground (checkbox_yes if is_fullscreen else checkbox_no)
						action set_fullscreen(not is_fullscreen)
						size (25, 25)
					text (_('Fullscreen') + ' (F11)'):
						color 0
						text_size 25
				
				null ysize 5
				
				text (_('Resolution') + ':'):
					xalign 0.5
					color 0
					
				for i in (0, 1):
					hbox:
						xalign 0.5
						spacing 10
						
						for w, h in settings_resolutions[i * 3 : (i + 1) * 3]:
							textbutton (str(w) + 'x' + str(h)):
								xsize 100
								ground (settings_selected_btn if (w, h) == get_stage_size() else settings_usual_btn)
								action set_stage_size(w, h)
			
			elif settings_tab == 'Sounds':
				text _('Volume'):
					xalign 0.5
					color 0
				
				for i in xrange(len(std_mixers)):
					$ mixer, mixer_name = std_mixers[i], std_mixers_names[i]
					hbox:
						xalign 0.5
						spacing 5
						
						text (_(mixer_name) + ':'):
							color 0
							yalign 0.5
							xsize 110
						
						textbutton '-':
							size (25, 25)
							action renpy.music.add_mixer_volume(-0.1, mixer)
						image im.bar(config[mixer + '_volume']):
							size (min(300, get_stage_width() / 3), 25)
						textbutton '+':
							size (25, 25)
							action renpy.music.add_mixer_volume(+0.1, mixer)
			
			elif settings_tab == 'Other':
				hbox:
					xalign 0.5
					spacing 15
					
					$ show_all_text = config.text_cps > 100000
					button:
						ground (checkbox_yes if show_all_text else checkbox_no)
						action settings_set_text_cps_on(not show_all_text)
						size (25, 25)
					text _('Show all text at once'):
						color 0
						text_size 25
				
				text _('Text display speed'):
					xalign 0.5
					color 0
				hbox:
					xalign 0.5
					spacing 5
					
					textbutton '-':
						size (25, 25)
						action settings_add_text_cps(-20)
					image im.bar(((config.text_cps % 100000) - 20) / 200.0):
						size (300, 25)
					textbutton '+':
						size (25, 25)
						action settings_add_text_cps(+20)
				
				null size 5
				
				
				text _('Autosave'):
					xalign 0.5
					color 0
					text_size 15
				hbox:
					xalign 0.5
					spacing 5
					
					textbutton '<-':
						size (25, 25)
						action settings_prev_autosave_time
					text ((str(config.autosave / 60.0) + ' ' + _('minutes')) if config.autosave > 0 else _('Disabled')):
						xsize 300
						text_align 'center'
						color 0
						text_size 25
					textbutton '->':
						size (25, 25)
						action settings_next_autosave_time
				
				
				null size 5
				
				hbox:
					xalign 0.5
					spacing 15
					
					button:
						ground (checkbox_yes if config.debug_screen_visible_mode else checkbox_no)
						action debug_screen.toggle_fps
						size (25, 25)
					text (_('Show FPS') + ' (F3)'):
						color 0
						text_size 25
				
				hbox:
					xalign 0.5
					spacing 15
					
					button:
						ground (checkbox_yes if not config.shift_is_run else checkbox_no)
						action SetDict(config, 'shift_is_run', not config.shift_is_run)
						size (25, 25)
					text _('Usual moving - run'):
						color 0
						text_size 25
			
			elif settings_tab == 'Language':
				for lang in settings_langs:
					textbutton lang xalign 0.5 xsize 150 action renpy.change_language(lang)
	
	
	textbutton _('Settings' if settings_show_mods else 'Mods'):
		align (0.05, 0.95)
		action SetVariable('settings_show_mods', not settings_show_mods)
	
	textbutton _('Return'):
		align (0.95, 0.95)
		action HideMenu('settings')
	key 'ESCAPE' action HideMenu('settings')

