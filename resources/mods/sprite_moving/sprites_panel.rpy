init 1 python:
	
	def panel__sprite_name(sprite):
		res = sprite.sprite_name
		if len(res) > 25:
			res = res[:23].rstrip() + '..'
		return res
	
	def panel__upper_sprite():
		if not panel.cur_sprite: return
		
		index = sprites.list.index(panel.cur_sprite)
		if index != 0:
			sprites.list[index - 1], sprites.list[index] = sprites.list[index], sprites.list[index - 1]
	
	def panel__lower_sprite():
		if not panel.cur_sprite: return
		
		index = sprites.list.index(panel.cur_sprite)
		if index != len(sprites.list) - 1:
			sprites.list[index + 1], sprites.list[index] = sprites.list[index], sprites.list[index + 1]
	
	def panel__remove_sprite():
		if not panel.cur_sprite: return
		
		index = sprites.list.index(panel.cur_sprite)
		sprites.list.pop(index)
		
		if sprites.list:
			if len(sprites.list) <= index:
				index = -1
			panel.cur_sprite = sprites.list[index]
		else:
			panel.cur_sprite = None
		
		panel.update_sprite_count()
	
	
	def panel__update_sprite_count():
		free_ysize = get_stage_height() - (panel.info_text_size + style.place_btn.ysize * 2 + panel.small_indent) - style.sprite_btn.ysize - 5 * panel.indent
		panel.sprite_btn_count = (free_ysize + panel.small_indent) / (style.sprite_btn.ysize + panel.small_indent)
		
		panel.all_sprite_btns = panel.sprite_btn_count >= len(sprites.list)
		if not panel.all_sprite_btns:
			panel.sprite_btn_count -= 2 # place for <prev> and <next> buttons
	
	def panel__update_sprite_size():
		panel.sprites_xsize = get_stage_width()  - panel.xsize
		panel.sprites_ysize = get_stage_height() - panel.ysize
		
		panel.update_sprite_count()
	
	
	panel.place_btn_groups = [
		('fl', 'cl', 'cr', 'fr'),
		('l', 'c', 'r'),
	]
	panel.place_names = {
		'fl': 'fleft',
		'fr': 'fright',
		'cl': 'cleft',
		'cr': 'cright',
		'l': 'left',
		'r': 'right',
		'c': 'center',
	}
	panel.default_effect_code = 'Dissolve(0.2)'
	
	def panel__place_name_is(name, sprite = None):
		sprite = sprite or panel.cur_sprite
		if not sprite: return False
		
		data = sprite.new_data
		cur_actions = (
			'pos ' + str((data.xpos, data.ypos)),
			'anchor ' + str((data.xanchor, data.yanchor))
		)
		
		g = globals()
		for short_name in panel.place_names:
			place_name = panel.place_names[short_name]
			place = g[place_name]
			if place.actions == cur_actions:
				return name == short_name
		
		return False
	
	def panel__set_place(name):
		place_name = panel.place_names[name]
		sprite = panel.cur_sprite
		index = sprites.list.index(sprite)
		
		cmd = sprite.sprite_name.split(' ')
		cmd += ['at', place_name]
		cmd += ['as', sprite.as_name]
		cmd += ['with', panel.default_effect_code]
		
		sprites.show(cmd, [])
		panel.cur_sprite = sprites.list[index]
	
	def panel__toggle_sprite_alpha(sprite):
		if sprite.new_data.alpha == 0:
			sprite.new_data.alpha = 1.0
		else:
			sprite.new_data.alpha *= -1
	
	
	build_object('panel')
	
	
	style.change_sprite_btn = Style(style.textbutton)
	style.change_sprite_btn.size = 28
	style.change_sprite_btn.ground = im.round_rect('#08F', 28, 28, 4)
	style.change_sprite_btn.hover  = im.round_rect('#F00', 28, 28, 4)
	style.change_sprite_btn.color = 0xFFFF00
	style.change_sprite_btn.outlinecolor = 0x000000
	
	style.hide_sprite_btn = Style(style.textbutton)
	style.hide_sprite_btn.size = 25
	style.hide_sprite_btn.ground = im.round_rect('#08F', 25, 25, 4)
	style.hide_sprite_btn.hover  = im.round_rect('#F00', 25, 25, 4)
	style.hide_sprite_btn.color = 0xFFFF00
	style.hide_sprite_btn.outlinecolor = 0x000000
	
	style.sprite_btn = Style(style.textbutton)
	style.sprite_btn.xsize = 200
	style.sprite_btn.ground = im.round_rect('#08F', 200, style.sprite_btn.ysize, 4)
	style.sprite_btn.hover  = im.round_rect('#F00', 200, style.sprite_btn.ysize, 4)
	style.sprite_btn.color = 0xFFFFFF
	style.sprite_btn.outlinecolor = 0x000000
	
	style.change_sprite_index_btn = Style(style.sprite_btn)
	style.change_sprite_index_btn.xsize = panel.small_btn_xsize
	style.change_sprite_index_btn.ground = im.round_rect('#08F', panel.small_btn_xsize, style.change_sprite_index_btn.ysize, 4)
	style.change_sprite_index_btn.hover  = im.round_rect('#F00', panel.small_btn_xsize, style.change_sprite_index_btn.ysize, 4)
	style.change_sprite_index_btn.xalign = 0.5
	
	style.place_btn = Style(style.textbutton)
	style.place_btn.size = (40, 25)
	style.place_btn.ground = im.round_rect('#08F', 40, 25, 4)
	style.place_btn.hover  = im.round_rect('#F00', 40, 25, 4)
	style.place_btn.color = 0xFFFFFF
	style.place_btn.outlinecolor = 0x000000
	
	
	panel.first_sprite_index = 0
	
	panel.update_sprite_size()
	signals.add('resized_stage', panel.update_sprite_size)
	


screen sprites_panel:
	alpha 0 if panel.hide else 1
	
	image panel.back:
		xalign 1.0
		xsize panel.xsize
		ysize 1.0
		
		vbox:
			ypos panel.indent
			spacing panel.indent
			xsize panel.xsize
			
			hbox:
				xalign 0.5
				spacing panel.small_indent
				
				textbutton '+':
					style 'change_sprite_btn'
					color 0x00FF00
					action ShowScreen('add_image')
				
				if panel.cur_sprite not in sprites.list:
					$ panel.cur_sprite = sprites.list[-1] if sprites.list else None
				
				$ panel.upper_enable = panel.cur_sprite is not None and sprites.list.index(panel.cur_sprite) != 0
				textbutton '↑':
					style 'change_sprite_btn'
					hover  style.change_sprite_btn['hover' if panel.upper_enable else 'ground']
					mouse  panel.upper_enable
					action panel.upper_sprite
				
				$ panel.lower_enable = panel.cur_sprite is not None and sprites.list.index(panel.cur_sprite) != len(sprites.list) - 1
				textbutton '↓':
					style 'change_sprite_btn'
					hover  style.change_sprite_btn['hover' if panel.lower_enable else 'ground']
					mouse  panel.lower_enable
					action panel.lower_sprite
				
				textbutton '-':
					style 'change_sprite_btn'
					color 0xFF0000
					hover  style.change_sprite_btn['hover' if panel.cur_sprite is not None else 'ground']
					mouse  panel.cur_sprite is not None
					action panel.remove_sprite
				
			vbox:
				xalign 0.5
				spacing panel.small_indent
				
				if not panel.all_sprite_btns:
					textbutton '↑':
						alpha 0 if panel.first_sprite_index == 0 else 1
						style 'change_sprite_index_btn'
						action 'panel.first_sprite_index -= 1'
				
				for i, sprite in enumerate(sprites.list[panel.first_sprite_index : panel.first_sprite_index + panel.sprite_btn_count]):
					hbox:
						spacing panel.small_indent
						
						$ hided = sprite.new_data.alpha <= 0 and sprite.old_data is None
						textbutton (' X ' if hided else ' O '):
							style 'hide_sprite_btn'
							ground style.hide_sprite_btn['hover' if hided else 'ground']
							hover  style.hide_sprite_btn.hover
							action panel.toggle_sprite_alpha(sprite)
						
						textbutton ('  %s. %s' % (panel.first_sprite_index + i + 1, panel.sprite_name(sprite))):
							style 'sprite_btn'
							text_align 'left'
							ground style.sprite_btn['hover' if sprite == panel.cur_sprite else 'ground']
							hover  style.sprite_btn.hover
							action 'panel.cur_sprite = sprite'
				
				if not panel.all_sprite_btns:
					textbutton '↓':
						alpha 0 if panel.first_sprite_index == len(sprites.list) - panel.sprite_btn_count else 1
						style 'change_sprite_index_btn'
						action 'panel.first_sprite_index += 1'
		
		vbox:
			xalign 0.5
			yanchor 1.0
			ypos get_stage_height() - panel.indent
			spacing panel.indent
			
			vbox:
				alpha 1 if panel.cur_sprite else 0
				xalign 0.5
				spacing panel.small_indent
				
				for btn_group in panel.place_btn_groups:
					hbox:
						xalign 0.5
						spacing panel.small_indent
						
						for btn_name in btn_group:
							textbutton btn_name:
								style 'place_btn'
								ground style.place_btn['hover' if panel.place_name_is(btn_name) else 'ground']
								hover  style.place_btn.hover
								action panel.set_place(btn_name)
			
			text ('Config width x height:\n%sx%s' % (config.width, config.height)):
				color 0x000000
				text_size panel.info_text_size
				text_align 0.5
				xalign 0.5
			

