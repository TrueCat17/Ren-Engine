init python:
	show_screen('snow')
	set_can_mouse_hide(False)
	set_can_autosave(False)
	
	
	image_render = False
	
	objs = []
	def set_count(count):
		global objs
		d = count - len(objs)
		
		if d < 0:
			objs = objs[0:count]
		else:
			width, height = get_stage_size()
			
			rand_int = random.randint
			def rand_float(min, max):
				return random.random() * (max - min) + min
			
			for i in xrange(d):
				size = rand_int(2, 10)
				
				x = rand_int(0, width)
				y = rand_int(0, height)
				dx = rand_float(-0.12, 0.12) * size
				dy = rand_float( 0.15, 0.40) * size
				
				sizes = (size, size)
				
				obj = [x, y, dx, dy, sizes]
				objs.append(obj)
	
	def update_snow():
		global prev_time_update, tmp_image
		
		k = (time.time() - prev_time_update) * 60
		prev_time_update = time.time()
		
		width, height = get_stage_size()
		x, y, dx, dy, sizes = 0, 1, 2, 3, 4
		
		if image_render:
			img_cache = {}
			for i in xrange(2, 11):
				img_cache[(i, i)] = im.scale('mods/snow/snow.png', i, i)
			
			tmp_image_args = [(width, height)]
			for obj in objs:
				obj[x] = (obj[x] + obj[dx] * k) % width
				obj[y] = (obj[y] + obj[dy] * k) % height
				
				tmp_image_args.extend(
					((obj[x], obj[y]), img_cache[obj[sizes]])
				)
			tmp_image = im.composite(*tmp_image_args)
		else:
			for obj in objs:
				obj[x] = (obj[x] + obj[dx] * k) % width
				obj[y] = (obj[y] + obj[dy] * k) % height
	
	set_count(4000)
	prev_time_update = time.time()


screen snow:
	image 'images/bg/bus_stop.jpg':
		size (1.0, 1.0)
	
	$ update_snow()
	
	if image_render:
		image tmp_image
	else:
		for x, y, dx, dy, sizes in objs:
			image 'mods/snow/snow.png':
				xpos ftoi(x)
				ypos ftoi(y)
				size sizes
	
	image im.Rect('#0004'):
		size (300, 70)
		align (0.5, 0.95)
		
		null:
			align (0.5, 0.2)
			xsize 250
			
			textbutton '<':
				xalign 0.0
				text_size 20
				size (25, 25)
				action set_count(max(0, len(objs) - 100))
			
			text str(len(objs)):
				xalign 0.5
				text_size 20
				size (100, 25)
				text_align 'center'
				text_valign 'center'
				color 0x000000
			
			textbutton '>':
				xalign 1.0
				text_size 20
				size (25, 25)
				action set_count(min(len(objs) + 100, 20000))
		
		null:
			align (0.5, 0.8)
			xsize 250
			
			text ('Render: ' + ('to image' if image_render else 'usual')):
				xalign 0.0
				size (200, 25)
				text_size 20
				text_valign 'center'
				color 0x000000
			
			textbutton '%':
				xalign 1.0
				size (25, 25)
				text_size 20
				action SetVariable('image_render', not image_render)

