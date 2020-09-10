init python:
	set_fps(20)
	set_can_mouse_hide(False)
	set_can_autosave(False)
	
	db_hide_interface = True # for disable pause-menu in screen <hotkeys>
	start_screens = ['hotkeys', 'main_menu']
	
	back_path = gui + 'menu/main/back.png'
	mods = get_mods()

screen main_menu:
	image back_path:
		size (1.0, 1.0)
	
	vbox:
		align (0.5, 0.25)
		spacing 5
		
		for name, dir_name in mods:
			textbutton name action start_mod(dir_name)
	
	vbox:
		align (0.5, 0.85)
		spacing 5
		
		textbutton "Загрузить" xalign 0.5 action ShowMenu('load')
		textbutton "Настройки" xalign 0.5 action ShowMenu('settings')
		textbutton "Выход"     xalign 0.5 action exit_from_game

