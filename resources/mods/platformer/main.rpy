init -100 python:
	set_fps(60)
	physics_step = 1.0 / get_fps()
	
	cell_size = 32 # pow of 2: 8, 16, 32...

init python:
	start_screens += ['platformer']
	set_level(0)
	
	prev_update_time = 0
	update_dtime = 0
	last_stay_time = 0
	
	level_x = level_y = 0
	
	def update():
		global last_stay_time
		if not left and not right:
			last_stay_time = get_game_time()
		
		global update_dtime
		update_dtime += get_last_tick()
		while update_dtime > physics_step:
			update_dtime -= physics_step
			
			update_physics()
			check_fail()
		
		global level_x, level_y
		sw, sh = get_stage_size()
		lw, lh = level_w * cell_size, level_h * cell_size
		
		if lw < sw:
			level_x = (sw - lw) / 2
		else:
			level_x = sw / 2 - (me.x + cell_size / 2)
		if lh < sh:
			level_y = (sh - lh) / 2
		else:
			level_y = sh / 2 - (me.y + cell_size / 2)

