init python:
	gui.dialogue_text_font = 'FixEx3'
	gui.name_text_font = 'FixEx3'
	
	gui.choice_button_text_font = 'FixEx3'
	
	tutorial = Character('', what_color = '#F80',
		what_prefix = '{color=#F00}[{/color}',
		what_suffix = '{color=#F00}]{/color}',
	)

label start:
	call rpg_start
	call rpg_loop
