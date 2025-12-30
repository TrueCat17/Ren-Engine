init -10 python:
	
	def demos__init():
		demos.utils = []
		demos.demos = []
		
		for mod_name, dir_name in get_mods():
			picture = 'images/mod_previews/%s.webp' % dir_name
			if not os.path.exists(picture):
				picture = demos.picture_place_holder
			
			array = demos['utils' if dir_name in demos.util_names else 'demos']
			array.append((mod_name, dir_name, picture))
	
	
	def demos__update():
		array = demos[common_screen.name]
		
		demos.page = persistent.demos_page if common_screen.name == 'demos' else 0
		
		btn_count = 8
		demos.page_count = math.ceil(len(array) / btn_count)
		demos.page = in_bounds(demos.page, 0, demos.page_count - 1)
		demos.page_text = '%s/%s' % (demos.page + 1, demos.page_count)
		
		start_index = demos.page * btn_count
		end_index = start_index + btn_count
		page_buttons = array[start_index : end_index]
		
		page_buttons.extend([None] * (btn_count - len(page_buttons)))
		
		demos.page_buttons_1 = page_buttons[:btn_count // 2]
		demos.page_buttons_2 = page_buttons[btn_count // 2:]
	
	
	def demos__show():
		black_cover.start()
		set_timeout(ShowScreen('demos'), black_cover.appearance_time)
		common_screen.show_time = get_game_time() + black_cover.appearance_time * 2
	
	def demos__hide():
		black_cover.start()
		set_timeout(HideScreen(common_screen.name), black_cover.appearance_time)
	
	
	build_object('demos')
	demos.util_names = ('sprite_moving', 'rpg_editor')
	demos.picture_place_holder = im.rect('#888')
	persistent.setdefault('demos_page', 0)
	
	demos.slot_ysize = 0.1405
	
	signals.add('language', demos.init)
	
	set_timeout(Eval("load_image(gui.bg('main_bg'))"), 0.8)


init 100 python:
	if get_current_mod() == 'main_menu' and get_current_mod_index() != 0:
		common_screen.name = persistent.common_screen_tab
		if common_screen.name == 'save':
			common_screen.name = 'load'
		if common_screen.name in ('demos', 'utils', 'load'):
			show_screen(common_screen.name)


screen utils:
	use demos


screen demos:
	image gui.bg('main_bg'):
		size 1.0
	
	$ demos.update()
	
	null:
		$ btn_bottom = style.return_button.get_current('ypos') + style.return_button.get_current('ysize')
		ypos btn_bottom
		xsize 1.0
		ysize get_stage_height() - btn_bottom
		
		vbox:
			align 0.5
			spacing 0.05
			
			hbox:
				spacing 0.1
				
				for page_name in ('page_buttons_1', 'page_buttons_2'):
					vbox:
						spacing 0.03
						
						$ page = demos[page_name]
						
						for params in page:
							if not params:
								null:
									xsize 0.3
									ysize demos.slot_ysize
							
							else:
								$ mod_name, dir_name, picture = params
								
								button:
									xsize 0.3
									ysize demos.slot_ysize
									ground gui.slot_hover
									action start_mod(dir_name)
									
									hbox:
										yalign 0.5
										
										null size 0.02
										
										image picture:
											size int(0.1 * get_stage_height())
										
										null size 0.02
										
										text mod_name:
											font 'Fregat'
											text_size 0.035
											text_size_min 26
											text_size_max 0.045
											xsize 0.15
											yalign 0.5
			
			hbox:
				style 'change_mod_page_button_hbox'
				alpha 1 if demos.page_count > 1 else 0
				zoom (common_screen.gui_zoom - 1) / 2 + 1
				
				for i in (-1, 0, +1): # prev_btn, text, next_btn
					if i:
						$ disable_btn = demos.page == (0 if i == -1 else demos.page_count - 1)
						textbutton gui['back_button_text' if i == -1 else 'next_button_text']:
							style 'change_mod_page_button'
							alpha 0 if disable_btn else 1
							yalign 0.5
							action 'demos.page += i'
					else:
						text demos.page_text:
							style 'mod_page_text'
							yalign 0.5
				
				python:
					if common_screen.name == 'demos':
						persistent.demos_page = demos.page
	
	textbutton _('Return'):
		style 'return_button'
		action hide_screen(common_screen.name)
	
	key 'ESCAPE' action hide_screen(common_screen.name)
