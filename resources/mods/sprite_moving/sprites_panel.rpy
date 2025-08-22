init 1 python:
	
	def sprite_panel__sprite_name(sprite):
		res = sprite.sprite_name
		if len(res) > 25:
			res = res[:23].rstrip() + '..'
		return res
	
	def sprite_panel__upper():
		if not sprite_panel.cur_sprite: return
		
		sprites.effects_to_end()
		
		index = sprites.list.index(sprite_panel.cur_sprite)
		if index != 0:
			sprites.list[index - 1], sprites.list[index] = sprites.list[index], sprites.list[index - 1]
	
	def sprite_panel__lower():
		if not sprite_panel.cur_sprite: return
		
		sprites.effects_to_end()
		
		index = sprites.list.index(sprite_panel.cur_sprite)
		if index != len(sprites.list) - 1:
			sprites.list[index + 1], sprites.list[index] = sprites.list[index], sprites.list[index + 1]
	
	def sprite_panel__remove():
		if not sprite_panel.cur_sprite: return
		
		sprites.effects_to_end()
		
		index = sprites.list.index(sprite_panel.cur_sprite)
		sprites.list.pop(index)
		
		if sprites.list:
			if len(sprites.list) <= index:
				index = -1
			sprite_panel.cur_sprite = sprites.list[index]
		else:
			sprite_panel.cur_sprite = None
		
		sprite_panel.update_sprite_count()
	
	def sprite_panel__fix_cur_sprite():
		cur_sprite = sprite_panel.cur_sprite
		if cur_sprite in sprites.list: return
		
		cur_tag = cur_sprite and cur_sprite.tag
		
		for sprite in reversed(sprites.list):
			if not cur_tag or sprite.tag == cur_tag:
				sprite_panel.cur_sprite = sprite
				break
		else:
			sprite_panel.cur_sprite = sprites.list[-1] if sprites.list else None
	
	
	def sprite_panel__update_sprite_count():
		free_ysize = get_stage_height()
		free_ysize -= style.change_sprite_btn.ysize
		free_ysize -= style.place_btn.ysize * 2 + panels.small_indent
		free_ysize -= panels.info_text_size * 2
		free_ysize -= 5 * panels.indent
		
		sprite_panel.sprite_btn_count = (free_ysize + panels.small_indent) // (style.sprite_btn.ysize + panels.small_indent)
		
		sprite_panel.all_sprite_btns = sprite_panel.sprite_btn_count >= len(sprites.list)
		if not sprite_panel.all_sprite_btns:
			sprite_panel.sprite_btn_count -= 2 # place for <prev> and <next> buttons
	
	def sprite_panel__update_sprites_size():
		panels.sprites_xsize = get_stage_width()  - panels.xsize
		panels.sprites_ysize = get_stage_height() - panels.ysize
		
		sprite_panel.update_sprite_count()
	
	
	def sprite_panel__place_name_is(place_name, sprite = None):
		sprite = sprite or sprite_panel.cur_sprite
		if not sprite: return False
		
		cur_actions = (
			'pos '    + str((sprite.xpos,    sprite.ypos)),
			'anchor ' + str((sprite.xanchor, sprite.yanchor))
		)
		
		g = globals()
		place = g[place_name]
		
		place_actions = place.actions
		if len(place_actions) != 2:
			return False
		
		for cur_action, (place_action, filename, numline) in zip(cur_actions, place_actions):
			if cur_action != place_action:
				return False
		return True
	
	def sprite_panel__set_place(place_name):
		cur_sprite = sprite_panel.cur_sprite
		
		cmd = cur_sprite.sprite_name.split(' ')
		cmd += ['at', place_name]
		cmd += ['as', cur_sprite.tag]
		cmd += ['with', sprite_panel.default_effect_code]
		
		sprites.show(cmd, ())
		
		for sprite in reversed(sprites.list):
			if sprite.tag == cur_sprite.tag:
				sprite_panel.cur_sprite = sprite
				break
	
	def sprite_panel__toggle_sprite_alpha(sprite):
		if sprite.alpha == 0:
			sprite.alpha = 1.0
		else:
			sprite.alpha *= -1
	
	def sprite_panel__get_sprites():
		tmp_sprites = []
		
		for sprite in sprites.list:
			effect = sprite.effect
			if effect:
				tmp_sprites.extend(effect.removing_sprites)
		
		new_sprites = [sprite for sprite in sprites.list if sprite not in tmp_sprites]
		
		first_index = sprite_panel.first_sprite_index
		count = sprite_panel.sprite_btn_count
		return new_sprites[first_index : first_index + count]
	
	
	build_object('sprite_panel')
	
	sprite_panel.place_btn_groups = (
		('fl', 'cl', 'cr', 'fr'),
		('l', 'c', 'r'),
	)
	sprite_panel.place_names = {
		'fl': 'fleft',
		'fr': 'fright',
		'cl': 'cleft',
		'cr': 'cright',
		'l': 'left',
		'r': 'right',
		'c': 'center',
	}
	sprite_panel.default_effect_code = 'Dissolve(0.2)'
	
	sprite_panel.first_sprite_index = 0
	
	sprite_panel.update_sprites_size()
	signals.add('resized_stage', sprite_panel.update_sprites_size)

init:
	style change_sprite_btn is textbutton:
		size 28
		ground im.round_rect('#08F', 20, 20, 4)
		hover  im.round_rect('#F00', 20, 20, 4)
		color '#FF0'
		outlinecolor 0
	
	style hide_sprite_btn is change_sprite_btn:
		size 25
	
	style sprite_btn is change_sprite_btn:
		size (200, 25)
		color '#FFF'
	
	style change_sprite_index_btn is sprite_btn:
		xsize panels.small_btn_xsize
		xalign 0.5
	
	style place_btn is sprite_btn:
		size (40, 25)


screen sprites_panel:
	alpha 0 if panels.hide else 1
	
	python:
		sprite_panel.fix_cur_sprite()
		screen_tmp = SimpleObject()
		screen_tmp.have_cur_sprite = sprite_panel.cur_sprite is not None
		screen_tmp.cur_index = sprites.list.index(sprite_panel.cur_sprite) if screen_tmp.have_cur_sprite else None
	
	image panels.back:
		xalign 1.0
		xsize panels.xsize
		ysize 1.0
		
		vbox:
			ypos    panels.indent
			spacing panels.indent
			xsize panels.xsize
			
			hbox:
				xalign 0.5
				spacing panels.small_indent
				
				textbutton '+':
					style 'change_sprite_btn'
					color '#0F0'
					action show_screen('add_image')
				
				$ screen_tmp.upper_enable = screen_tmp.cur_index not in (None, 0)
				textbutton '↑':
					style 'change_sprite_btn'
					hover  style.change_sprite_btn['hover' if screen_tmp.upper_enable else 'ground']
					mouse  screen_tmp.upper_enable
					action sprite_panel.upper
				
				$ screen_tmp.lower_enable = screen_tmp.cur_index not in (None, len(sprites.list) - 1)
				textbutton '↓':
					style 'change_sprite_btn'
					hover  style.change_sprite_btn['hover' if screen_tmp.lower_enable else 'ground']
					mouse  screen_tmp.lower_enable
					action sprite_panel.lower
				
				textbutton '-':
					style 'change_sprite_btn'
					color '#F00'
					hover  style.change_sprite_btn['hover' if screen_tmp.have_cur_sprite else 'ground']
					mouse  screen_tmp.have_cur_sprite
					action sprite_panel.remove
				
				textbutton '?':
					style 'change_sprite_btn'
					color '#0F0'
					action show_screen('help')
				key 'F1' action show_screen('help')
			
			vbox:
				xalign 0.5
				spacing panels.small_indent
				
				if not sprite_panel.all_sprite_btns:
					textbutton '↑':
						style 'change_sprite_index_btn'
						alpha 0 if sprite_panel.first_sprite_index == 0 else 1
						action 'sprite_panel.first_sprite_index -= 1'
				
				for i, sprite in enumerate(sprite_panel.get_sprites()):
					hbox:
						spacing panels.small_indent
						
						$ screen_tmp.hided = sprite.alpha <= 0
						textbutton ('X' if screen_tmp.hided else 'O'):
							style 'hide_sprite_btn'
							selected screen_tmp.hided
							action sprite_panel.toggle_sprite_alpha(sprite)
						
						textbutton ('  %s. %s' % (sprite_panel.first_sprite_index + i + 1, sprite_panel.sprite_name(sprite))):
							style 'sprite_btn'
							text_align 'left'
							selected sprite_panel.cur_sprite == sprite
							action 'sprite_panel.cur_sprite = sprite'
				
				if not sprite_panel.all_sprite_btns:
					textbutton '↓':
						style 'change_sprite_index_btn'
						alpha 0 if sprite_panel.first_sprite_index == len(sprites.list) - sprite_panel.sprite_btn_count else 1
						action 'sprite_panel.first_sprite_index += 1'
		
		vbox:
			xalign 0.5
			yanchor 1.0
			ypos get_stage_height() - panels.indent
			spacing panels.indent
			
			vbox:
				alpha 1 if screen_tmp.have_cur_sprite else 0
				xalign 0.5
				spacing panels.small_indent
				
				for btn_group in sprite_panel.place_btn_groups:
					hbox:
						xalign 0.5
						spacing panels.small_indent
						
						for btn_name in btn_group:
							$ screen_tmp.place_name = sprite_panel.place_names[btn_name]
							
							textbutton btn_name:
								style 'place_btn'
								selected sprite_panel.place_name_is(screen_tmp.place_name)
								action sprite_panel.set_place(screen_tmp.place_name)
			
			text ('Config width x height:\n%sx%s' % (config.width, config.height)):
				color 0
				text_size panels.info_text_size
				text_align 0.5
				xalign 0.5
