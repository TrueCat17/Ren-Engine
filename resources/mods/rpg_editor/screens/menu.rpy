init:
	style rpg_editor_menu is textbutton:
		ground im.circle('#080')
		hover  im.circle('#0B0')
		size   25
		color  '#FF0'
		align (1.0, 0.0)

screen menu:
	zorder 10000
	
	textbutton '?':
		style 'rpg_editor_menu'
		action show_screen('help')
	
	key 'F1' action show_screen('help')
