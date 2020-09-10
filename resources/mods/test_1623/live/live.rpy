# test_1623_live__

init python:
	test_1623_live__exit = False
	
	test_1623_live__rect_x = 0
	test_1623_live__rect_y = 0
	
	test_1623_live__fps = 20
	
	
	def test_1623_live__init():
		global test_1623__width, test_1623__height
		global test_1623__pause, test_1623_live__exit
		global test_1623_live__alives, test_1623__color_field
		global test_1623_live__nears
		
		set_fps(test_1623_live__fps)
		
		test_1623__width = 50
		test_1623__height = 30
		
		test_1623__pause = False
		test_1623_live__exit = False
		
		test_1623_live__alives = set()
		for i in xrange(test_1623__width * test_1623__height / 3):
			x = renpy.random.randint(0, test_1623__width  - 1)
			y = renpy.random.randint(0, test_1623__height - 1)
			test_1623_live__alives.add((x, y))
		
		test_1623_live__nears = {}
		for y in xrange(test_1623__height):
			for x in xrange(test_1623__width):
				nears = []
				for dy in xrange(-1, 2):
					for dx in xrange(-1, 2):
						if dx != 0 or dy != 0:
							cell = ((x + dx) % test_1623__width, (y + dy) % test_1623__height)
							nears.append(cell)
				test_1623_live__nears[(x, y)] = nears
		
		test_1623__color_field = ['white' for i in xrange(test_1623__width * test_1623__height)]
		test_1623_live__render()
	
	
	def test_1623_live__update():
		global test_1623_live__alives
		
		next_alives = set()
		
		can_alive = set()
		for cell in test_1623_live__alives:
			can_alive.add(cell)
			
			nears = test_1623_live__nears[cell]
			for near in nears:
				can_alive.add(near)
		
		for cell in can_alive:
			nears = test_1623_live__nears[cell]
			
			live_nears = 0
			for near in nears:
				if near in test_1623_live__alives:
					live_nears += 1
					if live_nears == 4:
						break
			
			if live_nears == 3 or (live_nears == 2 and cell in test_1623_live__alives):
				next_alives.add(cell)
		
		test_1623_live__alives = next_alives
		test_1623_live__render()
	
	def test_1623_live__render():
		for i in xrange(test_1623__width * test_1623__height):
			test_1623__color_field[i] = 'white'
		
		for x, y in test_1623_live__alives:
			index = y * test_1623__width + x
			test_1623__color_field[index] = 'black'
		
		rect_index = test_1623_live__rect_y * test_1623__width + test_1623_live__rect_x
		if test_1623__color_field[rect_index] == 'black':
			test_1623__color_field[rect_index] = 'black_rect'
		else:
			test_1623__color_field[rect_index] = 'rect'
	
	
	def test_1623_live__change_cell_state():
		rect = (test_1623_live__rect_x, test_1623_live__rect_y)
		if rect in test_1623_live__alives:
			test_1623_live__alives.remove(rect)
		else:
			test_1623_live__alives.add(rect) 
	
	
	def test_1623_live__clear():
		global test_1623_live__alives
		test_1623_live__alives = set()
	
	
	def test_1623_live__to_exit():
		global test_1623_live__exit
		test_1623_live__exit = True
	
	
	def test_1623_live__make_planer():
		test_1623_live__clear()
		
		start_x = int(test_1623__width / 2) - 1
		start_y = int(test_1623__height / 2) - 1
		
		test_1623_live__alives.add((start_x + 0, start_y + 0))
		test_1623_live__alives.add((start_x + 1, start_y + 0))
		test_1623_live__alives.add((start_x + 2, start_y + 0))
		test_1623_live__alives.add((start_x + 2, start_y + 1))
		test_1623_live__alives.add((start_x + 1, start_y + 2))
		
		test_1623_live__render()
	
	
	def test_1623_live__make_r_pentamino():
		test_1623_live__clear()
		
		start_x = int(test_1623__width / 2) - 1
		start_y = int(test_1623__height / 2) - 1
		
		test_1623_live__alives.add((start_x + 1, start_y + 0))
		test_1623_live__alives.add((start_x + 2, start_y + 0))
		test_1623_live__alives.add((start_x + 0, start_y + 1))
		test_1623_live__alives.add((start_x + 1, start_y + 1))
		test_1623_live__alives.add((start_x + 1, start_y + 2))
		
		test_1623_live__render()
	
	
	def test_1623_live__make_space_ship():
		test_1623_live__clear()
		
		start_x = int(test_1623__width / 2) - 1
		start_y = int(test_1623__height / 2) - 1
		
		test_1623_live__alives.add((start_x + 1, start_y + 0))
		test_1623_live__alives.add((start_x + 2, start_y + 0))
		test_1623_live__alives.add((start_x + 3, start_y + 0))
		test_1623_live__alives.add((start_x + 4, start_y + 0))
		test_1623_live__alives.add((start_x + 0, start_y + 1))
		test_1623_live__alives.add((start_x + 4, start_y + 1))
		test_1623_live__alives.add((start_x + 4, start_y + 2))
		test_1623_live__alives.add((start_x + 0, start_y + 3))
		test_1623_live__alives.add((start_x + 3, start_y + 3))
		
		test_1623_live__render()
	
	def test_1623_live__make_taxi():
		test_1623_live__clear()
		
		start_x = int(test_1623__width / 2) - 10
		start_y = int(test_1623__height / 2) - 1
		
		for i in xrange(9):
			test_1623_live__alives.add((start_x + i * 2 + 1, start_y + 0))
		for i in xrange(10):
			test_1623_live__alives.add((start_x + i * 2 + 0, start_y + 1))
		
		test_1623_live__render()
	
	
	def test_1623_live__on_left_press():
		global test_1623_live__rect_x
		test_1623_live__rect_x = (test_1623_live__rect_x - 1) % test_1623__width
	def test_1623_live__on_right_press():
		global test_1623_live__rect_x
		test_1623_live__rect_x = (test_1623_live__rect_x + 1) % test_1623__width
	def test_1623_live__on_up_press():
		global test_1623_live__rect_y
		test_1623_live__rect_y = (test_1623_live__rect_y - 1) % test_1623__height
	def test_1623_live__on_down_press():
		global test_1623_live__rect_y
		test_1623_live__rect_y = (test_1623_live__rect_y + 1) % test_1623__height


screen test_1623_live__screen:
	key 'K_LEFT' action test_1623_live__on_left_press
	key 'K_RIGHT' action test_1623_live__on_right_press
	key 'K_UP' action test_1623_live__on_up_press
	key 'K_DOWN' action test_1623_live__on_down_press
	key 'a' action test_1623_live__on_left_press
	key 'd' action test_1623_live__on_right_press
	key 'w' action test_1623_live__on_up_press
	key 's' action test_1623_live__on_down_press
	
	key 'K_SPACE' action test_1623_live__change_cell_state
	key 'K_RETURN' action test_1623_live__change_cell_state
	
	key 'c' action test_1623_live__clear
	
	python:
		if not test_1623__pause:
			test_1623_live__update()
		else:
			test_1623_live__render()
	
	use test_1623__main_screen
	
	vbox:
		align (0.5, 0.98)
		spacing 5
		
		hbox:
			xalign 0.5

			textbutton 'ReStart (Init Random)' action test_1623_live__init
			textbutton 'Clear' action test_1623_live__clear
			
			textbutton ('Continue' if test_1623__pause else 'Pause'):
				action test_1623__change_pause_state
			
			textbutton 'Exit' action test_1623_live__to_exit
		
		hbox:
			xalign 0.5
			
			text 'Make: ' text_size 20
			textbutton 'Planer'      action test_1623_live__make_planer
			textbutton 'SpaceShip'   action test_1623_live__make_space_ship
			textbutton 'R-Pentamino' action test_1623_live__make_r_pentamino
			textbutton 'Taxi'        action test_1623_live__make_taxi


label test_1623_live__start:
	$ test_1623_live__init()
	
	window hide
	scene bg black
	show screen test_1623_live__screen
	
	while not test_1623_live__exit:
		pause 0.1
	
	hide screen test_1623_live__screen
	
	jump test_1623__main

