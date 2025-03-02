init -1000:
	
	image rn happy     = 'images/sprites/rn/happy.png'
	image rn normal    = 'images/sprites/rn/normal.png'
	image rn serious   = 'images/sprites/rn/serious.png'
	image rn serious-2 = 'images/sprites/rn/serious_2.png'
	image rn surprise  = 'images/sprites/rn/surprise.png'
	
	image rn normal night = im.ReColor('images/sprites/rn/normal.png', 160, 160, 220, 255)
	
	
	image photo ren = im.MatrixColor('images/sprites/rn/happy.png', im.matrix.saturation(0)):
		zoom 0.5
	image photo room = im.MatrixColor('images/bg/room.jpg', im.matrix.colorize('#FF0', '#08F')):
		zoom 0.33
