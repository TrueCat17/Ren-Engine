init 1 python:
	def props__get_prop(prop):
		if panel.cur_sprite is None:
			return ''
		
		value = panel.cur_sprite.new_data[prop]
		if type(value) is float:
			value = round(value, 3)
		
		if value is None:
			image = props.get_cur_image()
			value = (get_image_width if prop == 'xsize' else get_image_height)(image)
		
		return prop + ': ' + str(value)
	
	def props__get_align():
		x = panel.cur_sprite.new_data.xanchor if panel.cur_sprite else 0.0
		y = panel.cur_sprite.new_data.yanchor if panel.cur_sprite else 0.0
		
		x_is_int = type(x) is int
		y_is_int = type(y) is int
		if x_is_int or y_is_int:
			image = props.get_cur_image()
			if x_is_int:
				x = float(x) / get_image_width(image)
			if y_is_int:
				y = float(y) / get_image_height(image)
		return in_bounds(x, -0.25, 1.25), in_bounds(y, -0.25, 1.25)
	
	def props__change(d):
		prop = props.cur_prop
		data = panel.cur_sprite.new_data
		
		if prop == 'alpha' and data.alpha < 0: # hided, dont edit directly
			return
		
		if data[prop] is None: # [x/y]size
			image = props.get_cur_image()
			data[prop] = (get_image_width if prop == 'xsize' else get_image_height)(image)
		
		data[prop] += d
		
		if prop == 'alpha':
			data.alpha = in_bounds(data.alpha, 0.0, 1.0)
	
	def props__convert_int_float():
		prop = props.cur_prop
		v = panel.cur_sprite.new_data[prop]
		
		image = props.get_cur_image()
		
		xfull, yfull = get_stage_size() if prop in props.stage_sized else get_image_size(image)
		full = xfull if prop[0] == 'x' else yfull
		
		if type(v) is float:
			v = int(v * full)
		else:
			v = float(v) / full
		
		panel.cur_sprite.new_data[prop] = v
	
	def props__get_cur_image():
		for data in panel.cur_sprite.get_all_data():
			image = data.res_image or data.image
			if image:
				return image
		return im.rect('#888')
	
	def props__on_sprite_click():
		props.mouse_down = True
		props.prev_mouse = get_mouse()
	def props__update_mouse():
		if props.mouse_down and not get_mouse_down():
			props.mouse_down = False
		if not props.mouse_down:
			return
		if not panel.cur_sprite or panel.cur_sprite.new_data.alpha <= 0:
			return
		
		xzoom = 1.0 if panel.hide else (1.0 * panel.sprites_xsize / get_stage_width())
		yzoom = 1.0 if panel.hide else (1.0 * panel.sprites_ysize / get_stage_height())
		
		mouse = get_mouse()
		dx = (mouse[0] - props.prev_mouse[0]) / xzoom
		dy = (mouse[1] - props.prev_mouse[1]) / yzoom
		
		data = panel.cur_sprite.new_data
		if type(data.xpos) is int:
			data.xpos += int(math.ceil(dx))
		else:
			data.xpos += dx / get_stage_width()
		
		if not hotkeys.ctrl:
			if type(data.ypos) is int:
				data.ypos += int(math.ceil(dy))
			else:
				data.ypos += dy / get_stage_height()
		
		props.prev_mouse = mouse
	
	
	build_object('props')
	
	props.int_changes = [1, 10, 100]
	props.float_changes = [0.01, 0.1, 0.5]
	
	props.cur_prop = 'xpos'
	
	props.groups = [
		('xanchor', 'yanchor'),
		('xcrop', 'ycrop', 'xsizecrop', 'ysizecrop'),
		('xsize', 'ysize', 'xzoom', 'yzoom'),
		('xpos', 'ypos', 'alpha', 'rotate'),
	]
	
	props.cant_convert_int_float = ['xzoom', 'yzoom', 'alpha', 'rotate']
	props.stage_sized = ['xpos', 'ypos', 'xsize', 'ysize'] # (if float) use get_stage_size, other props - get_image_size
	
	
	style.action_btn = Style(style.textbutton)
	style.action_btn.size = (115, 40)
	style.action_btn.ground = im.round_rect('#44F', 115, 40, 4)
	style.action_btn.hover  = im.round_rect('#F80', 115, 40, 4)
	style.action_btn.color = 0xFFFFFF
	style.action_btn.outlinecolor = 0x000000
	
	style.prop_btn = Style(style.textbutton)
	style.prop_btn.xsize = 110
	style.prop_btn.ground = im.round_rect('#08F', 110, style.prop_btn.ysize, 4)
	style.prop_btn.hover  = im.round_rect('#F80', 110, style.prop_btn.ysize, 4)
	
	style.prop_change_btn = Style(style.textbutton)
	style.prop_change_btn.xsize = 55
	style.prop_change_btn.ground = im.round_rect('#44F', 55, style.prop_change_btn.ysize, 4)
	style.prop_change_btn.hover  = im.round_rect('#F80', 55, style.prop_change_btn.ysize, 4)
	
	props.convert_btn_xsize = style.prop_change_btn.xsize * 2 + panel.small_indent
	
	
	props.anchor_bg = im.rect('#BBB')
	props.anchor_size = style.prop_btn.ysize * 2 + panel.small_indent
	props.anchor_point = im.rect('#F40')
	props.anchor_point_size = 10

screen props_panel:
	alpha 0 if panel.hide else 1
	
	image panel.back:
		yalign 1.0
		xsize panel.sprites_xsize
		ysize panel.ysize
		
		image panel.line:
			size (1, panel.ysize)
			xalign 1.0
		
		vbox:
			alpha 1 if panel.cur_sprite else 0
			xpos panel.indent
			spacing panel.small_indent
			yalign 0.5
			
			textbutton (_('Copy code') + ' \n(Ctrl+C)'):
				style 'action_btn'
				action panel.copy_code
			textbutton (_('Hide Panels') + '\n(H)'):
				style 'action_btn'
				action panel.toggle_hide
			textbutton (_('Reset Sprite') + '\n(R)'):
				style 'action_btn'
				action images.reset_sprite
		
		hbox:
			alpha 1 if panel.cur_sprite else 0
			align 0.5
			spacing panel.indent
			
			for group in props.groups:
				vbox:
					spacing panel.small_indent
					
					for prop in group:
						textbutton props.get_prop(prop):
							style 'prop_btn'
							ground style.prop_btn['hover' if props.cur_prop == prop else 'ground']
							hover  style.prop_btn.hover
							action 'props.cur_prop = prop'
					
					if group == ('xanchor', 'yanchor'):
						image props.anchor_bg:
							xalign 0.5
							size props.anchor_size
							
							image props.anchor_point:
								size props.anchor_point_size
								align props.get_align()
		
		vbox:
			alpha 1 if panel.cur_sprite else 0
			spacing panel.small_indent
			xpos panel.sprites_xsize - panel.indent
			xanchor 1.0
			yalign 0.5
			
			$ prop_value = panel.cur_sprite and panel.cur_sprite.new_data[props.cur_prop]
			for i in (props.float_changes if type(prop_value) is float else props.int_changes):
				hbox:
					spacing panel.small_indent
					
					textbutton ('-' + str(i)):
						style 'prop_change_btn'
						action props.change(-i)
					textbutton ('+' + str(i)):
						style 'prop_change_btn'
						action props.change(+i)
			
			textbutton 'int ↔ float':
				alpha 0 if props.cur_prop in props.cant_convert_int_float else 1
				style 'prop_change_btn'
				xsize props.convert_btn_xsize
				ground im.round_rect('#44F', props.convert_btn_xsize, style.prop_change_btn.ysize, 4)
				hover  im.round_rect('#F80', props.convert_btn_xsize, style.prop_change_btn.ysize, 4)
				action props.convert_int_float
