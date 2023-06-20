init -1 python:
	show_agents = False
	
	panel_size = 150
	panel_btn_size = 25
	
	def update_panel_props():
		global panel_text_size
		panel_text_size = in_bounds(get_stage_height() / 35, 14, 28)
		global panel_big_spacing, panel_spacing
		panel_big_spacing = min(get_stage_height() // 50, 15)
		panel_spacing = max(panel_big_spacing // 2, 7)
	update_panel_props()
	signals.add('resized_stage', update_panel_props)
	
	def panel_text_width(text):
		return int(round(get_text_width(text, panel_text_size) / 1.2))

init:
	style textbutton:
		ground im.round_rect('#08F', 150, 25, 4)
		hover  im.round_rect('#F80', 150, 25, 4)
		size (150, 25)
		color 0xFFFFFF
		outlinecolor 0x000000
	
	style small_btn is textbutton:
		ground im.round_rect('#08F', 25, 25, 4)
		hover  im.round_rect('#F80', 25, 25, 4)
		size 25


screen colon(params):
	$ minus1, text1, add1, minus2, text2, add2 = screen.params
	has vbox
	spacing panel_spacing
	
	$ text_width = max(panel_text_width(text1), panel_text_width(text2))
	
	hbox:
		xalign 0.5
		
		textbutton '-':
			style 'small_btn'
			yalign 0.5
			action minus1
		text text1:
			yalign 0.5
			color 0xFFFFFF
			text_align 'center'
			text_size panel_text_size
			xsize text_width
		textbutton '+':
			style 'small_btn'
			yalign 0.5
			action add1
	
	hbox:
		xalign 0.5
		
		textbutton '-':
			style 'small_btn'
			yalign 0.5
			action minus2
		text text2:
			yalign 0.5
			color 0xFFFFFF
			text_align 'center'
			text_size panel_text_size
			xsize text_width
		textbutton '+':
			style 'small_btn'
			yalign 0.5
			action add2

screen panel:
	ysize panel_size
	yalign 1.0
	
	image im.rect('#444'):
		size (1.0, panel_size)
	
	vbox:
		align 0.5
		spacing panel_big_spacing
		
		hbox:
			xalign 0.5
			spacing panel_big_spacing
			
			$ params = (
				Function(del_agents, 10),
				_('Agents') + ': ' + str(len(agents)),
				Function(add_agents, 10),
				
				Function(del_steps, 5),
				_('Steps') + ': ' + str(step_life_time),
				Function(add_steps, 5),
			)
			use colon(params)
			
			$ params = (
				'rotation_angle = max(rotation_angle - 5, 5)',
				_('Rotation angle') + ': ' + str(rotation_angle),
				'rotation_angle = min(rotation_angle + 5, 175)',
				
				'sensors_angle = max(sensors_angle - 5, 5)',
				_('Viewing angle') + ': ' + str(sensors_angle),
				'sensors_angle = min(sensors_angle + 5, 175)',
			)
			use colon(params)
			
			$ params = (
				'extra_rotation = max(extra_rotation - 1, -15)',
				_('Extra rotation') + ': ' + str(extra_rotation),
				'extra_rotation = min(extra_rotation + 1, 15)',
				
				'wobble = max(wobble - 1, 0)',
				_('Wobble') + ': ' + str(wobble),
				'wobble = min(wobble + 1, 15)',
			)
			use colon(params)
		
		hbox:
			xalign 0.5
			spacing panel_spacing
			
			vbox:
				spacing panel_spacing
				
				textbutton (_('Restart') + ' (R)') action start
				textbutton (_('Hide agents' if show_agents else 'Show agents') + ' (X)') action ToggleVariable('show_agents')
			
			textbutton (_('Color') + ' (C)'):
				yalign 0.5
				action ToggleVariable('use_colors')
			
			vbox:
				spacing panel_spacing
				
				textbutton (_('Random') + ' (A)') action random_params
				textbutton (_('Default') + ' (D)') action default_params
			
			key 'R' action start
			key 'X' action ToggleVariable('show_agents')
			key 'C' action ToggleVariable('use_colors')
			key 'A' action random_params
			key 'D' action default_params
		
