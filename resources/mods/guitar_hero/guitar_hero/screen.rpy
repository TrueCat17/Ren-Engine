init -200 python:
	def guitar_hero__get_shadow_size():
		sw, sh = get_stage_size()
		
		string_count = guitar_hero.string_count
		string_xsize = get_absolute(guitar_hero.string_xsize, sw)
		string_spacing = get_absolute(guitar_hero.string_spacing, sw)
		xsize  = get_absolute(guitar_hero.shadow_xpadding, sw) * 2
		xsize += string_count * string_xsize + (string_count - 1) * string_spacing
		
		string_ysize = get_absolute(guitar_hero.string_ysize, sh)
		ysize  = get_absolute(guitar_hero.shadow_ypadding, sh) * 2
		ysize += string_ysize
		
		return (xsize, ysize)


screen guitar_hero:
	$ guitar_hero.update()
	
	xsize get_stage_width() - (guitar_hero.panel_size if guitar_hero.screen_size_without_panel else 0)
	alpha guitar_hero.alpha
	
	image guitar_hero.bg:
		size 1.0
	
	for key in '1234560':
		key key action guitar_hero['record_key' if guitar_hero.recording else 'process_key'](key)
	
	
	vbox:
		style 'guitar_hero_main_vbox'
		
		text guitar_hero.score_text:
			style 'guitar_hero_score_text'
			color guitar_hero.score_color
		
		if guitar_hero.ok_in_row_for_x2 > 0:
			text guitar_hero.combo_text:
				style 'guitar_hero_score_text'
				color guitar_hero.combo_color
		
		image guitar_hero.shadow_bg:
			size guitar_hero.get_shadow_size()
			
			hbox:
				spacing guitar_hero.string_spacing
				align 0.5
				
				for string in range(1, guitar_hero.string_count + 1):
					image guitar_hero.string_images[string]:
						xsize guitar_hero.string_xsize
						ysize guitar_hero.string_ysize
						corner_sizes guitar_hero.string_light_border_size
						
						image guitar_hero.note_target_images[string]:
							anchor 0.5
							pos (0.5, 1.0)
							xsize guitar_hero.note_xsize
							ysize guitar_hero.note_ysize
							
							text str(string):
								style 'guitar_hero_key_text'
						
						for y, alpha in guitar_hero.get_notes(string):
							image guitar_hero.note_images[string]:
								xalign 0.5
								yanchor 0.5
								ypos y
								xsize guitar_hero.note_xsize
								ysize guitar_hero.note_ysize
								alpha alpha
		
		text guitar_hero.get_time():
			style 'guitar_hero_time_text'
			xalign 0.5
		
		if guitar_hero.ok_in_row_for_x2 > 0:
			text ' ' style 'guitar_hero_score_text'
	
	
	use guitar_hero_panel
