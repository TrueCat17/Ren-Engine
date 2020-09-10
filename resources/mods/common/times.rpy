init -1000 python:
	
	set_time_text('day', 'Наступает день') # set text for RPG in day_time
	
	make_time('sunset', text = 'Наступает вечер', sprite=(240, 210, 255), location=(240, 210, 255)) # make def sunset_time
	make_time('night',  text = 'Наступает ночь',  sprite=(160, 200, 210), location=(140, 180, 210)) # night_time

