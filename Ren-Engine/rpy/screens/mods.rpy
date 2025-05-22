init python:
	def mods__update():
		mod_list = get_mods()
		screen_tmp.count = len(mod_list)
		
		btn_count = gui.prefs_mods_in_page
		screen_tmp.page_count = math.ceil(screen_tmp.count / btn_count)
		mods.page = in_bounds(mods.page, 0, screen_tmp.page_count - 1)
		screen_tmp.page_text = '%s/%s' % (mods.page + 1, screen_tmp.page_count)
		
		start_index = mods.page * btn_count
		end_index = start_index + btn_count
		screen_tmp.page_buttons = mod_list[start_index : end_index]
	
	build_object('mods')
	mods.page = 0


screen mods:
	has vbox
	style 'mods_vbox'
	
	$ screen_tmp = SimpleObject()
	$ mods.update()
	
	for i in range(gui.prefs_mods_in_page - len(screen_tmp.page_buttons)):
		null style 'mod_button'
	
	for name, dir_name in screen_tmp.page_buttons:
		textbutton name:
			style 'mod_button'
			action start_mod(dir_name)
	
	if screen_tmp.page_count > 1:
		hbox:
			style 'change_mod_page_button_hbox'
			
			for i in (-1, 0, +1): # prev_btn, text, next_btn
				if i:
					$ disable_btn = mods.page == (0 if i == -1 else screen_tmp.page_count - 1)
					textbutton gui['back_button_text' if i == -1 else 'next_button_text']:
						style 'change_mod_page_button'
						alpha 0 if disable_btn else 1
						yalign 0.5
						action 'mods.page += i'
				else:
					text screen_tmp.page_text:
						style 'mod_page_text'
						yalign 0.5
