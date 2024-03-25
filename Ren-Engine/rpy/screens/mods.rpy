init python:
	def mods__update():
		mods.list = get_mods()
		mods.count = len(mods.list)
		
		btn_count = gui.prefs_mods_in_page
		mods.page_count = math.ceil(mods.count / btn_count)
		mods.page = in_bounds(mods.page, 0, mods.page_count - 1)
		mods.page_text = str(mods.page + 1) + '/' + str(mods.page_count)
		
		start_index = mods.page * btn_count
		end_index = start_index + btn_count
		mods.page_buttons = mods.list[start_index : end_index]
	
	build_object('mods')
	mods.page = 0


screen mods:
	has vbox
	style 'mods_vbox'
	
	$ mods.update()
	
	for i in range(gui.prefs_mods_in_page - len(mods.page_buttons)):
		null style 'mod_button'
	
	for name, dir_name in mods.page_buttons:
		textbutton name:
			style 'mod_button'
			action start_mod(dir_name)
	
	if mods.page_count > 1:
		hbox:
			style 'change_mod_page_button_hbox'
			
			for i in (-1, 0, +1): # prev_btn, text, next_btn
				if i:
					$ disable_btn = mods.page == (0 if i == -1 else mods.page_count - 1)
					textbutton gui['back_button_text' if i == -1 else 'next_button_text']:
						style 'change_mod_page_button'
						alpha 0 if disable_btn else 1
						yalign 0.5
						action 'mods.page += i'
				else:
					text mods.page_text:
						style 'mod_page_text'
						yalign 0.5
