init python:
	def show_prefs():
		black_cover.start(ShowScreen('preferences'))
		common_screen.show_time = get_game_time() + black_cover.appearance_time * 2
	
	quick_menu_screen.old_items = quick_menu_screen.items
	quick_menu_screen.items = []
	
	for btn_name in ('History', 'Prefs', 'Skip'):
		for item in quick_menu_screen.old_items:
			if item[0] != btn_name: continue
			item = item.copy()
			
			if item[0] == 'Prefs':
				item[0] = 'Preferences'
				item[1] = show_prefs
			quick_menu_screen.items.append(item)
			
			quick_menu_screen.prefixes[item[0]] = '{image=images/gui/dialogue/btn_icons/%s.webp} ' % btn_name.lower()
