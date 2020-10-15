screen choose_menu:
	modal True
	zorder 100
	
	use hotkeys
	
	image 'images/bg/black.jpg':
		size (1.0, 1.0)
		alpha 0.05
	
	vbox:
		align (0.5, 0.5)
		spacing 10
		
		for i in xrange(len(choose_menu_variants)):
			if choose_menu_variants[i]:
				textbutton _(choose_menu_variants[i]) action Return(i):
					text_size 20
					size (300, 35)
			elif choose_menu_variants[i] is not None:
				null ysize 35
	
	
	vbox:
		align (0.5, 0.99)
		
		null ysize int(db_text_size * 1.5) # name
		
		hbox:
			spacing 5
			xalign 0.5
			
			button:
				yalign 0.5
				ground db_prev_btn
				size   (db_prev_btn_size, db_prev_btn_size)
				action prev_text_show
			
			null size db_voice_size # text
			null size (db_next_btn_size, db_next_btn_size) # next
	
	
	button:
		ground 	db_menu_btn

		anchor (0.5, 0.5)
		pos    (get_stage_width() - db_menu_btn_indent - db_menu_btn_size / 2, db_menu_btn_indent + db_menu_btn_size / 2)
		size   (db_menu_btn_size, db_menu_btn_size)
		action show_pause

