init -1000 python:
	
	def msg(text, color, msg_time = 2.0):
		global main_text, main_text_color, main_text_end_time
		main_text = text
		main_text_color = color
		main_text_end_time = get_game_time() + msg_time
	msg('', 0, -1)
	
	extra_text_vars = []


screen platformer:
	image im.rect('#222'):
		size (1.0, 1.0)
	
	
	$ shift_is_down = left = right = up = down = space = False
	
	key 'LEFT SHIFT'  action SetVariable('shift_is_down', True) first_delay 0
	key 'RIGHT SHIFT' action SetVariable('shift_is_down', True) first_delay 0
	
	key 'LEFT'  action SetVariable('left',  True) first_delay 0
	key 'RIGHT' action SetVariable('right', True) first_delay 0
	key 'UP'    action SetVariable('up',    True) first_delay 0
	key 'DOWN'  action SetVariable('down',  True) first_delay 0
	key 'a'     action SetVariable('left',  True) first_delay 0
	key 'd'     action SetVariable('right', True) first_delay 0
	key 'w'     action SetVariable('up',    True) first_delay 0
	key 's'     action SetVariable('down',  True) first_delay 0
	
	key 'SPACE' action SetVariable('space', True) first_delay 0
	
	$ update()
	
	
	image level_image:
		pos  (level_x, level_y)
		size (level_w * cell_size, level_h * cell_size)
		
		for obj_type, obj_x, obj_y in level_dynamic_objects:
			image obj_type.image:
				pos  (obj_x * cell_size, obj_y * cell_size)
				size (cell_size, cell_size)
		
		image me.main():
			pos  (int(me.x + 0.1 * cell_size), int(me.y - 0.33 * cell_size))
			size (int(cell_size * 0.8), int(cell_size * 1.6))
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

