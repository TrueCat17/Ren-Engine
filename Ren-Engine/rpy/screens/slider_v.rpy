init -1000 python:
	
	def slider_v__init(name, size, slider_size = 0.4, ground = None, hover = None, value = 0.0, button_size = 25, spacing = 4, buttons = True, button_style = None, hover_spacing = None):
		data = slider_v.data[name] = SimpleObject()
		data.size         = size
		data.slider_size  = slider_size
		data.ground       = ground or gui.vbar_ground
		data.hover        = hover  or gui.vbar_hover
		data.value        = value
		data.button_size  = button_size
		data.spacing      = spacing
		data.buttons      = buttons
		data.button_style = button_style or 'slider_v_button'
		
		data.scrolling = False
		data.hovered = False
		data.y = 0
		data.hover_spacing = hover_spacing if hover_spacing is not None else gui.vbar_hover_spacing
	
	def slider_v__change(name, **args):
		data = slider_v.data[name]
		
		for key, value in args.items():
			if key not in data:
				out_msg('slider_v.change', 'Unexpected param <%s>', key)
			else:
				data[key] = value
	
	def slider_v__get_value(name):
		return float(slider_v.data[name].value)
	
	def slider_v__set_value(name, value):
		slider_v.data[name].value = in_bounds(value, 0, 1)
	
	
	def slider_v__add_value(k):
		slider_v_cur.value = in_bounds(slider_v_cur.value + k, 0, 1)
	
	def slider_v__get_bar_size():
		res = get_absolute(slider_v_cur.size, get_stage_height()) - slider_v_cur.spacing * 2
		if slider_v_cur.buttons:
			res -= 2 * (slider_v_cur.button_size + slider_v_cur.spacing)
		return res
	
	def slider_v__update(local_y):
		bar = slider_v.get_bar_size()
		slider = bar * slider_v_cur.slider_size
		if bar <= slider:
			value = 0
		else:
			value = (local_y - slider / 2) / (bar - slider)
		slider_v_cur['value'] = in_bounds(value, 0, 1)
	
	
	build_object('slider_v')
	slider_v.data = {}


init:
	style slider_v_button is textbutton
	
	# implementaion detail, don't touch
	style slider_v_over_btn is button:
		ground im.rect('#000')
		hover  im.rect('#000')
		corner_sizes 0
		alpha 0.01


screen slider_v(name):
	$ slider_v_cur = slider_v.data[screen.name]
	
	xsize slider_v_cur.button_size
	ysize slider_v_cur.size
	alpha 1 if slider_v_cur.slider_size < 1 else 0
	
	vbox:
		# [spacing, btn], spacing, bar, spacing, [btn, spacing]
		
		if slider_v_cur.buttons:
			null ysize slider_v_cur.spacing
			
			textbutton '↑':
				style slider_v_cur.button_style
				text_size slider_v_cur.button_size - 10
				size      slider_v_cur.button_size
				action slider_v.add_value(-0.1)
		
		null ysize slider_v_cur.spacing
		
		
		python:
			screen_tmp = SimpleObject()
			screen_tmp.xsize = slider_v_cur.button_size
			screen_tmp.ysize = slider_v.get_bar_size()
			screen_tmp.hover_full_size = screen_tmp.ysize - slider_v_cur.hover_spacing * 2
			
			screen_tmp.hover_start = screen_tmp.hover_full_size * slider_v_cur.value * (1 - slider_v_cur.slider_size) + slider_v_cur.hover_spacing
			screen_tmp.hover_size = screen_tmp.hover_full_size * slider_v_cur.slider_size
		
		image slider_v_cur.ground:
			corner_sizes -1
			xsize screen_tmp.xsize
			ysize screen_tmp.ysize
			
			null:
				clipping True
				ypos screen_tmp.hover_start
				ysize screen_tmp.hover_size
				
				image slider_v_cur.hover:
					corner_sizes -1
					ypos -screen_tmp.hover_start
					xsize screen_tmp.xsize
					ysize screen_tmp.ysize
			
			# separate button for case <ground with transparent parts>
			button:
				style 'slider_v_over_btn'
				xsize screen_tmp.xsize
				ysize screen_tmp.ysize
				
				unhovered 'slider_v_cur.hovered = False'
				action ['slider_v_cur.hovered = True', slider_v.update(get_local_mouse()[1])]
		
		python:
			if not slider_v_cur.scrolling and slider_v_cur.hovered and get_mouse_down():
				slider_v_cur.scrolling = True
				slider_v_cur.y = get_mouse()[1] - get_local_mouse()[1]
			if not get_mouse_down():
				slider_v_cur.scrolling = False
			
			if slider_v_cur.scrolling:
				slider_v.update(get_mouse()[1] - slider_v_cur.y)
		
		
		null ysize slider_v_cur.spacing
		
		if slider_v_cur.buttons:
			textbutton '↓':
				style slider_v_cur.button_style
				text_size slider_v_cur.button_size - 10
				size      slider_v_cur.button_size
				action slider_v.add_value(+0.1)
			
			null ysize slider_v_cur.spacing
