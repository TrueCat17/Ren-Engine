screen choose_menu:
	modal True
	zorder 100
	
	use hotkeys
	
	image 'images/bg/black.jpg':
		size  1.0
		alpha 0.05
	
	vbox:
		align (0.5, 0.5)
		spacing 10
		
		for i in xrange(len(choose_menu_variants)):
			if choose_menu_variants[i]:
				textbutton _(choose_menu_variants[i]):
					text_size 20
					size (300, 35)
					action Return(i)
			elif choose_menu_variants[i] is not None:
				null ysize 35
	
	
	vbox:
		align (0.5, 0.99)
		
		null ysize int(db.text_size * 1.5) # name
		
		hbox:
			spacing 5
			xalign 0.5
			
			button:
				yalign 0.5
				ground db.prev_btn
				size   (db.prev_btn_size, db.prev_btn_size)
				action history.show
			
			null size db.voice_size # text
			null size (db.next_btn_size, db.next_btn_size) # next
	
	
	button:
		ground 	db.menu_btn
		
		anchor 0.5
		pos   (get_stage_width() - db.menu_btn_indent - db.menu_btn_size / 2, db.menu_btn_indent + db.menu_btn_size / 2)
		size  (db.menu_btn_size, db.menu_btn_size)
		action pause_screen.show

