init python:
	def authors__update_text():
		authors.programmer = _('Programmer') + ': '
		authors.artist = _('Artist') + ': '
	
	def authors__get_alpha():
		if db.visible:
			return 0
		
		dtime = get_game_time()
		if dtime < authors.showing_time:
			return 1
		
		dtime -= authors.showing_time
		if dtime > authors.disappearance_time:
			hide_screen('authors')
		
		return 1 - dtime / authors.disappearance_time
	
	
	authors = SimpleObject()
	build_object('authors')
	
	authors.showing_time = 15
	authors.disappearance_time = 1
	
	authors.bg = im.round_rect('#0007', 50, 50, 8)
	
	signals.add('language', authors.update_text)
	
	show_screen('authors')


init:
	style authors is text:
		font 'Fregat'
		color '#EEF'
		
		text_size 30 / 1080
		text_size_min 16


screen authors:
	alpha authors.get_alpha()
	
	image authors.bg:
		corner_sizes -1
		
		size (0.16, 0.08)
		size_min (160, 50)
		
		$ indent = get_stage_width() // 60
		xpos indent
		ypos get_stage_height() - indent
		yanchor 1.0
		
		hbox:
			align 0.5
			
			vbox:
				text authors.programmer style 'authors'
				text authors.artist     style 'authors'
			
			vbox:
				text 'TrueCat' style 'authors'
				text 'Snaro'   style 'authors'
