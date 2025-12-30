init python:
	start_screens.append('snow')
	start_screens.remove('dialogue_box')
	
	set_can_mouse_hide(False)
	config.has_autosave = False
	
	
	btn_bg = im.round_rect('#DDDB', 40, 40, 8)
	image_render = False
	
	snowflakes = []
	def set_snowflake_count(count):
		d = count - len(snowflakes)
		
		if d < 0:
			snowflakes[count:] = []
		else:
			width, height = get_stage_size()
			
			for i in range(d):
				size = random.randint(2, 10)
				
				x = random.uniform(0, 1)
				y = random.uniform(0, 1)
				dx = random.uniform(-7, 7) * size / width
				dy = random.uniform(10, 25) * size / height
				
				snowflake = [x, y, dx, dy, size]
				snowflakes.append(snowflake)
	
	def update_snow():
		k = get_last_tick()
		
		# snowflake = [x, y, dx, dy, size]
		
		if image_render:
			width, height = float(get_stage_width()), float(get_stage_height())
			img_cache = [None] + [im.scale('mods/snow/snow.webp', i, i) for i in range(1, 11)]
			
			args = [(width, height)]
			for snowflake in snowflakes:
				x = snowflake[0] = (snowflake[0] + snowflake[2] * k) % 1.0
				y = snowflake[1] = (snowflake[1] + snowflake[3] * k) % 1.0
				
				args.extend(
					((x * width, y * height), img_cache[snowflake[4]])
				)
			global snow_render_image
			snow_render_image = im.composite(*args)
		else:
			for snowflake in snowflakes:
				snowflake[0] = (snowflake[0] + snowflake[2] * k) % 1.0
				snowflake[1] = (snowflake[1] + snowflake[3] * k) % 1.0
	
	set_snowflake_count(5000)


init:
	style btn is textbutton:
		ground im.round_rect('#08F', 20, 20, 4)
		hover  im.round_rect('#F80', 20, 20, 4)
		size 25
		text_size 20
		color 0


screen snow:
	$ update_snow()
	
	if image_render:
		image snow_render_image:
			size 1.0
	else:
		# snowflake = [x, y, dx, dy, size]
		for snowflake in snowflakes:
			image 'mods/snow/snow.webp':
				xpos snowflake[0]
				ypos snowflake[1]
				size snowflake[4]
	
	image btn_bg:
		corner_sizes -1
		size (350, 70)
		align (0.5, 0.95)
		
		null:
			align (0.5, 0.2)
			xsize 300
			
			textbutton '<':
				style 'btn'
				xalign 0.0
				action set_snowflake_count(max(0, len(snowflakes) - 100))
			
			text str(len(snowflakes)):
				xalign 0.5
				text_size 20
				size (100, 25)
				text_align  'center'
				text_valign 'center'
				color 0
			
			textbutton '>':
				style 'btn'
				xalign 1.0
				action set_snowflake_count(min(len(snowflakes) + 100, 30000))
		
		null:
			align (0.5, 0.8)
			xsize 300
			
			text _('Render: to image' if image_render else 'Render: usual'):
				xalign 0.0
				size (250, 25)
				text_size 20
				text_valign 'center'
				color 0
			
			textbutton '%':
				style 'btn'
				xalign 1.0
				action ToggleVariable('image_render')
