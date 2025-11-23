init 10 python:
	sys.setrecursionlimit(100000) # for pickling of <network> of cells
	
	set_fps(20)
	set_can_mouse_hide(False)
	
	start_screens = ['hotkeys', 'debug_screen']
	
	sc_black_bg = im.rect('#000')
	sc_white_bg = im.rect('#FFF')
	
	
	
	def sc_change_seed():
		input.ask_int(sc_set_seed, 'Seed', str(persistent.sc_seed))
	
	def sc_set_seed(value_str):
		try:
			persistent.sc_seed = int(value_str)
		except:
			notification.out('%s is not int', value_str)
	
	
	def sc_reset():
		persistent.sc_count_of_players = 4
		persistent.sc_count_of_bots = 3
		persistent.sc_difficulty_level = 0
		persistent.sc_width  = 48
		persistent.sc_height = 24
		persistent.sc_seed = -1
	
	if 'sc_seed' not in persistent:
		sc_reset()
	
	
	def sc_show_menu(do = True):
		if not do:
			return
		
		for screen in ('sc_map', 'sc_control', 'sc_info'):
			hide_screen(screen)
		show_screen('sc_main_menu')
	
	
	def sc_start():
		sc_map.count_of_players = persistent.sc_count_of_players
		sc_map.count_of_bots    = persistent.sc_count_of_bots
		sc_map.bonus_for_bots = persistent.sc_difficulty_level * 5
		
		width  = persistent.sc_width
		height = persistent.sc_height
		seed = persistent.sc_seed
		sc_map.generate(width, height, seed)
		
		for screen in ('sc_map', 'sc_control', 'sc_info'):
			show_screen(screen)
		hide_screen('sc_main_menu')
	
	show_screen('sc_main_menu')
	#sc_start()
	
	
	sc_main_menu = SimpleObject()
	sc_main_menu.bg = im.rect('#EEE')
	sc_main_menu.panel_image = im.matrix_color(sc_map.image_dir + 'panel.webp', im.matrix.colorize('#BBB', '#E0E0E0'))
	
	sc_main_menu.btns = (
		('width',  16, 80, 8),
		('height', 16, 80, 8),
		('count_of_players', 1, 4, 1),
		('count_of_bots',    0, 4, 1),
		('difficulty_level', 0, 20, 1),
	)


screen sc_main_menu:
	image sc_main_menu.bg:
		size 1.0
	
	image sc_main_menu.panel_image:
		corner_sizes -1
		size 0.7
		size_min 320
		align 0.5
	
	python:
		screen_tmp = SimpleObject()
		screen_tmp.panel_size = absolute(max(0.7 * get_stage_height(), 320))
		screen_tmp.btn_indent = get_stage_height() // 36
		screen_tmp.btn_size = style.sc_main_menu_btn.get_current('ysize')
		
		screen_tmp.container_xsize = style.sc_main_menu_btn.get_current('xsize')
		
		screen_tmp.btns_count = len(sc_main_menu.btns) + 3
		screen_tmp.btns_size = screen_tmp.btns_count * screen_tmp.btn_size + (screen_tmp.btns_count - 1) * screen_tmp.btn_indent
		
		screen_tmp.title_text_size = style.sc_main_menu_title.get_current('text_size')
		
		# (panel border + null) * 2 = (2 + 1) * 2 = 6
		screen_tmp.indent = (screen_tmp.panel_size - 6 - screen_tmp.title_text_size - screen_tmp.btns_size) / 3
	
	vbox:
		align 0.5
		spacing screen_tmp.indent
		
		null ysize 1
		
		text _('Simple Civilization'):
			style 'sc_main_menu_title'
		
		vbox:
			xalign 0.5
			spacing screen_tmp.btn_indent
			
			for name, min_value, max_value, diff in sc_main_menu.btns:
				null:
					xalign 0.5
					xsize screen_tmp.container_xsize
					
					text (_(name.replace('_', ' ').capitalize()) + ':'):
						style 'sc_main_menu_text'
					
					hbox:
						xalign 1.0
						spacing screen_tmp.btn_indent
						
						textbutton '<':
							style 'sc_main_menu_change_btn'
							action "persistent['sc_' + name] = max(min_value, persistent['sc_' + name] - diff)"
						
						text str(persistent['sc_' + name]):
							style 'sc_main_menu_text'
							xsize 0.025
						
						textbutton '>':
							style 'sc_main_menu_change_btn'
							action "persistent['sc_' + name] = min(max_value, persistent['sc_' + name] + diff)"
			
			python:
				if persistent.sc_count_of_bots > persistent.sc_count_of_players:
					persistent.sc_count_of_bots = persistent.sc_count_of_players
			
			
			textbutton ('%s: %s' % (_('Seed'), persistent.sc_seed)):
				style 'sc_main_menu_seed_btn'
				action sc_change_seed
			
			textbutton _('Reset'):
				style 'sc_main_menu_btn'
				action sc_reset
			
			textbutton _('Start'):
				style 'sc_main_menu_btn'
				action sc_start
		
		null ysize 1
	
	key 'F1' action show_screen('help')
	
	if hotkeys.shift:
		key 'RETURN' action sc_start
