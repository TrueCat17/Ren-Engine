init python:
	quick_menu = False
	config.has_autosave = False
	
	
	show_screen('masks')
	
	num_mask = 0
	mask_dir = 'images/masks/'
	mask_images = os.listdir(mask_dir)
	mask_images.sort()
	
	step = 5
	duration = 1.0
	mask_cmp = '<='
	
	mask_start_time = get_game_time()
	def restart_mask():
		global mask_start_time
		mask_start_time = get_game_time()
	
	style.textbutton.ground = im.rect('#08F')
	style.textbutton.hover  = im.rect('#F80')

screen masks:
	python:
		mask_value = in_bounds((get_game_time() - mask_start_time) * 255 / duration, 0, 255)
		if mask_value != 255:
			mask_value = int(mask_value / step) * step
		
		w, h = get_stage_size()
		base_image = im.Scale('images/bg/bus_stop.jpg', w, h)
		mask_image = im.Scale(mask_dir + mask_images[num_mask], w, h)
	
	
	image im.Mask(base_image, mask_image, mask_value, 'r', mask_cmp):
		size 1.0
	
	vbox:
		spacing 5
		align (0.05, 0.95)
		
		hbox:
			spacing 5
			
			textbutton '<-':
				size 30
				action ['duration = max(0.5, duration - 0.5)', restart_mask]
			
			text (' Time: %s ' % duration):
				size (100, 30)
				color 0xFFFFFF
				outlinecolor 0x000000
				text_align 'center'
				text_valign 'center'
			
			textbutton '->':
				size 30
				action ['duration = min(duration + 0.5, 10.0)', restart_mask]
		hbox:
			spacing 5
			
			textbutton '<-':
				size 30
				action ['step = max(1, step - 1)', restart_mask]
			
			text (' Step: %s ' % step):
				size (100, 30)
				color 0xFFFFFF
				outlinecolor 0x000000
				text_align 'center'
				text_valign 'center'
			
			textbutton '->':
				size 30
				action ['step = min(step + 1, 20)', restart_mask]
	
	hbox:	
		spacing 5
		align (0.35, 0.95)
		
		textbutton '<-':
			size 30
			action ['num_mask = max(0, num_mask - 1)', restart_mask]
		
		textbutton mask_images[num_mask]:
			size (150, 30)
			action restart_mask
		
		textbutton '->':
			size 30
			action ['num_mask = min(num_mask + 1, len(mask_images) - 1)', restart_mask]
	
	hbox:
		spacing 5
		align (0.95, 0.95)
		
		for mask_cmp_name in ('<', '>', '==', '!=', '<=', '>='):
			textbutton mask_cmp_name:
				ground style.textbutton['ground' if mask_cmp != mask_cmp_name else 'hover']
				size 30
				action ['mask_cmp = mask_cmp_name', restart_mask]

