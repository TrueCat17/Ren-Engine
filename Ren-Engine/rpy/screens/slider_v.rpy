init -1000 python:
	sliders_v = {}
	
	def slider_v_init(name, size, slider_size = 0.4, ground = None, hover = None, value = 0.0, button_size = 25, spacing = 4, buttons = True):
		sliders_v[name] = {
			'size': size,
			'slider_size': slider_size,
			'ground': ground or gui.vbar_ground,
			'hover': hover or gui.vbar_hover,
			'value': value,
			'button_size': button_size,
			'spacing': spacing,
			'buttons': buttons,
		}
	def slider_v_change(name, **args):
		t = sliders_v[name]
		
		for key in args:
			if key not in t:
				out_msg('slider_v_change', 'Unexpected param <' + key + '>')
			else:
				t[key] = args[key]
	
	def slider_v_get_value(name):
		return float(sliders_v[name]['value'])
	
	def slider_v_set_value(name, value):
		sliders_v[name]['value'] = in_bounds(value, 0, 1)
	
	
	def slider_v_add_value(k):
		slider_v_cur['value'] = in_bounds(slider_v_cur['value'] + k, 0, 1)
	
	def slider_v_get_bar_size():
		res = get_absolute(slider_v_cur['size'], get_stage_height()) - slider_v_cur['spacing'] * 2
		if slider_v_cur['buttons']:
			res -= 2 * (slider_v_cur['button_size'] + slider_v_cur['spacing'])
		return res
	
	def slider_v_update(local_y):
		bar = slider_v_get_bar_size()
		slider = bar * slider_v_cur['slider_size']
		if bar <= slider:
			value = 0
		else:
			value = (local_y - slider / 2) / (bar - slider)
		slider_v_cur['value'] = in_bounds(value, 0, 1)
	
	
	slider_v_scrolling = False
	slider_v_hovered = False
	slider_v_scroll_y = 0


init:
	style slider_v_button is textbutton
	
	# implementaion detail, don't touch
	style slider_v_over_btn is button:
		ground im.rect('#000')
		hover  im.rect('#000')
		corner_sizes 0
		alpha 0.01


screen slider_v(name):
	$ slider_v_cur = sliders_v[screen.name]
	
	xsize slider_v_cur['button_size']
	ysize slider_v_cur['size']
	alpha 1 if slider_v_cur['slider_size'] < 1 else 0
	
	vbox:
		# [spacing, btn], spacing, bar, spacing, [btn, spacing]
		
		if slider_v_cur['buttons']:
			null ysize slider_v_cur['spacing']
			
			textbutton '/\\':
				style 'slider_v_button'
				text_size slider_v_cur['button_size'] - 10
				size slider_v_cur['button_size']
				action slider_v_add_value(-0.1)
		
		null ysize slider_v_cur['spacing']
		
		
		python:
			tmp_slider_size = slider_v_cur['slider_size']
			tmp_value = slider_v_cur['value']
			
			tmp_xsize = slider_v_cur['button_size']
			tmp_ysize = slider_v_get_bar_size()
		
		image slider_v_cur['ground']:
			corner_sizes -1
			xsize tmp_xsize
			ysize tmp_ysize
			
			image slider_v_cur['hover']:
				corner_sizes -1
				ypos tmp_ysize * tmp_value * (1 - tmp_slider_size)
				xsize tmp_xsize
				ysize tmp_ysize * tmp_slider_size
				crop (0, float(tmp_value * (1 - tmp_slider_size)), 1.0, float(tmp_slider_size))
			
			# separate button for case <ground with transparent parts>
			button:
				style 'slider_v_over_btn'
				xsize tmp_xsize
				ysize tmp_ysize
				
				unhovered 'slider_v_hovered = False'
				action ['slider_v_hovered = True', slider_v_update(get_local_mouse()[1])]
		
		python:
			if not slider_v_scrolling and slider_v_hovered and get_mouse_down():
				slider_v_scrolling = True
				slider_v_y = get_mouse()[1] - get_local_mouse()[1]
			if not get_mouse_down():
				slider_v_scrolling = False
			
			if slider_v_scrolling:
				slider_v_update(get_mouse()[1] - slider_v_y)
		
		
		null ysize slider_v_cur['spacing']
		
		if slider_v_cur['buttons']:
			textbutton '\\/':
				style 'slider_v_button'
				text_size slider_v_cur['button_size'] - 10
				size slider_v_cur['button_size']
				action slider_v_add_value(+0.1)
			
			null ysize slider_v_cur['spacing']

