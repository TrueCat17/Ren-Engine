init python:
	
	def images__update(dir_path):
		for path, ds, fs in os.walk(dir_path):
			path = make_sure_dir(path)
			
			for f in fs:
				if not f.endswith('.rpy'): continue
				
				with open(path + f, 'rb') as f:
					content = f.read().decode('utf-8')
				
				for l in content.split('\n'):
					l = l.strip()
					if not l.startswith('image '): continue
					
					l = l[6:] # 6 = len('image ')
					if l.endswith(':'):
						l = l[:-1]
					if '=' in l:
						l = l[:l.index('=')]
					l = l.strip()
					if l.startswith('_'): continue
					
					if ' ' in l:
						name = l[:l.index(' ')]
					else:
						name = l
					
					if image_was_registered(l):
						names = images.sprites.setdefault(name, [])
						names.append(l)
		
		images.names = sorted(images.sprites.keys())
		images.set_name(images.names and images.names[0])
		
		images.max_name_len = 0
		for name in images.names:
			images.max_name_len = max(images.max_name_len, len(name))
		one_symbol_xsize = style.image_name_btn.text_size / 1.5
		images.name_xsize = int(one_symbol_xsize * images.max_name_len * 1.2)
		
		images.name_count = images.xsize // (images.name_xsize + panels.indent)
		images.all_names = images.name_count >= len(images.names)
		if not images.all_names:
			images.name_count -= 2 # place for <prev> and <next> buttons
			xsize = images.xsize - style.change_image_name_index_btn.xsize * 2 - panels.indent * 4
			images.name_count = (xsize + panels.indent) // (images.name_xsize + panels.indent)
		
		images.first_name_index = 0
		images.max_first_name_index = len(images.names) - images.name_count
	
	def images__set_name(name):
		changed = images.cur_name != name
		if changed:
			images.set_image(None)
		images.cur_name = name or ''
		images.images = images.sprites[name] if name else []
		
		free_ysize = images.ysize - (style.image_name_btn.ysize + panels.indent * 2) - (panels.info_text_size * 2 + panels.indent * 2)
		images.image_count = (free_ysize + panels.small_indent) // (style.image_name_btn.ysize + panels.small_indent)
		images.all_images = images.image_count >= len(images.images)
		if not images.all_images:
			images.image_count -= 2 # place for <prev> and <next> buttons
		
		if changed:
			images.first_image_index = 0
		images.max_first_image_index = len(images.images) - images.image_count
	
	def images__set_image(image):
		images.cur_image = image
		if image:
			tag = sprites.get_tag_of_image_name(image)
			images.sprite = Sprite(tag, image, get_image(image), center.actions, (), None)
	
	
	def images__add_sprite(image):
		cmd = image.split(' ')
		tag = cmd[0]
		
		if hotkeys.ctrl:
			i = 0
			while True:
				i += 1
				tmp_tag = tag + '-' + str(i)
				
				for spr in sprites.list:
					if spr.tag == tmp_tag:
						break
				else:
					tag = tmp_tag
					break
			cmd += ['as', tag]
		
		if hotkeys.shift:
			cmd += ['at', 'None']
		
		cmd += ['with', sprite_panel.default_effect_code]
		
		sprites.show(cmd, ())
		
		for sprite in reversed(sprites.list):
			if sprite.tag == tag:
				sprite_panel.cur_sprite = sprite
				break
		
		sprite_panel.update_sprite_count()
	
	def images__reset_sprite():
		sprite = sprite_panel.cur_sprite
		if not sprite: return
		
		cmd = sprite.sprite_name.split(' ')
		
		usual_tag = cmd[0]
		if usual_tag != sprite.tag:
			cmd += ['as', sprite.tag]
		
		if ' at None' in sprite.call_str:
			cmd += ['at', 'None']
		else:
			cmd += ['at', 'center']
		cmd += ['with', sprite_panel.default_effect_code]
		
		sprite.ignore_old_data = True
		sprites.show(cmd, ())
		
		old_sprite = sprite
		for sprite in reversed(sprites.list):
			if sprite.tag == old_sprite.tag and sprite is not old_sprite:
				sprite_panel.cur_sprite = sprite
				break
	
	
	def images__fix_sprites_real_size():
		k = get_from_hard_config('window_w_div_h', float)
		if k > 1.777:
			images.sprites_real_xsize = round(images.sprites_real_ysize * k)
		else:
			images.sprites_real_ysize = round(images.sprites_real_xsize / k)
	
	def images__update_sprites_size():
		images.sprites_xsize = config.width  or get_stage_width()
		images.sprites_ysize = config.height or get_stage_height()
		
		images.sprites_xzoom = images.sprites_real_xsize / images.sprites_xsize
		images.sprites_yzoom = images.sprites_real_ysize / images.sprites_ysize
	
	
	build_object('images')
	
	images.sprites = {}
	images.fog = im.rect('#0005')
	
	images.back = im.rect('#FFF')
	images.xsize = None
	images.ysize = None
	images.border = im.rect('#00F')
	images.border_size = 3
	images.sprites_back = im.rect('#888')
	
	images.sprites_real_xsize = 1920 // 4
	images.sprites_real_ysize = 1080 // 4
	images.fix_sprites_real_size()
	images.update_sprites_size()
	signals.add('resized_stage', images.update_sprites_size)
	
	images.first_image_index = 0


init:
	style image_name_btn is textbutton:
		xsize 250
		ground im.round_rect('#08F', 20, 20, 4)
		hover  im.round_rect('#F00', 20, 20, 4)
		text_size 18
		color '#0F0'
		outlinecolor 0
	
	style change_image_name_index_btn is image_name_btn:
		xsize panels.small_btn_xsize
		xalign 0.5
	
	style sprite_name_btn is image_name_btn:
		ground im.round_rect('#F80', 20, 20, 4)
		hover  im.round_rect('#F00', 20, 20, 4)
		text_size 20
	
	style change_sprite_name_index_btn is sprite_name_btn:
		xsize panels.small_btn_xsize


init 1 python:
	if images.xsize is None:
		images.xsize = style.image_name_btn.xsize + panels.indent * 3 + images.sprites_real_xsize
	if images.ysize is None:
		images.ysize = (style.image_name_btn.ysize + panels.indent * 2) + images.sprites_real_ysize + (panels.info_text_size * 2 + panels.indent * 2)
	
	images.update('mods/common/')
	hotkeys.disable_key_on_screens['ESCAPE'].append('add_image')


screen add_image:
	key 'ESCAPE' action hide_screen('add_image')
	
	button:
		ground images.fog
		hover  images.fog
		size 1.0
		mouse False
		action hide_screen('add_image')
	
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
			ypos    panels.indent
			spacing panels.indent
			
			textbutton '←':
				style 'change_sprite_name_index_btn'
				alpha 0 if images.all_names or images.first_name_index == 0 else 1
				action 'images.first_name_index -= 1'
			
			for name in images.names[images.first_name_index : images.first_name_index + images.name_count]:
				textbutton name:
					style 'sprite_name_btn'
					xsize images.name_xsize
					selected images.cur_name == name
					hovered images.set_name(name)
			
			textbutton '→':
				style 'change_sprite_name_index_btn'
				alpha 0 if images.all_names or images.first_name_index == images.max_first_name_index else 1
				action 'images.first_name_index += 1'
		
		hbox:
			ypos style.sprite_name_btn.ysize + panels.indent * 2
			ysize images.ysize - (panels.info_text_size * 2 + panels.indent * 2) - (style.sprite_name_btn.ysize + panels.indent * 2)
			xsize images.xsize - panels.indent * 2
			xalign 0.5
			spacing panels.indent
			
			vbox:
				spacing panels.small_indent
				yalign 0.5
				
				textbutton '↑':
					style 'change_image_name_index_btn'
					alpha 0 if images.all_images or images.first_image_index == 0 else 1
					action 'images.first_image_index -= 1'
				
				for image in images.images[images.first_image_index : images.first_image_index + images.image_count]:
					textbutton image:
						style 'image_name_btn'
						selected images.cur_image == image
						hovered images.set_image(image)
						action images.add_sprite(image)
				
				textbutton '↓':
					style 'change_image_name_index_btn'
					alpha 0 if images.all_images or images.first_image_index == images.max_first_image_index else 1
					action 'images.first_image_index += 1'
			
			null:
				yalign 0.5
				
				xzoom images.sprites_xzoom
				yzoom images.sprites_yzoom
				xsize images.sprites_xsize
				ysize images.sprites_ysize
				
				image images.sprites_back:
					xsize images.sprites_xsize
					ysize images.sprites_ysize
				
				if images.sprite:
					$ images.sprite.update()
					
					# copy-paste from Ren-Engine/rpy/screens/sprites.rpy
					for spr in images.sprite.get_all_sprites() if images.sprite else ():
						$ tmp_image = spr.res_image or spr.image
						if not tmp_image:
							continue
						
						image tmp_image:
							size   (spr.real_xsize, spr.real_ysize)
							anchor (spr.real_xanchor, spr.real_yanchor)
							pos    (spr.real_xpos + spr.real_xanchor, spr.real_ypos + spr.real_yanchor) # real_pos already taked anchor, cancel it
							crop   (spr.xcrop, spr.ycrop, spr.xsizecrop, spr.ysizecrop)
							alpha   spr.real_alpha
							rotate  spr.real_rotate
		
		text (_('Ctrl - do not remove previous sprite with this name') + '\n' + _('Shift - not to be forcibly in the center')):
			color 0
			text_size panels.info_text_size
			xalign 0.5
			ypos images.ysize - panels.indent
			yanchor 1.0
