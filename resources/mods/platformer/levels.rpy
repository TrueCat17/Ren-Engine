init -10 python:
	levels = []
	def add_level(level):
		if not level:
			out_msg('Invalid level')
			return
		w, h = len(level[0]), len(level)
		
		start = False
		for line in level:
			if not start:
				start = 'S' in line
			if line[-1] != ' ':
				out_msg('add_level', 'Line must ends with space symbol')
				return
			if len(line) != w:
				out_msg('add_level', 'All lines of level must have one length')
				return
		
		if not start:
			out_msg('add_level', 'Start point not found')
			return
		
		levels.append(level)
	
	
	# Symbols:
	#   < > (space): empty block
	#   S: start point, respawn
	#   X: wall
	#   / or \: corners of wall
	#   _: vertex of wall
	#   -: ladder, path to up/down
	#   o: coin, money, score
	#   E: exit to next level
	
	
	level_classes = {
		' ': Pltf_Empty,
		'S': Pltf_Start,
		'X': Pltf_Wall,
		'/': Pltf_LeftWall,
		'\\': Pltf_RightWall,
		'_': Pltf_VertexWall,
		'-': Pltf_Ladder,
		'o': Pltf_Coin,
		'E': Pltf_Exit,
	}
	
	
	# Warning!
	# Last symbol must be space: < >
	# And all strings of level must have one length
	
	
	# 0
	add_level([
		r'                     ',
		r'       E  _          ',
		r'         /X\o/XXXX-  ',
		r'                  -  ',
		r'                  -  ',
		r'                  -  ',
		r' /X-X\          /XX\ ',
		r'   - X\     S  /X    ',
		r' o  o X\  _   /X     ',
		r'XXXXXXXXXXXXXXXXXXXX '
	])
	
	# 1
	add_level([
		r'E                o ',
		r'X XXXXXXXXXXXXX-X\ ',
		r' o             -   ',
		r'   /XX\-/XXXXXXX\  ',
		r'   S   -      o    ',
		r'X /XXXXXX\-/XXXXX\ ',
		r'       o  -     o  ',
		r'XXXXXXXXXXXXXXXXXX ',
	])
	
	# 2
	add_level([
		r'o o ',
		r'    ',
		r' S  ',
		r'/X\ '
	])
