init python:
	show_screen('masks')
	
	num_mask = 0
	mask_dir = 'images/masks/'
	mask_images = os.listdir(mask_dir)
	mask_images.sort()
	
	mask_cmp_desks = (
		('l', '<'),
		('g', '>'),
		('e', '=='),
		('ne', '!='),
		('le', '<='),
		('ge', '>=')
	)
	mask_cmp = 'le'
	
	step = 5
	duration = 1.0
	
	mask_start_time = get_game_time()
	def restart_mask():
		global mask_start_time
		mask_start_time = get_game_time()

screen masks:
	python:
		mask_value = in_bounds((get_game_time() - mask_start_time) * 255 / duration, 0, 255)
		if mask_value != 255:
			mask_value = int(mask_value / step) * step
		
		w, h = get_stage_width(), get_stage_height()
		base_image = im.Scale('images/bg/bus_stop.jpg', w, h)
		mask_image = im.Scale(mask_dir + mask_images[num_mask], w, h)
	
	
	image im.Mask(base_image, mask_image, mask_value, 'r', mask_cmp):
		size (1.0, 1.0)
	
	vbox:
		spacing 5
		align (0.05, 0.95)
		
		hbox:
			spacing 5
			
			textbutton '<-':
				size (30, 30)
				color 0xFFFFFF
				action [SetVariable('duration', max(0.5, duration - 0.5)), restart_mask]
			
			text str(duration):
				size (50, 30)
				color 0xFF0000
				text_align 'center'
				text_valign 'center'
			
			textbutton '->':
				color 0xFFFFFF
				size (30, 30)
				action [SetVariable('duration', min(duration + 0.5, 10)), restart_mask]
		hbox:
			spacing 5
			
			textbutton '<-':
				size (30, 30)
				color 0xFFFFFF
				action [SetVariable('step', max(1, step - 1)), restart_mask]
			
			text str(step):
				size (50, 30)
				color 0xFF0000
				text_align 'center'
				text_valign 'center'
			
			textbutton '->':
				color 0xFFFFFF
				size (30, 30)
				action [SetVariable('step', min(step + 1, 20)), restart_mask]
	
	hbox:	
		spacing 5
		align (0.35, 0.95)
		
		textbutton '<-':
			xsize 30
			color 0xFFFFFF
			action [SetVariable('num_mask', max(0, num_mask - 1)), restart_mask]
		
		textbutton mask_images[num_mask]:
			xsize 150
			action restart_mask
		
		textbutton '->':
			xsize 30
			color 0xFFFFFF
			action [SetVariable('num_mask', min(num_mask + 1, len(mask_images) - 1)), restart_mask]
	
	hbox:
		spacing 5
		align (0.95, 0.95)
		
		for mask_cmp_name, mask_cmp_desk in mask_cmp_desks:
			textbutton (mask_cmp_name + ' (' + mask_cmp_desk + ')'):
				xsize 50
				action [SetVariable('mask_cmp', mask_cmp_name), restart_mask]

