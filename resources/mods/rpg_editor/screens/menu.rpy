screen menu:
	zorder 1000000
	
	textbutton '?':
		color  0xFFFF00
		ground im.circle('#080')
		hover  im.circle('#0B0')
		size   25
		align (1.0, 0.0)
		action show_help
	key 'F1' action show_help
