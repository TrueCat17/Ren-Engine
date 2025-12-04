init python:
	draw_list = []

screen doom:
	xsize DOOM_W
	ysize DOOM_H
	align 0.5
	
	$ ctrl = False
	key 'LEFT CTRL'  first_delay 0 action 'ctrl = True'
	key 'RIGHT CTRL' first_delay 0 action 'ctrl = True'
	xzoom 1 if ctrl else (get_stage_width()  / DOOM_W)
	yzoom 1 if ctrl else (get_stage_height() / DOOM_H)
	
	python:
		speed = PLAYER_SPEED * get_last_tick()
		rot_speed = PLAYER_ROT_SPEED * get_last_tick()
		x = y = 0
	
	$ allow_arrows()
	key 'LEFT'  first_delay 0 action 'player.angle += rot_speed'
	key 'RIGHT' first_delay 0 action 'player.angle -= rot_speed'
	
	key 'A' first_delay 0 action 'y += speed'
	key 'D' first_delay 0 action 'y -= speed'
	key 'W' first_delay 0 action 'x += speed'
	key 'S' first_delay 0 action 'x -= speed'
	
	python:
		if x and y:
			x /= 2 ** 0.5
			y /= 2 ** 0.5
		
		player.pos[0] += x * _cos(player.angle) - y * _sin(player.angle)
		player.pos[1] += x * _sin(player.angle) + y * _cos(player.angle)
		
		draw_list = []
		doom.update()
	
	for img, pos, ysize, crop in draw_list:
		image img:
			pos pos
			xsize 1
			ysize ysize
			crop crop
	
	for key in '123456789':
		key key:
			first_delay 1000
			action doom.set_level(key)


screen desc:
	zorder 10
	
	vbox:
		align (0.98, 0.02)
		spacing 5
		
		for text in ('Use keys 1-9 to set the desired level', 'Controls: WASD and arrows', 'Ctrl - disable zoom'):
			text _(text):
				xalign 1.0
				color '#FFF'
				outlinecolor 0
				text_size 20
