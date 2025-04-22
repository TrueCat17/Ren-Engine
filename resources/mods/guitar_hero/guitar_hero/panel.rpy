screen guitar_hero_panel:
	image guitar_hero.panel_bg:
		xsize guitar_hero.panel_size
		ysize 1.0
		xalign 1.0
		
		vbox:
			xalign 0.5
			ypos guitar_hero.spacing
			spacing guitar_hero.spacing
			
			vbox:
				xalign 0.5
				spacing guitar_hero.small_spacing
				
				if guitar_hero.close_btn:
					textbutton _(guitar_hero.close_btn_text):
						style 'guitar_hero_close_btn'
						action guitar_hero.hide
				
				if guitar_hero.playing():
					if guitar_hero.pause_btn:
						key 'C' action (guitar_hero.unpause if guitar_hero.paused else guitar_hero.pause)
						textbutton (_('Unpause' if guitar_hero.paused else 'Pause') + ' (C)'):
							style 'guitar_hero_btn'
							action guitar_hero.unpause if guitar_hero.paused else guitar_hero.pause
					elif guitar_hero.record_btn:
						null:
							ysize style.guitar_hero_btn.ysize
					
					textbutton (_('Stop') + ' (S)'):
						style 'guitar_hero_btn'
						alpha 1 if guitar_hero.stop_btn or (guitar_hero.record_btn and guitar_hero.recording) else 0
						action guitar_hero.stop
				else:
					if guitar_hero.record_btn:
						key 'R' action guitar_hero.record
						textbutton (_('Record') + ' (R)'):
							style 'guitar_hero_btn'
							action guitar_hero.record
					elif guitar_hero.pause_btn:
						null:
							ysize style.guitar_hero_btn.ysize
					
					textbutton (_('Play') + ' (S)'):
						style 'guitar_hero_btn'
						action guitar_hero.play
				
				key 'S' action guitar_hero['stop_by_key' if guitar_hero.playing() else 'play']
			
			
			image guitar_hero.separator_bg:
				xsize guitar_hero.panel_size
				ysize 1
			
			
			null:
				xalign 0.5
				ysize guitar_hero.panel_songs_ysize
				
				vbox:
					xalign 0.5
					spacing guitar_hero.small_spacing
					
					if not guitar_hero.playing():
						key 'UP'   action guitar_hero.select_prev_song
						key 'DOWN' action guitar_hero.select_next_song
					
					if guitar_hero.show_arrows:
						textbutton '↑':
							style 'guitar_hero_ctrl_btn'
							alpha 0 if guitar_hero.playing() else 1
							action guitar_hero.select_prev_song
					
					for song in guitar_hero.get_songs():
						textbutton guitar_hero.get_name_for_song(song):
							style 'guitar_hero_song_btn'
							selected guitar_hero.song == song
							alpha 0 if guitar_hero.playing() and guitar_hero.song != song else 1
							action guitar_hero.set_song(song)
					
					if guitar_hero.show_arrows:
						textbutton '↓':
							style 'guitar_hero_ctrl_btn'
							alpha 0 if guitar_hero.playing() else 1
							action guitar_hero.select_next_song
			
			
			image guitar_hero.separator_bg:
				xsize guitar_hero.panel_size
				ysize 1
			
			
			hbox:
				xalign 0.5
				
				if not guitar_hero.playing():
					key 'LEFT'  action guitar_hero.add_difficulty(-1)
					key 'RIGHT' action guitar_hero.add_difficulty(+1)
				
				textbutton '←':
					style 'guitar_hero_ctrl_btn'
					alpha 0 if guitar_hero.playing() or guitar_hero.difficulty == 0 or guitar_hero.block_difficulty else 1
					action guitar_hero.add_difficulty(-1)
				
				text _(guitar_hero.difficulty_names[guitar_hero.difficulty]):
					style 'guitar_hero_mode_text'
				
				textbutton '→':
					style 'guitar_hero_ctrl_btn'
					alpha 0 if guitar_hero.playing() or guitar_hero.difficulty == 2 or guitar_hero.block_difficulty else 1
					action guitar_hero.add_difficulty(+1)
