init -10 python:
	def panels__copy_code():
		sprites.effects_to_end()
		
		common_res = ''
		
		for sprite in sprites.list.copy():
			hided = sprite.alpha <= 0
			if hided: continue
			
			for spr in sprite.get_all_sprites():
				image = spr.res_image or spr.image
				if image:
					break
			else:
				image = im.rect('#888')
			image_xsize, image_ysize = get_image_size(image)
			
			for tmp_name in sprite_panel.place_names.values():
				if sprite_panel.place_name_is(tmp_name, sprite):
					place_name = tmp_name
					break
			else:
				place_name = None
			
			
			params = {}
			if sprite.alpha < 1:
				params['alpha'] = sprite.alpha
			
			if sprite.rotate:
				params['rotate'] = sprite.rotate
			
			if sprite.xzoom != 1:
				params['xzoom'] = sprite.xzoom
			if sprite.yzoom != 1:
				params['yzoom'] = sprite.yzoom
			
			if place_name is None:
				params['xanchor'] = sprite.xanchor
				params['yanchor'] = sprite.yanchor
				
				params['xpos'] = sprite.xpos
				params['ypos'] = sprite.ypos
			
			if sprite.xsize not in (None, image_xsize):
				params['xsize'] = sprite.xsize
			if sprite.ysize not in (None, image_ysize):
				params['ysize'] = sprite.ysize
			
			abs_xsizecrop = get_absolute(sprite.xsizecrop, image_xsize)
			abs_ysizecrop = get_absolute(sprite.ysizecrop, image_ysize)
			if (sprite.xcrop, sprite.ycrop, abs_xsizecrop, abs_ysizecrop) != (0, 0, image_xsize, image_ysize):
				params['crop'] = (sprite.xcrop, sprite.ycrop, sprite.xsizecrop, sprite.ysizecrop)
			
			changed_props = []
			if params:
				def has_prop(name):
					return name in params
				
				while True:
					tag = '__tmp_spr__%s' % random.randint(0, 1000000)
					for spr in sprites.list:
						if spr.tag == tag:
							break
					else:
						break
				
				sprites.show(sprite.sprite_name.split(' ') + ['at', 'None', 'as', tag], ())
				tmp_sprite = sprites.list[-1]
				tmp_sprite.update()
				
				
				def full_equal(a, b):
					return a == b and type(a) is type(b)
				
				def set_prop(name, value = _undefined):
					if value is _undefined:
						value = params[name]
					
					names = get_atl_props(name)
					
					if len(names) == 1:
						add = not full_equal(tmp_sprite.get(name), value)
					else:
						if type(value) not in (tuple, list):
							value = (value, value)
						if len(value) != len(names):
							return
						
						add = False
						for _name, _value in zip(names, value):
							if not full_equal(tmp_sprite.get(_name), _value):
								add = True
								break
					
					if add:
						if type(value) in (tuple, list):
							value = tuple(round(v, 3) if type(v) is float else v for v in value)
							
							if len(value) == 2 and full_equal(value[0], value[1]):
								value = value[0] # (50, 50) -> 50
						
						else:
							if type(value) is float:
								value = round(value, 3)
						
						changed_props.append(name + ' ' + str(value))
				
				
				if has_prop('xzoom') and has_prop('yzoom'):
					set_prop('zoom', (params['xzoom'], params['yzoom']))
				else:
					if has_prop('xzoom'):
						set_prop('xzoom')
					elif has_prop('yzoom'):
						set_prop('yzoom')
				
				if has_prop('xanchor') and full_equal(params['xanchor'], params['xpos']):
					params['xalign'] = params['xpos']
				if has_prop('yanchor') and full_equal(params['yanchor'], params['ypos']):
					params['yalign'] = params['ypos']
				
				if has_prop('xalign') and has_prop('yalign'):
					if not full_equal(params['xalign'], 0.5) or not full_equal(params['yalign'], 1.0):
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
				
				sprites.list.pop() # remove tmp_sprite
			
			
			is_scene = sprite is sprites.scene
			res = '%s %s' % ('scene' if is_scene else 'show', sprite.sprite_name)
			
			if sprites.get_tag_of_image_name(sprite.sprite_name) != sprite.tag:
				res += ' as ' + sprite.tag
			
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
	
	def panels__fix_size():
		w, h = get_stage_size()
		k = get_from_hard_config('window_w_div_h', float)
		
		if k > 1.777:
			spr_h = h - panels.ysize
			spr_w = round(spr_h * k)
			panels.xsize = w - spr_w
		else:
			spr_w = w - panels.xsize
			spr_h = round(spr_w / k)
			panels.ysize = h - spr_h
	
	
	build_object('panels')
	
	panels.back = im.rect('#FFFFF8')
	panels.line = im.rect('#888')
	panels.indent = 15
	panels.small_indent = 5
	panels.info_text_size = 22
	
	panels.xsize = 256 # right
	panels.ysize = 144 # bottom
	panels.fix_size() # use param 'window_w_div_h' from <params.conf> to fix it
	
	panels.small_btn_xsize = 40

init 1 python:
	config.keymap['copy_code'] = ['Ctrl_C']
	config.keymap['hide_panels'] = ['H']
	config.keymap['reset_sprite'] = ['R']
	config.underlay.append(renpy.Keymap(
		copy_code = panels.copy_code,
		hide_panels = ToggleVariable('panels.hide'),
		reset_sprite = images.reset_sprite,
	))
	panels.hide = False
	
	def check_pause(screen_name):
		if screen_name == 'pause' and panels.hide:
			panels.hide = False
	signals.add('show_screen', check_pause)
