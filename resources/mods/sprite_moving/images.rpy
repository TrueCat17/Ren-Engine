init python:
	
	def images__update(dir_path):
		if images.sprites is None:
			images.sprites = {}
		
		for path, ds, fs in os.walk(dir_path):
			for f in fs:
				if not f.endswith('.rpy'): continue
				
				for l in open(os.path.join(path, f), 'rb'):
					l = str(l, 'utf8').strip()
					if not l.startswith('image '): continue
					
					l = l[6:] # 6 = len('image ')
					if l.endswith(':'):
						l = l[:-1]
					if '=' in l:
						l = l[:l.index('=')]
					l = l.strip()
					
					if ' ' in l:
						name = l[:l.index(' ')]
					else:
						name = l
					
					if not image_was_registered(l): continue
					
					if name not in images.sprites:
						images.sprites[name] = []
					images.sprites[name].append(l)
		
		images.names = sorted(images.sprites.keys())
		images.set_name(images.names and images.names[0])
		
		images.max_name_len = 0
		for name in images.names:
			images.max_name_len = max(images.max_name_len, len(name))
		one_symbol_xsize = images.sprite_name_btn_text_size / 1.5
		images.name_xsize = int(one_symbol_xsize * images.max_name_len * 1.2)
		
		images.name_count = images.xsize // (images.name_xsize + images.indent)
		images.all_names = images.name_count >= len(images.names)
		if not images.all_names:
			images.name_count -= 2 # place for <prev> and <next> buttons
		
		images.first_name_index = 0
		images.max_first_name_index = len(images.names) - images.name_count
		
		style.sprite_name_btn = Style(style.textbutton)
		style.sprite_name_btn.ground = im.round_rect('#F80', images.name_xsize, images.sprite_name_btn_ysize, 4)
		style.sprite_name_btn.hover  = im.round_rect('#F00', images.name_xsize, images.sprite_name_btn_ysize, 4)
		style.sprite_name_btn.size = (images.name_xsize, images.sprite_name_btn_ysize)
		style.sprite_name_btn.text_size = images.sprite_name_btn_text_size
		style.sprite_name_btn.color = 0x00FF00
		style.sprite_name_btn.outlinecolor = 0x000000
		
		style.change_sprite_name_index_btn = Style(style.sprite_name_btn)
		style.change_sprite_name_index_btn.xsize = panel.small_btn_xsize
		style.change_sprite_name_index_btn.ground = im.round_rect('#F80', panel.small_btn_xsize, style.change_sprite_name_index_btn.ysize, 4)
		style.change_sprite_name_index_btn.hover  = im.round_rect('#F00', panel.small_btn_xsize, style.change_sprite_name_index_btn.ysize, 4)
	
	def images__set_name(name):
		changed = images.cur_name != name
		if changed:
			images.set_image(None)
		images.cur_name = name or ''
		images.images = images.sprites[name] if name else []
		
		free_ysize = images.ysize - (images.sprite_name_btn_ysize + images.indent * 2) - (panel.info_text_size + images.indent * 2)
		images.image_count = (free_ysize + images.small_indent) // (style.image_name_btn.ysize + images.small_indent)
		images.all_images = images.image_count >= len(images.images)
		if not images.all_images:
			images.image_count -= 2 # place for <prev> and <next> buttons
		
		if changed:
			images.first_image_index = 0
		images.max_first_image_index = len(images.images) - images.image_count
	
	def images__set_image(image):
		images.cur_image = image
		images.sprite = image and Sprite(get_image(image), center.actions, [], None)
	
	
	def images__add_sprite(image):
		cmd = image.split(' ')
		
		if hotkeys.ctrl:
			name = cmd[0]
			i = 0
			while True:
				i += 1
				as_name = name + '-' + str(i)
				
				for spr in sprites.list:
					if spr.as_name == as_name:
						break
				else:
					break
			cmd += ['as', as_name]
		
		sprites.show(cmd, [])
		panel.cur_sprite = sprites.list[-1]
		panel.update_sprite_count()
	
	def images__reset_sprite():
		sprite = panel.cur_sprite
		index = sprites.list.index(sprite)
		
		cmd = sprite.sprite_name.split(' ')
		cmd += ['at', 'center']
		cmd += ['as', sprite.as_name]
		cmd += ['with', panel.default_effect_code]
		
		sprites.show(cmd, [])
		panel.cur_sprite = sprites.list[index]
	
	
	def images__fix_sprite_real_size():
		k = get_from_hard_config("window_w_div_h", float)
		if k > 1.777:
			images.sprite_real_xsize = int(round(images.sprite_real_ysize * k))
		else:
			images.sprite_real_ysize = int(round(images.sprite_real_xsize / k))
	
	def images__update_sprite_size():
		images.sprite_xsize = config.width or get_stage_width()
		images.sprite_ysize = config.height or get_stage_height()
		
		images.sprite_xzoom = images.sprite_real_xsize / images.sprite_xsize
		images.sprite_yzoom = images.sprite_real_ysize / images.sprite_ysize
	
	
	build_object('images')
	
	images.back = im.rect('#FFF')
	images.xsize = None
	images.ysize = None
	images.border = im.rect('#00F')
	images.border_size = 3
	images.sprite_back = im.rect('#888')
	
	images.sprite_real_xsize = 1920 // 4
	images.sprite_real_ysize = 1080 // 4
	images.fix_sprite_real_size()
	images.update_sprite_size()
	signals.add('resized_stage', images.update_sprite_size)
	
	images.indent = 15
	images.small_indent = 5
	images.first_image_index = 0
	
	images.sprite_name_btn_ysize = style.textbutton.ysize
	images.sprite_name_btn_text_size = 20
	
	style.image_name_btn = Style(style.textbutton)
	style.image_name_btn.xsize = 250
	style.image_name_btn.ground = im.round_rect('#08F', 250, style.image_name_btn.ysize, 4)
	style.image_name_btn.hover  = im.round_rect('#F00', 250, style.image_name_btn.ysize, 4)
	style.image_name_btn.text_size = 18
	style.image_name_btn.color = 0x00FF00
	style.image_name_btn.outlinecolor = 0x000000
	
	style.change_image_name_index_btn = Style(style.image_name_btn)
	style.change_image_name_index_btn.xsize = panel.small_btn_xsize
	style.change_image_name_index_btn.ground = im.round_rect('#08F', panel.small_btn_xsize, style.change_image_name_index_btn.ysize, 4)
	style.change_image_name_index_btn.hover  = im.round_rect('#F00', panel.small_btn_xsize, style.change_image_name_index_btn.ysize, 4)
	style.change_image_name_index_btn.xalign = 0.5
	
	if images.xsize is None:
		images.xsize = style.image_name_btn.xsize + images.indent * 3 + images.sprite_real_xsize
	if images.ysize is None:
		images.ysize = (images.sprite_name_btn_ysize + images.indent * 2) + images.sprite_real_ysize + (panel.info_text_size + images.indent * 2)
	
	images.update('mods/common/')
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('add_image')

screen add_image:
	key 'ESCAPE' action HideScreen('add_image')
	
	button:
		ground im.rect('#000')
		hover  im.rect('#000')
		alpha 0.3
		size 1.0
		mouse False
		action HideScreen('add_image')
	
	image images.border:
		align 0.5
		xsize images.xsize + 2 * images.border_size
		ysize images.ysize + 2 * images.border_size
		
	image images.back:
		align 0.5
		xsize images.xsize
		ysize images.ysize
		
		hbox:
			xalign 0.5
			ypos images.indent
			spacing images.indent
			
			textbutton '←':
				alpha 0 if images.all_names or images.first_name_index == 0 else 1
				style 'change_sprite_name_index_btn'
				action 'images.first_name_index -= 1'
			
			for name in images.names[images.first_name_index : images.first_name_index + images.name_count]:
				textbutton name:
					style 'sprite_name_btn'
					hovered images.set_name(name)
			
			textbutton '→':
				alpha 0 if images.all_names or images.first_name_index == images.max_first_name_index else 1
				style 'change_sprite_name_index_btn'
				action 'images.first_name_index += 1'
		
		hbox:
			ypos style.sprite_name_btn.ysize + images.indent * 2
			ysize images.ysize - (panel.info_text_size + images.indent * 2) - (style.sprite_name_btn.ysize + images.indent * 2)
			xsize images.xsize - images.indent * 2
			xalign 0.5
			spacing images.indent
			
			vbox:
				spacing images.small_indent
				yalign 0.5
				
				textbutton '↑':
					alpha 0 if images.all_images or images.first_image_index == 0 else 1
					style 'change_image_name_index_btn'
					action 'images.first_image_index -= 1'
				
				for image in images.images[images.first_image_index : images.first_image_index + images.image_count]:
					textbutton image:
						style 'image_name_btn'
						ground style.image_name_btn['hover' if image == images.cur_image else 'ground']
						hover  style.image_name_btn.hover
						hovered images.set_image(image)
						action images.add_sprite(image)
				
				textbutton '↓':
					alpha 0 if images.all_images or images.first_image_index == images.max_first_image_index else 1
					style 'change_image_name_index_btn'
					action 'images.first_image_index += 1'
			
			null:
				yalign 0.5
				
				xzoom images.sprite_xzoom
				yzoom images.sprite_yzoom
				xsize images.sprite_xsize
				ysize images.sprite_ysize
				
				image images.sprite_back:
					xsize images.sprite_xsize
					ysize images.sprite_ysize
				
				# copy-paste from Ren-Engine/rpy/screens/sprites.rpy
				for data in images.sprite.get_all_data() if images.sprite else []:
					$ tmp_image = data.res_image or data.image
					if not tmp_image or data.real_alpha <= 0:
						continue
					
					image tmp_image:
						size   (data.real_xsize, data.real_ysize)
						anchor (data.real_xanchor, data.real_yanchor)
						pos    (data.real_xpos + data.real_xanchor, data.real_ypos + data.real_yanchor) # real_pos already taked anchor, cancel it
						crop   (data.xcrop, data.ycrop, data.xsizecrop, data.ysizecrop)
						alpha   data.real_alpha
						rotate  data.real_rotate
			
		text _('Ctrl - do not remove previous sprite with this name'):
			color 0x000000
			text_size panel.info_text_size
			xalign 0.5
			ypos images.ysize - images.indent
			yanchor 1.0
