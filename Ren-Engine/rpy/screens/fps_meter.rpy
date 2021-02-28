init -1000 python:
	fps_meter_size = 24
	fps_meter_color = 0xFFFF00
	fps_meter_font = 'Alcdnova'
	fps_meter_xalign, fps_meter_yalign = 0.01, 0.01
	fps_meter_outlinecolor = 0x880000
	
	fps_time_array = []
	def get_last_fps():
		cur_time = get_game_time()
		
		global fps_time_array
		fps_time_array.append(cur_time)
		fps_time_array = fps_time_array[-get_fps():]
		
		count = 0
		for t in fps_time_array:
			if cur_time - t < 1:
				count += 1
		return str(min(count, get_fps()))


screen fps_meter:
	zorder 1000000
	
	if config.fps_meter:
		text get_last_fps():
			font         fps_meter_font
			text_size    fps_meter_size
			color        fps_meter_color
			xalign       fps_meter_xalign
			yalign       fps_meter_yalign
			outlinecolor fps_meter_outlinecolor
