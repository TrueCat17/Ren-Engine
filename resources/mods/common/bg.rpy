init -1000:
	
	transform default_decl_at:
		size 1.0
	
	
	image bg black = "images/bg/black.jpg"
	
	image bg entry = "images/bg/entry.webp"
	image bg monitor = "images/bg/monitor.webp"
	
	
	image bg room_screen:
		size 1.0
		
		contains "images/bg/room_screen.webp"
		contains "images/bg/screen_stars.png":
			pos (absolute(494.5), absolute(201.5))
		
		contains "images/bg/room_screen_over.webp":
			pos (absolute(537.5), 1212)
