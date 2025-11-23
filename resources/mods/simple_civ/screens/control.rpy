init -10 python:
	def sc_control__get_xsize():
		return max(get_stage_width() // 7, 250)

screen sc_control:
	python:
		screen_tmp = SimpleObject()
		screen_tmp.xsize = sc_control.get_xsize()
		screen_tmp.indent = min(get_stage_height() // 72, 10)
		screen_tmp.text_indent = screen_tmp.indent - 4
		screen_tmp.text_size = min(get_stage_height() // 20, 20)
		screen_tmp.btn_size = screen_tmp.xsize - screen_tmp.indent * 2
	
	image sc_control.back:
		xsize screen_tmp.xsize
		ysize get_stage_height() - sc_info.get_ysize()
		xalign 1.0
		
		vbox:
			spacing screen_tmp.indent
			xalign 0.5
			ypos screen_tmp.indent
			
			null:
				xsize screen_tmp.btn_size
				
				text ('%s: %s' % (_('Step'), sc_map.step)):
					yalign 0.5
					color 0
					text_size screen_tmp.text_size
				
				textbutton _('Next'):
					style 'sc_btn' if sc_map.executed_bots else 'sc_disabled_btn'
					align (1.0, 0.5)
					xsize 85
					text_size screen_tmp.text_size
					
					hovered   sc_info.set_msg('Shift+Enter')
					unhovered sc_info.set_msg('')
					
					action sc_map.next_step if sc_map.executed_bots else None
				if hotkeys.shift:
					key 'RETURN' action (sc_map.next_step if sc_map.executed_bots else None)
			
			
			vbox:
				spacing screen_tmp.text_indent
				
				text ('%s: %s' % (_('Seed'), sc_map.seed)):
					color 0
					text_size screen_tmp.text_size
				
				$ cell = sc_control.selected_cell
				if cell:
					text ('x = %s, y = %s' % (cell.x, cell.y)):
						color 0
						text_size screen_tmp.text_size
					
					text ('%s <%s> (%s)' % (_('Field'), _(cell.resource), cell.resource_count)):
						color 0
						text_size screen_tmp.text_size
					
					text ('%s: %s' % (_('Force'), cell.player_forces)):
						color 0
						text_size screen_tmp.text_size
					
					python:
						text = ''
						if cell.building:
							text = ('%s: %s #%s\n' % (_('Building'), _(cell.building), cell.building_level))
							if cell.disabled and (cell.player is sc_map.player or sc_map.player.bot):
								text += '{color=#FF0}{outlinecolor=0}%s{/outlinecolor}{/color}, ' % _('Disabled')
						
						if cell.player is not cell.next_player and cell.player is sc_map.player:
							text += '{color=#FF0}{outlinecolor=0}%s{/outlinecolor}{/color}' % _('Will be lost')
						else:
							text = text.rstrip(', ').rstrip('\n')
					
					text text:
						color 0
						text_size screen_tmp.text_size
		
		vbox:
			align (0.5, 1.0)
			spacing screen_tmp.indent
			
			textbutton _('Technologies'):
				style    'sc_btn'
				xsize     screen_tmp.btn_size
				text_size screen_tmp.text_size
				hovered   sc_info.set_msg('F2')
				unhovered sc_info.set_msg_if_prev('', 'F2')
				action show_screen('sc_technologies')
			key 'F2' action show_screen('sc_technologies')
			
			textbutton _('Help'):
				style    'sc_btn'
				xsize     screen_tmp.btn_size
				text_size screen_tmp.text_size
				hovered   sc_info.set_msg('F1')
				unhovered sc_info.set_msg_if_prev('', 'F1')
				action    show_screen('help')
			key 'F1' action show_screen('help')
			
			textbutton _('Pause'):
				style    'sc_btn'
				xsize     screen_tmp.btn_size
				text_size screen_tmp.text_size
				hovered   sc_info.set_msg('Esc')
				unhovered sc_info.set_msg_if_prev('', 'Esc')
				action pause_screen.show
			
			textbutton _('New Game'):
				style    'sc_btn'
				xsize     screen_tmp.btn_size
				text_size screen_tmp.text_size
				action input.confirm(sc_show_menu, 'End the current game?')
			
			null ysize 1
	
	
	for item in sc_control.menu_items:
		if item.key:
			key item.key:
				first_delay item.delay
				delay       item.delay
				action item.actions
	key 'MENU' action sc_control.show_menu
	
	for i in (1, 2, 3, 4):
		key str(i) action sc_control.set_player(i - 1)
