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
	
	btn_size = 30
	indent = 5

init:
	style masks_text is text:
		size (135, btn_size)
		color '#FFF'
		outlinecolor 0
		text_align  'center'
		text_valign 'center'
	
	style masks_textbutton is textbutton:
		ground im.round_rect('#08F', 20, 20, 6)
		hover  im.round_rect('#F80', 20, 20, 6)
		color       '#EEE'
		hover_color '#111'
		size btn_size

screen masks:
	python:
		mask_value = in_bounds((get_game_time() - mask_start_time) / duration, 0, 1) * 255
		if mask_value != 255:
			mask_value = int(mask_value / step) * step
		
		w, h = get_stage_size()
		base_image = im.scale('images/bg/bus_stop.jpg', w, h)
		mask_image = im.scale(mask_dir + mask_images[num_mask], w, h)
	
	
	image im.mask(base_image, mask_image, mask_value, 'r', mask_cmp):
		size 1.0
	
	null:
		xalign 0.5
		xsize get_stage_width() - btn_size * 2
		
		ypos get_stage_height() - btn_size // 2
		yanchor 1.0
		
		vbox:
			spacing indent
			align (0.0, 1.0)
			
			hbox:
				spacing indent
				
				textbutton '←':
					style 'masks_textbutton'
					action ['duration = max(0.5, duration - 0.5)', restart_mask]
				
				text ('Time: %s' % duration):
					style 'masks_text'
				
				textbutton '→':
					style 'masks_textbutton'
					action ['duration = min(duration + 0.5, 10.0)', restart_mask]
			
			hbox:
				spacing indent
				
				textbutton '←':
					style 'masks_textbutton'
					action ['step = max(1, step - 1)', restart_mask]
				
				text ('Step: %s' % step):
					style 'masks_text'
				
				textbutton '→':
					style 'masks_textbutton'
					action ['step = min(step + 1, 20)', restart_mask]
		
		hbox:
			spacing indent
			align (0.5, 1.0)
			
			textbutton '←':
				style 'masks_textbutton'
				action ['num_mask = max(0, num_mask - 1)', restart_mask]
			
			textbutton mask_images[num_mask]:
				style 'masks_textbutton'
				xsize 150
				action restart_mask
			
			textbutton '→':
				style 'masks_textbutton'
				action ['num_mask = min(num_mask + 1, len(mask_images) - 1)', restart_mask]
		
		hbox:
			spacing indent
			align (1.0, 1.0)
			
			for mask_cmp_name in ('<', '>', '==', '!=', '<=', '>='):
				textbutton mask_cmp_name:
					style 'masks_textbutton'
					selected mask_cmp == mask_cmp_name
					action ['mask_cmp = mask_cmp_name', restart_mask]
