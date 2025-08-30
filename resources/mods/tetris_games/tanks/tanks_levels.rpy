init python:
	tetris.tanks_levels = {}
	
	# X - wall
	# . - empty
	# P - player
	# F - friend
	# A - enemy in team A
	# B, C... - enemies in other teams
	# (if not campaign-mode, all A-Z (+F) tanks in A team)
	
	tetris.tanks_levels[0] = '''
		XXXXXXXXXXXXXXXXXXXXXXXXX
		X.....XXXXXXXXXXXXX.....X
		X.....XXXXXXXXXXXXX.....X
		X..P..XXXXXXXXXXXXX..A..X
		X.....XXXXXXXXXXXXX.....X
		X.....XXXXXXXXXXXXX.....X
		XXXXXXXXXXXXXXXXXXXXXXXXX
		XXXXXXXXXXXXXXXXXXXXXXXXX
		XXXXXXXXXXXXXXXXXXXXXXXXX
		X.....XXXXXXXXXXXXX.....X
		X.....XXXXXXXXXXXXX.....X
		X..B..XXXXXXXXXXXXX..C..X
		X.....XXXXXXXXXXXXX.....X
		X.....XXXXXXXXXXXXX.....X
		XXXXXXXXXXXXXXXXXXXXXXXXX
	'''
	
	tetris.tanks_levels[1] = '''
		XXXXXXXXXXXXXXXXXXXXXXXXXXX
		X........X.......X........X
		X........X....A..X........X
		X.P......X.......X........X
		X...X....X.......X....X...X
		XXXXX...XXXXX...XXX...XXXXX
		X...X....X.......X....X...X
		X.........................X
		X...................A.....X
		X.........................X
		X...X....X.......X....X...X
		XXXXX...XXXXX...XXX...XXXXX
		X...X....X.......X....X...X
		X........X.......X........X
		X.....A..X.......X........X
		X........X.......X........X
		XXXXXXXXXXXXXXXXXXXXXXXXXXX
	'''
	
	tetris.tanks_levels[2] = '''
		XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
		X..........X.................X....X
		X...P......X.....X...........X.A..X
		X..........X...............X......X
		X.......XXXXX.....................X
		X.................................X
		X.................................X
		X.................................X
		X.............................A...X
		X.................................X
		X.......XXXXX.....................X
		X..........X..........X....XX.....X
		X...F......X.............A.X......X
		X..........X...X.......X......X...X
		XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
	'''
	
	tetris.tanks_levels[3] = '''
		XXXXXXXXXXXXXXXXXXXXXXXXX
		X.....XXXXX...XXXXX.....X
		X..P..XXXXX.A.XXXXX..A..X
		X.....XXXXX...XXXXX.....X
		X.......................X
		X.......................X
		X.......................X
		X.....................A.X
		X.......................X
		X.......................X
		X.......................X
		X.....XXXXX...XXXXX.....X
		X..F..XXXXX.A.XXXXX..A..X
		X.....XXXXX...XXXXX.....X
		XXXXXXXXXXXXXXXXXXXXXXXXX
	'''
	
	tetris.tanks_levels[4] = '''
		XXXXXXXXXXXXXXXXXXXXX......XX.....XX
		X.....X..........X.........XX.....XX
		X.....X....................XX.....XX
		X.....X....................XX..A..XX
		X.....X................X..........XX
		X.....................XX..........XX
		X....................XXX..........XX
		X..P.................XXX..........XX
		X....................XXX..........XX
		X.....................XX..........XX
		X.....X................X..........XX
		X.....X....................XX..A..XX
		X.....X....................XX.....XX
		X.....X..........X.........XX.....XX
		XXXXXXXXXXXXXXXXXXXXX......XX.....XX
	'''
