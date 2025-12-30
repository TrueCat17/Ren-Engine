init python:
	def common_screen__update_params(name):
		global hide_screen
		hide_screen = common_screen.hide_screen
		
		common_screen.name = name
		
		w, h = get_image_size(gui.bg('main_bg'))
		
		xzoom = w / get_stage_width()
		yzoom = h / get_stage_height()
		common_screen.zoom = max(xzoom, yzoom)
		
		xsize = round(w / common_screen.zoom)
		ysize = round(h / common_screen.zoom)
		common_screen.size = (xsize, ysize)
	
	def common_screen__hide_screen(name):
		if black_cover.get_alpha() > 0:
			return
		
		global hide_screen
		hide_screen = common_screen.orig_hide_screen
		
		black_cover.start()
		set_timeout(HideScreen(name), black_cover.appearance_time)
	
	
	build_object('common_screen')
	common_screen.orig_hide_screen = hide_screen
	common_screen.gui_zoom = 1.4
	
	
	is_main_menu = get_current_mod() == 'main_menu'
	if is_main_menu:
		demos.slot_ysize = 0.1455
		
		for name in ('demos', 'utils', 'preferences', 'load'):
			replace_screen(name, name + '_on_screen')
		
		for style_name in ('menu_button', 'page_button', 'prefs_page_button', 'bar', 'bar_button', 'bool_button', 'checkbox', 'return_button'):
			tmp_style = style[style_name]
			if tmp_style.text_size:
				tmp_style.text_size *= common_screen.gui_zoom
			if style_name not in ('page_button', 'prefs_page_button', 'return_button'):
				tmp_style.xsize *= common_screen.gui_zoom
			tmp_style.ysize *= common_screen.gui_zoom
		style.bar.xsize = 0.2
		
		for style_name in ('menu_text', 'bool_text'):
			style[style_name].text_size *= common_screen.gui_zoom
		
		for style_name in ('prefs_content', 'prefs_pages_vbox', 'pages_vbox', 'slots_vbox', 'slots_hbox'):
			style[style_name].spacing *= common_screen.gui_zoom
		
		style.bool_button.xsize_max = 0.45
		
		style.prefs_content.xpos = 0.585
		reus.prefs_name_width = 0.2
		style.pages_vbox.xpos = 0.1
		
		style.slots_content.ypos = 0.16
		style.slots_content.yanchor = 0
		style.pages_vbox.ypos = 0.2
		style.pages_vbox.yanchor = 0.04
		
		# common style for <demos>, <utils>, <preferences>, <load> and <return> btns
		style.return_button.pos = (0.9, 0.05)
		style.return_button.xsize_min = 110
		style.return_button.ysize_min = 25
		style.return_button.text_size_min = 22
		
		style.return_button.alpha = 0 # only for <return> btn
		
		
		# spec <return> btn
		return_btn_ground = im.rect('#00000002')
		return_btn_hover  = im.color('mods/common/screens/exit.webp', '#08F')
	else:
		for name in ('demos', 'utils', 'preferences', 'load', 'save'):
			replace_screen(name, name + '_with_btns')
		
		reus.prefs_name_width = 0.25
	
	def update_common_screen_buttons_params():
		# as <return> button, but on the left
		tmp_style = style.return_button
		
		screen_tmp.alpha = 1
		
		screen_tmp.spacing = style.pages_vbox.get_current('spacing', get_stage_height())
		
		screen_tmp.xsize = tmp_style.get_current('xsize')
		screen_tmp.ysize = tmp_style.get_current('ysize')
		
		screen_tmp.xpos = style.pages_vbox.xpos if is_main_menu else tmp_style.get_current('ypos')
		screen_tmp.ypos = tmp_style.get_current('ypos')
		
		screen_tmp.btns = ('Demos', 'Utils', 'Preferences', 'Load')
		if not is_main_menu:
			screen_tmp.btns += ('Save', )
	
	def call_common_screen_buttons_action(btn):
		show_screen(btn.lower())
		persistent.common_screen_tab = btn.lower()
		set_timeout(HideScreen(common_screen.name), 0) # in the next frame


screen common_screen_buttons:
	$ hide_screen = common_screen.orig_hide_screen
	
	$ screen_tmp = SimpleObject()
	$ update_common_screen_buttons_params()
	
	has hbox
	spacing screen_tmp.spacing
	
	alpha screen_tmp.alpha
	
	xsize screen_tmp.xsize
	ysize screen_tmp.ysize
	
	xpos screen_tmp.xpos
	ypos screen_tmp.ypos
	
	for btn in screen_tmp.btns:
		textbutton _(btn):
			style 'return_button'
			ypos 0
			alpha 1
			selected common_screen.name == btn.lower()
			action call_common_screen_buttons_action(btn)


screen common_screen(name):
	$ common_screen.update_params(screen.name)
	
	image 'images/bg/room_screen.webp':
		size (2560, 1440)
		zoom get_stage_width() / 2560
		
		null:
			clipping True
			xpos absolute(494.5)
			ypos 201
			size common_screen.size
			zoom common_screen.zoom
			
			null:
				align 0.5
				size get_stage_size()
				
				if common_screen.name in ('demos', 'utils'):
					use demos
				if common_screen.name == 'preferences':
					use preferences
				if common_screen.name == 'load':
					use load
				
				use common_screen_buttons
		
		image 'images/bg/room_screen_over.webp':
			xpos absolute(537.5)
			ypos 1212
			size (144, 37)
	
	if is_main_menu:
		button:
			ground return_btn_ground
			hover  return_btn_hover
			selected 0.5 < (get_game_time() - (common_screen.show_time or 0)) < 1.0
			
			size  (0.158, 1.0)
			xalign 1.0
			
			action demos.hide

screen demos_on_screen:
	zorder 10001
	modal  True
	use common_screen('demos')
screen utils_on_screen:
	zorder 10001
	modal  True
	use common_screen('utils')
screen preferences_on_screen:
	zorder 10001
	modal  True
	use common_screen('preferences')
screen load_on_screen:
	zorder 10001
	modal  True
	use common_screen('load')


screen demos_with_btns:
	zorder 10001
	modal  True
	save   False
	$ common_screen.update_params('demos')
	use demos
	use common_screen_buttons
screen utils_with_btns:
	zorder 10001
	modal  True
	save   False
	$ common_screen.update_params('utils')
	use utils
	use common_screen_buttons
screen preferences_with_btns:
	zorder 10001
	modal  True
	save   False
	$ common_screen.update_params('preferences')
	use preferences
	use common_screen_buttons
screen load_with_btns:
	zorder 10001
	modal  True
	save   False
	$ common_screen.update_params('load')
	use load
	use common_screen_buttons
screen save_with_btns:
	zorder 10001
	modal  True
	save   False
	$ common_screen.update_params('save')
	use save
	use common_screen_buttons
