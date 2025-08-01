init -1000 python:
	
	platformer_bg = im.rect('#222')
	
	def msg(text, color, msg_time = 2.0):
		global main_text, main_text_color, main_text_end_time
		main_text = text
		main_text_color = color
		main_text_end_time = get_game_time() + msg_time
	msg('', 0, -1)
	
	extra_text_vars = []


screen platformer:
	image platformer_bg:
		size 1.0
	
	
	$ left = right = up = down = space = False
	
	$ allow_arrows()
	key 'LEFT'  first_delay 0 action  'left = True'
	key 'RIGHT' first_delay 0 action 'right = True'
	key 'UP'    first_delay 0 action    'up = True'
	key 'DOWN'  first_delay 0 action  'down = True'
	
	key 'A'     first_delay 0 action  'left = True'
	key 'D'     first_delay 0 action 'right = True'
	key 'W'     first_delay 0 action    'up = True'
	key 'S'     first_delay 0 action  'down = True'
	
	key 'SPACE' first_delay 0 action 'space = True'
	
	$ update()
	
	
	image level_image:
		pos  (level_x, level_y)
		xsize level_w * cell_size
		ysize level_h * cell_size
		
		for obj_type, obj_x, obj_y in level_dynamic_objects:
			image obj_type.image:
				xpos obj_x * cell_size
				ypos obj_y * cell_size
				size cell_size
		
		image me.main():
			anchor (0.5, 0.8)
			xpos me.x
			ypos me.y
			xsize int(cell_size * 0.8)
			ysize int(cell_size * 1.6)
			crop  me.crop
	
	if get_game_time() < main_text_end_time:
		text main_text:
			color main_text_color
			text_size 30
			align (0.5, 0.1)
	
	vbox:
		spacing 10
		align (0.99, 0.99)
		
		for var_name, var_desc, var_color in extra_text_vars:
			text (var_desc + str(globals()[var_name])):
				xalign 1.0
				color var_color
