init -10 python:
	def panel__toggle_hide():
		panel.hide = not panel.hide
	
	def panel__copy_code():
		common_res = ''
		
		for sprite in sprites.list:
			data = sprite.new_data
			hided = data.alpha < 0 # <, not <=
			if hided: continue
			
			for tmp_data in sprite.get_all_data():
				image = tmp_data.res_image or tmp_data.image
				if image:
					break
			else:
				image = im.rect('#888')
			image_xsize, image_ysize = get_image_size(image)
			
			for short_name in panel.place_names:
				if panel.place_name_is(short_name, sprite):
					place_name = panel.place_names[short_name]
					break
			else:
				place_name = None
			
			
			params = {}
			if data.alpha < 1:
				params['alpha'] = data.alpha
			
			if data.rotate:
				params['rotate'] = data.rotate
			
			if place_name is None:
				params['xanchor'] = data.xanchor
				params['yanchor'] = data.yanchor
				
				params['xpos'] = data.xpos
				params['ypos'] = data.ypos
			
			if data.xsize is not None and data.xsize != image_xsize:
				params['xsize'] = data.xsize
			if data.ysize is not None and data.ysize != image_ysize:
				params['ysize'] = data.ysize
			
			abs_xsizecrop = get_absolute(data.xsizecrop, image_xsize)
			abs_ysizecrop = get_absolute(data.ysizecrop, image_ysize)
			if (data.xcrop, data.ycrop, abs_xsizecrop, abs_ysizecrop) != (0, 0, image_xsize, image_ysize):
				params['crop'] = (data.xcrop, data.ycrop, data.xsizecrop, data.ysizecrop)
			
			changed_props = []
			if params:
				def has_prop(name):
					return name in params
				
				while True:
					tag = '__tmp_spr__' + str(random.randint(0, 1000000))
					for spr in sprites.list:
						if spr.tag == tag:
							break
					else:
						break
				sprites.show(sprite.sprite_name.split(' ') + ['at', 'None', 'as', tag], [])
				inited_data = sprites.list[-1].new_data
				
				def full_equal(a, b):
					return a == b and type(a) is type(b)
				
				def set_prop(name, value = _undefined):
					if value is _undefined:
						value = params[name]
					
					names = get_atl_props(name)
					
					if len(names) == 1:
						add = not full_equal(inited_data[name], value)
					else:
						if type(value) not in (tuple, list):
							if len(names) != 2:
								return
							value = [value, value]
						if len(value) != len(names):
							return
						
						add = False
						for i, tmp_name in enumerate(names):
							if not full_equal(inited_data[tmp_name], value[i]):
								add = True
								break
					
					if add:
						if type(value) in (tuple, list):
							for i in range(len(value)):
								if type(value[i]) is float:
									value[i] = round(value[i], 3)
						else:
							if type(value) is float:
								value = round(value, 3)
						
						if type(value) in (tuple, list) and len(value) == 2 and full_equal(value[0], value[1]):
							value = value[0] # (50, 50) -> 50
						
						changed_props.append(name + ' ' + str(value))
				
				
				if has_prop('xanchor') and full_equal(params['xanchor'], params['xpos']):
					params['xalign'] = params['xpos']
				if has_prop('yanchor') and full_equal(params['yanchor'], params['ypos']):
					params['yalign'] = params['ypos']
				
				if has_prop('xalign') and has_prop('yalign'):
					set_prop('align', (params['xalign'], params['yalign']))
				else:
					if has_prop('xalign'):
						set_prop('xalign')
					elif has_prop('xpos'):
						set_prop('xpos')
						set_prop('xanchor')
					if has_prop('yalign'):
						set_prop('yalign')
					elif has_prop('ypos'):
						set_prop('ypos')
						set_prop('yanchor')
				
				if has_prop('xsize') and has_prop('ysize'):
					set_prop('size', (params['xsize'], params['ysize']))
				else:
					if has_prop('xsize'):
						set_prop('xsize')
					if has_prop('ysize'):
						set_prop('ysize')
				
				for prop in ('crop', 'rotate', 'alpha'):
					if has_prop(prop):
						set_prop(prop)
				
				sprites.list.pop(-1) # remove tmp sprite with inited_data
			
			
			res = 'show ' + sprite.sprite_name
			if place_name is not None:
				if place_name != 'center' or changed_props:
					res += ' at ' + place_name
			
			if changed_props:
				res += ':\n'
			for changed_prop in changed_props:
				res += '\t' + changed_prop + '\n'
			
			common_res += res + '\n'
		
		if not set_clipboard_text(common_res):
			notification.out('Error on set text in clipboard')
	
	def panel__fix_size():
		w, h = get_stage_size()
		k = get_from_hard_config("window_w_div_h", float)
		
		if k > 1.777:
			spr_h = h - panel.ysize
			spr_w = int(round(spr_h * k))
			panel.xsize = w - spr_w
		else:
			spr_w = w - panel.xsize
			spr_h = int(round(spr_w / k))
			panel.ysize = h - spr_h
	
	
	build_object('panel')
	
	panel.back = im.rect('#FFFFF8')
	panel.line = im.rect('#888')
	panel.indent = 15
	panel.small_indent = 5
	panel.info_text_size = 22
	
	panel.xsize = 256 # right
	panel.ysize = 144 # bottom
	panel.fix_size() # use param "window_w_div_h" from <params.conf> to fix it
	
	panel.small_btn_xsize = 40

init 1 python:
	config.keymap['copy_code'] = ['Ctrl_C']
	config.keymap['hide_panels'] = ['H']
	config.keymap['reset_sprite'] = ['R']
	config.underlay.append(renpy.Keymap(
		copy_code = panel.copy_code,
		hide_panels = panel.toggle_hide,
		reset_sprite = images.reset_sprite,
	))
	panel.hide = False
	
	def check_pause(screen_name):
		if screen_name == 'pause' and panel.hide:
			panel.toggle_hide()
	signals.add('show_screen', check_pause)
