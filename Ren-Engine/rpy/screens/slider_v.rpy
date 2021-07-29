init -1000 python:
	sliders_v = {}
	
	def slider_v_init(name, length, page_length, ground = None, hover = None, value = 0.0, button_size = 25, buttons = True):
		sliders_v[name] = {
			'length': length,
			'page_length': page_length,
			'ground': ground or vbar_ground,
			'hover': hover or vbar_hover,
			'value': value,
			'button_size': button_size,
			'buttons': buttons
		}
	def slider_v_change(name, **args):
		t = sliders_v[name]
		
		for key in args:
			if key not in t:
				out_msg('slider_v_change', 'Unexpected param <' + key + '>')
			else:
				t[key] = args[key]
	
	def slider_v_get_value(name):
		return sliders_v[name]['value']
	
	slider_v_current_name = None
	def slider_v_set(name):
		global slider_v_current_name
		if name in sliders_v:
			slider_v_current_name = name
		else:
			out_msg('slider_v_set', 'Name <' + str(name) + '> is not inited')
	
	def slider_v_add_value(k):
		slider_v_cur['value'] = in_bounds(slider_v_cur['value'] + k * tmp_scroll, 0.0, 1.0)
	
	def slider_v_update(local_y):
		sh = get_stage_height()
		length = get_absolute(slider_v_cur['page_length'], sh) - (2 * slider_v_cur['button_size'] if slider_v_cur['buttons'] else 0)
		scroll = length * tmp_scroll
		if length <= scroll:
			value = 0
		else:
			value = (local_y - scroll / 2) / (length - scroll)
		
		slider_v_cur['value'] = in_bounds(value, 0.0, 1.0)
	
	
	slider_v_scrolling = False
	slider_v_hovered = False
	slider_v_scroll_y = 0


screen slider_v:
	python:
		if slider_v_current_name is None:
			out_msg('Screen slider_v', 'For use SliderV you must call slider_v_set(name)')
			slider_v_cur = None
			slider_v_size = (0, 0)
		else:
			slider_v_cur = sliders_v[slider_v_current_name]
			slider_v_current_name = None
			slider_v_size = (slider_v_cur['button_size'], slider_v_cur['length'])
	
	size slider_v_size
	
	if slider_v_cur is not None:
		vbox:
			align (0.97, 0.5)
			xsize 35
			
			if slider_v_cur['buttons']:
				textbutton '/\\':
					color 0xFFFFFF
					text_size slider_v_cur['button_size'] - 5
					xalign 0.5
					size (slider_v_cur['button_size'], slider_v_cur['button_size'])
					action slider_v_add_value(-0.5)
			
			python:
				tmp_length = get_absolute(slider_v_cur['length'], get_stage_height())
				tmp_scroll = get_absolute(slider_v_cur['page_length'], get_stage_height()) / float(tmp_length)
				
				tmp_image = im.bar(
					slider_v_cur['value'] * (1 - tmp_scroll) + tmp_scroll,
					slider_v_cur['value'] * (1 - tmp_scroll),
					vertical = True, ground = slider_v_cur['ground'], hover = slider_v_cur['hover'])
			
			button:
				ground tmp_image
				hover  tmp_image
				xalign 0.5
				xsize slider_v_cur['button_size']
				ysize get_absolute(slider_v_cur['page_length'], get_stage_height()) -
				        (2 * slider_v_cur['button_size'] if slider_v_cur['buttons'] else 0)
				
				unhovered SetVariable('slider_v_hovered', False)
				action [SetVariable('slider_v_hovered', True), slider_v_update(get_local_mouse()[1])]
			
			python:
				if not slider_v_scrolling and slider_v_hovered and get_mouse_down():
					slider_v_scrolling = True
					slider_v_y = get_mouse()[1] - get_local_mouse()[1]
				if not get_mouse_down():
					slider_v_scrolling = False
				
				if slider_v_scrolling:
					slider_v_update(get_mouse()[1] - slider_v_y)
			
			if slider_v_cur['buttons']:
				textbutton '\\/':
					color 0xFFFFFF
					text_size slider_v_cur['button_size'] - 5
					xalign 0.5
					size (slider_v_cur['button_size'], slider_v_cur['button_size'])
					action slider_v_add_value(+0.5)

