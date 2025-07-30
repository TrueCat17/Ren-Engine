init python:
	## see other props and funcs in readme.md
	
	
	guitar_hero.bg = im.rect('#0000')
	guitar_hero.shadow_bg = im.rect('#000B')
	
	guitar_hero.close_btn = True
	#guitar_hero.stop_btn = False
	#guitar_hero.record_btn = False
	#guitar_hero.pause_btn = False
	
	#guitar_hero.screen_size_without_panel = False


label start:
	scene bg room with dissolve
	"Some start text with bg."
	window hide
	
	$ guitar_hero.show()
	
	$ db.skip_tab = False
	
	$ prev_playing = False
	while renpy.has_screen('guitar_hero'):
		$ playing = guitar_hero.playing()
		
		if not playing and prev_playing:
			$ guitar_hero.block_playing = True
			$ db.skip_tab = False
			
			me "Difficulty: [guitar_hero.difficulty].\nQuality: [round(guitar_hero.last_game_quality * 100, 1)]%%."
			
			if guitar_hero.last_game_quality > 0.8:
				if guitar_hero.difficulty == 2:
					me "Awesome!"
				if guitar_hero.difficulty == 1:
					me "Nice."
				if guitar_hero.difficulty == 0:
					me "Not bad..?"
			
			window hide
			$ guitar_hero.block_playing = False
		
		$ prev_playing = playing
		
		pause 0.1
	
	$ guitar_hero.hide()
	
	"Some end text."
	window hide
	
	scene with dissolve
