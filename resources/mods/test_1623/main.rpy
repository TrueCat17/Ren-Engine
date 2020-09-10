# test_1623_

init python:
	day_time()
	
	test_1623__pause = False
	
	test_1623__width = 1
	test_1623__height = 1
	test_1623__size = 16
	
	test_1623__color_field = ['green']
	
	def test_1623__change_pause_state():
		global test_1623__pause
		test_1623__pause = not test_1623__pause


screen test_1623__main_screen:
	key 'ESCAPE' action SetVariable('test_1623__pause', True)
	
	image 'mods/test_1623/images/px/white.png':
		align (0.5, 0.4)
		size (test_1623__width * test_1623__size, test_1623__height * test_1623__size)
	
	vbox:
		align (0.5, 0.4)
		for i in xrange(test_1623__height):
			# На случай, если test_1623__height поменяется прямо во время этого цикла
			if i >= test_1623__height:
				break
			
			hbox:
				python:
					test_1623__line = []
					
					test_1623__count = 0
					for j in xrange(test_1623__width):
						test_1623__count += 1
						
						test_1623__color = test_1623__color_field[i * test_1623__width + j]
						
						test_1623__is_end = j == test_1623__width - 1
						
						if test_1623__is_end:
							test_1623__next_color = 'no'
						else:
							test_1623__next_color = test_1623__color_field[i * test_1623__width + j + 1]
					
						test_1623__need_draw = test_1623__color != test_1623__next_color
						if test_1623__need_draw:
							test_1623__line.append((test_1623__color, test_1623__count))
							test_1623__count = 0
				for test_1623__color, test_1623__count in test_1623__line:
					image ('mods/test_1623/images/px/' + test_1623__color + '.png'):
						size (test_1623__size * test_1623__count, test_1623__size)


label start:
	jump test_1623__main

label test_1623__main:
	$ set_fps(20)
	
	scene bg room
	
	window show
	'Выбирай игру!'
	menu:
		'Жизнь':
			jump test_1623_live__start
		'Змейка':
			jump test_1623_shake__start
		'Танчики':
			jump test_1623_tanks__start_usual
		'Упрощённые танчики':
			jump test_1623_tanks__start_simple
		''
		'Описание':
			jump test_1623_description__start
		''
		'Выход':
			'До встречи!'
