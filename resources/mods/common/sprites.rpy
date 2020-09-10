init -1000:
	
	$ default_decl_at = ["size (0.45, 0.85)", "pause 0.1", "repeat"]
	
	image rn happy     = 'images/sprites/rn/happy.png'
	image rn normal    = 'images/sprites/rn/normal.png'
	image rn serious   = 'images/sprites/rn/serious.png'
	image rn serious-2 = 'images/sprites/rn/serious_2.png'
	image rn surprise  = 'images/sprites/rn/surprise.png'
	
	image rn normal night = im.ReColor('images/sprites/rn/normal.png', 160, 160, 220, 255)
	
	$ default_decl_at = []
	
	
	image photo ren = im.MatrixColor('images/sprites/rn/happy.png', im.matrix.saturation(0)):
		size (0.22, 0.42)
		pause 0.1
		repeat
	image photo room = im.MatrixColor('images/bg/room.jpg', im.matrix.colorize('#FF0', '#08F')):
		size (0.4, 0.3)
		pause 0.1
		repeat

