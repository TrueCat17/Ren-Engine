init -1:
	style guitar_hero_main_vbox is vbox:
		align 0.5
		spacing guitar_hero.string_spacing
	
	
	style guitar_hero_btn is textbutton:
		ground im.round_rect('#08E', 20, 20, 6)
		hover  im.round_rect('#E80', 20, 20, 6)
		color '#FFF'
		text_size 22
	
	style guitar_hero_close_btn is guitar_hero_btn:
		ground im.round_rect('#F80', 20, 20, 6)
		hover  im.round_rect('#F00', 20, 20, 6)
		color 0
	
	style guitar_hero_song_btn is guitar_hero_btn:
		hover im.round_rect('#A2A', 20, 20, 6)
		xsize 220
	
	style guitar_hero_ctrl_btn is guitar_hero_btn:
		color '#FF0'
		outlinecolor 0
		size 26
		xalign 0.5
	
	
	style guitar_hero_mode_text is text:
		color 0
		xsize 150
		ysize style.guitar_hero_ctrl_btn.ysize
		text_align  'center'
		text_valign 'center'
	
	style guitar_hero_time_text is text:
		color '#FF0'
		outlinecolor 0
		text_size 24
	
	style guitar_hero_score_text is text:
		outlinecolor 0
		text_size 50 / 675
		pos (0.5, 0.1)
		anchor 0.5
	
	style guitar_hero_key_text is text:
		font 'Consola'
		color '#FFF'
		outlinecolor 0
		text_size 16 / 675
		text_size_min 18
		align 0.5
