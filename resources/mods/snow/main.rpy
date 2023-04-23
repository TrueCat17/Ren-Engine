init python:
	show_screen('snow')
	set_can_mouse_hide(False)
	config.has_autosave = False
	
	
	image_render = False
	
	objs = []
	def set_count(count):
		global objs
		d = count - len(objs)
		
		if d < 0:
			objs = objs[0:count]
		else:
			width, height = get_stage_size()
			
			for i in range(d):
				size = random.randint(2, 10)
				
				x = random.uniform(0, 1)
				y = random.uniform(0, 1)
				dx = random.uniform(-7, 7) * size / width
				dy = random.uniform(10, 25) * size / height
				
				obj = [x, y, dx, dy, size]
				objs.append(obj)
	
	def update_snow():
		k = get_last_tick()
		
		x, y, dx, dy, size = 0, 1, 2, 3, 4
		
		if image_render:
			width, height = float(get_stage_width()), float(get_stage_height())
			img_cache = [None] + [im.scale('mods/snow/snow.png', i, i) for i in range(1, 11)]
			
			tmp_image_args = [(width, height)]
			for obj in objs:
				obj[x] = (obj[x] + obj[dx] * k) % 1.0
				obj[y] = (obj[y] + obj[dy] * k) % 1.0
				
				tmp_image_args.extend(
					((obj[x] * width, obj[y] * height), img_cache[obj[size]])
				)
			global tmp_image
			tmp_image = im.composite(*tmp_image_args)
		else:
			for obj in objs:
				obj[x] = (obj[x] + obj[dx] * k) % 1.0
				obj[y] = (obj[y] + obj[dy] * k) % 1.0
	
	set_count(5000)


screen snow:
	image 'images/bg/bus_stop.jpg':
		size 1.0
	
	$ update_snow()
	
	if image_render:
		image tmp_image:
			size 1.0
	else:
		# x, y, dx, dy, size = 0, 1, 2, 3, 4
		for obj in objs:
			image 'mods/snow/snow.png':
				xpos obj[0]
				ypos obj[1]
				size obj[4]
	
	image im.Rect('#0004'):
		size (350, 70)
		align (0.5, 0.95)
		
		null:
			align (0.5, 0.2)
			xsize 300
			
			textbutton '<':
				xalign 0.0
				text_size 20
				size 25
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
				size 25
				action set_count(min(len(objs) + 100, 30000))
		
		null:
			align (0.5, 0.8)
			xsize 300
			
			text _('Render: ' + ('to image' if image_render else 'usual')):
				xalign 0.0
				size (250, 25)
				text_size 20
				text_valign 'center'
				color 0x000000
			
			textbutton '%':
				xalign 1.0
				size 25
				text_size 20
				action SetVariable('image_render', not image_render)

