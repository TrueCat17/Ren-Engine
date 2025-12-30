init -1 python:
	def black_cover__start(from_middle = False):
		black_cover.dtime = black_cover.appearance_time if from_middle else 0
		black_cover.was_middle = from_middle
	
	def black_cover__get_alpha():
		dtime = black_cover.dtime
		if dtime > black_cover.appearance_time and not black_cover.was_middle:
			black_cover.was_middle = True
			black_cover.dtime = black_cover.appearance_time
			return 1
		
		black_cover.dtime += min(get_last_tick(), 2 / get_fps())
		
		if black_cover.was_middle:
			dtime -= black_cover.appearance_time
			return 1 - dtime / black_cover.appearance_time
		else:
			return dtime / black_cover.appearance_time
	
	
	build_object('black_cover')
	
	black_cover.appearance_time = 0.35
	black_cover.bg = im.rect('#000')
	
	show_screen('black_cover')
	black_cover.start(from_middle = True)


screen black_cover:
	zorder 1e6
	alpha black_cover.get_alpha()
	
	image black_cover.bg:
		size 1.0
