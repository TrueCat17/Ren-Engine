init -990 python:
	
	# VN characters:
	
	sm = Character('Семён', color = '#DDA')
	me = sm
	rn = Character('Рен', color = '#FFF')
	
	
	# RPG characters:
	
	un = Character('Лена',   unknown_name = 'Пионерка', color = '#B956FF')
	sl = Character('Славя',  unknown_name = 'Пионерка', color = '#F3F301')
	us = Character('Ульяна', unknown_name = 'Пионерка', color = '#FF3200')
	
	sh = Character('Шурик',  unknown_name = 'Пионер',   color = '#F3F301')
	
	
	sm.make_rpg('images/characters/', 'sm', 'winter')
	
	g = globals()
	for name in 'us un sl sh'.split(' '):
		g[name].make_rpg('images/characters/', name, 'pioneer')

init -980 python:
	register_character_animation(sl, 'hello',   'images/characters/anim/sl_hello', 0, 0, 4, 0, 3, 1.0)
	register_character_animation(us, 'cricket', 'images/characters/anim/us_cricket', 0, 0, 6, 0, 5, 1.5)

