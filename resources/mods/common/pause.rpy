init python:
	for item in pause_screen.items:
		if item[0] == 'Exit to menu':
			item[1] = Function(start_mod, 'main_menu')
