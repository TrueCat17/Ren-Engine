init 1 python:
	def props__get_prop(prop):
		if sprite_panel.cur_sprite is None:
			return ''
		
		value = sprite_panel.cur_sprite[prop]
		if type(value) is float:
			value = round(value, 3)
		
		if value is None:
			image = props.get_cur_image()
			value = (get_image_width if prop == 'xsize' else get_image_height)(image)
		
		return '%s: %s' % (prop, value)
	
	def props__get_anchor():
		cur_sprite = sprite_panel.cur_sprite
		if not cur_sprite:
			return 0
		
		x = cur_sprite.xanchor
		y = cur_sprite.yanchor
		
		x_is_int = type(x) is int
		y_is_int = type(y) is int
		if x_is_int or y_is_int:
			image = props.get_cur_image()
			if x_is_int:
				x /= get_image_width(image)
			if y_is_int:
				y /= get_image_height(image)
		return in_bounds(x, -0.25, 1.25), in_bounds(y, -0.25, 1.25)
	
	def props__change(d):
		prop = props.cur_prop
		sprite = sprite_panel.cur_sprite
		
		if prop == 'alpha' and sprite.alpha < 0: # hided, dont edit directly
			return
		
		if sprite[prop] is None: # [x/y]size
			image = props.get_cur_image()
			sprite[prop] = (get_image_width if prop == 'xsize' else get_image_height)(image)
		
		sprite[prop] += d
		
		if prop == 'alpha':
			sprite.alpha = in_bounds(sprite.alpha, 0.0, 1.0)
	
	def props__convert_int_float():
		prop = props.cur_prop
		sprite = sprite_panel.cur_sprite
		
		image = props.get_cur_image()
		
		stage_size = (config.width or get_stage_width(), config.height or get_stage_height())
		xfull, yfull = stage_size if prop in props.stage_sized else get_image_size(image)
		full = xfull if prop[0] == 'x' else yfull
		
		v = sprite[prop]
		if v is None and prop in ('xsize', 'ysize'):
			v = (get_image_width if prop == 'xsize' else get_image_height)(image)
		
		if type(v) is float:
			v = round(v * full)
		else:
			v /= full
		
		sprite[prop] = v
	
	def props__get_cur_image():
		for spr in sprite_panel.cur_sprite.get_all_sprites():
			image = spr.res_image or spr.image
			if image:
				return image
		return im.rect('#888')
	
	def props__on_sprite_click():
		props.prev_mouse = get_mouse()
	def props__update_mouse():
		if not get_mouse_down() or not props.prev_mouse:
			return
		
		sprite = sprite_panel.cur_sprite
		if not sprite or sprite.alpha <= 0:
			return
		
		sw, sh = get_stage_size()
		xzoom = 1.0 if panels.hide else (panels.sprites_xsize / sw)
		yzoom = 1.0 if panels.hide else (panels.sprites_ysize / sh)
		
		mouse = get_mouse()
		dx = (mouse[0] - props.prev_mouse[0]) / xzoom
		dy = (mouse[1] - props.prev_mouse[1]) / yzoom
		props.prev_mouse = mouse
		
		if type(sprite.xpos) is int:
			if config.width:
				dx *= config.width / sw
			dx = round(dx)
		else:
			dx /= sw
		sprite.xpos += dx
		
		if not hotkeys.ctrl:
			if type(sprite.ypos) is int:
				if config.height:
					dy *= config.height / sh
				dy = round(dy)
			else:
				dy /= sh
			sprite.ypos += dy
	
	
	build_object('props')
	
	props.int_changes = (1, 10, 100)
	props.float_changes = (0.01, 0.1, 0.5)
	
	props.cur_prop = 'xpos'
	
	props.groups = (
		('xanchor', 'yanchor'),
		('xcrop', 'ycrop', 'xsizecrop', 'ysizecrop'),
		('xsize', 'ysize', 'xzoom', 'yzoom'),
		('xpos', 'ypos', 'alpha', 'rotate'),
	)
	
	props.cant_convert_int_float = ('xzoom', 'yzoom', 'alpha', 'rotate')
	props.stage_sized = ('xpos', 'ypos', 'xsize', 'ysize') # (if float) use get_stage_size, other props - get_image_size

init 1:
	style action_btn is textbutton:
		size (115, 40)
		ground im.round_rect('#44F', 20, 20, 4)
		hover  im.round_rect('#F80', 20, 20, 4)
		color '#FFF'
		outlinecolor 0
	
	style prop_btn is action_btn:
		size (110, 25)
		ground im.round_rect('#08F', 20, 20, 4)
		outlinecolor None
	
	style prop_change_btn is action_btn:
		size (55, 25)
		outlinecolor None

init 2 python:
	props.convert_btn_xsize = style.prop_change_btn.xsize * 2 + panels.small_indent
	
	props.anchor_bg = im.rect('#BBB')
	props.anchor_size = style.prop_btn.ysize * 2 + panels.small_indent
	props.anchor_point = im.rect('#F40')
	props.anchor_point_size = 10


screen props_panel:
	alpha 0 if panels.hide else 1
	
	image panels.back:
		yalign 1.0
		xsize panels.sprites_xsize
		ysize panels.ysize
		
		image panels.line:
			xsize 1
			ysize panels.ysize
			xalign 1.0
		
		vbox:
			alpha 1 if sprite_panel.cur_sprite else 0
			xpos panels.indent
			spacing panels.small_indent
			yalign 0.5
			
			textbutton (_('Copy code') + '\n(Ctrl+C)'):
				style 'action_btn'
				action panels.copy_code
			textbutton (_('Hide Panels') + '\n(H)'):
				style 'action_btn'
				action panels.toggle_hide
			textbutton (_('Reset Sprite') + '\n(R)'):
				style 'action_btn'
				action images.reset_sprite
		
		hbox:
			alpha 1 if sprite_panel.cur_sprite else 0
			align 0.5
			spacing panels.indent
			
			for group in props.groups:
				vbox:
					spacing panels.small_indent
					
					for prop in group:
						textbutton props.get_prop(prop):
							style 'prop_btn'
							selected props.cur_prop == prop
							action 'props.cur_prop = prop'
					
					if group == ('xanchor', 'yanchor'):
						image props.anchor_bg:
							xalign 0.5
							size props.anchor_size
							
							image props.anchor_point:
								size props.anchor_point_size
								align props.get_anchor()
		
		vbox:
			alpha 1 if sprite_panel.cur_sprite else 0
			spacing panels.small_indent
			xpos panels.sprites_xsize - panels.indent
			xanchor 1.0
			yalign 0.5
			
			$ prop_value = sprite_panel.cur_sprite and sprite_panel.cur_sprite[props.cur_prop]
			for i in (props.float_changes if type(prop_value) is float else props.int_changes):
				hbox:
					spacing panels.small_indent
					
					textbutton ('-%s' % i):
						style 'prop_change_btn'
						action props.change(-i)
					textbutton ('+%s' % i):
						style 'prop_change_btn'
						action props.change(+i)
			
			textbutton 'int ↔ float':
				alpha 0 if props.cur_prop in props.cant_convert_int_float else 1
				style 'prop_change_btn'
				xsize props.convert_btn_xsize
				action props.convert_int_float
