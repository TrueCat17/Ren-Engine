init -100 python:
	DOOM_W, DOOM_H = 480, 270
	H_WIDTH, H_HEIGHT = DOOM_W // 2, DOOM_H // 2
	
	FOV = 90.0
	H_FOV = FOV / 2
	
	PLAYER_SPEED = 300
	PLAYER_ROT_SPEED = 130
	PLAYER_HEIGHT = 41
	
	SCREEN_DIST = H_WIDTH / math.tan(math.radians(H_FOV))
	
	
	mod_dir = os.path.dirname(get_filename(0)) + '/'
