init python:
	#__builtins__.start_mod('tutorial')
	set_can_mouse_hide(False)
	config.has_autosave = False
	pause_screen.disable = True
	
	ship_ambience = 'sound/ambience/ship.ogg'
	renpy.music.play(ship_ambience, channel = 'ambience', fadein = black_cover.appearance_time)
	renpy.music.set_pos(time.time() % renpy.music.get_audio_len(ship_ambience), channel = 'ambience')


label start:
	$ show_bg_entry()
	
	if not persistent.get('first_meet_ended'):
		call first_meet
		$ conversation.prepare(wait = True)
	else:
		$ conversation.prepare(wait = False)
	
	$ btns.show()
	$ conversation.show()
	
	while True:
		pause 1
