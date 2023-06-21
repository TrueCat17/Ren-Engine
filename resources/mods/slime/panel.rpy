init -1 python:
	show_agents = False
	
	panel_bg = im.rect('#333')
	panel_spacing = 7
	
	panel_ground_color = '#606060'
	panel_hover_color  = '#888'
	
	def update_panel_props():
		global panel_size, panel_btn_width, panel_btn_height, panel_text_width, panel_text_size
		panel_size = in_bounds(get_stage_width() // 4, 200, 400)
		panel_btn_width = in_bounds(get_stage_width() // 8, 150, 250)
		panel_btn_height = in_bounds(get_stage_width() // 40, 22, 40)
		panel_text_width = panel_size - panel_btn_height * 4
		panel_text_size = in_bounds(get_stage_height() // 35, 14, 28)
	update_panel_props()
	signals.add('resized_stage', update_panel_props)
	
	style.textbutton.color = 0xFFFFFF
	style.textbutton.outlinecolor = 0x000000


screen panel_row(params):
	$ minus, text, add = screen.params
	has hbox
	spacing panel_spacing
	
	textbutton '-':
		ground im.round_rect(panel_ground_color, panel_btn_height, panel_btn_height, 4)
		hover  im.round_rect(panel_hover_color,  panel_btn_height, panel_btn_height, 4)
		size panel_btn_height
		text_size style.textbutton.text_size * 1.5
		color 0xFF0000
		yalign 0.5
		action minus
	
	text text:
		color 0xFFFFFF
		yalign 0.5
		text_align 'center'
		text_size panel_text_size
		xsize panel_text_width
	
	textbutton '+':
		ground im.round_rect(panel_ground_color, panel_btn_height, panel_btn_height, 4)
		hover  im.round_rect(panel_hover_color,  panel_btn_height, panel_btn_height, 4)
		size panel_btn_height
		text_size style.textbutton.text_size * 1.5
		color 0x00FF00
		yalign 0.5
		action add

screen panel:
	xsize panel_size
	xalign 1.0
	
	image panel_bg:
		size (panel_size, 1.0)
	
	vbox:
		align (0.5, 0.05)
		spacing panel_spacing
		
		$ params = (
			Function(del_agents, 10),
			_('Agents') + ': ' + str(len(agents)),
			Function(add_agents, 10),
		)
		use panel_row(params)
		
		$ params = (
			Function(del_steps, 5),
			_('Steps') + ': ' + str(step_life_time),
			Function(add_steps, 5),
		)
		use panel_row(params)
		
		$ params = (
			'rotation_angle = max(rotation_angle - 5, 5)',
			_('Rotation angle') + ': ' + str(rotation_angle),
			'rotation_angle = min(rotation_angle + 5, 175)',
		)
		use panel_row(params)
		
		$ params = (
			'sensors_angle = max(sensors_angle - 5, 5)',
			_('Viewing angle') + ': ' + str(sensors_angle),
			'sensors_angle = min(sensors_angle + 5, 175)',
		)
		use panel_row(params)
		
		$ params = (
			'extra_rotation = max(extra_rotation - 1, -15)',
			_('Extra rotation') + ': ' + str(extra_rotation),
			'extra_rotation = min(extra_rotation + 1, 15)',
		)
		use panel_row(params)
		
		$ params = (
			'wobble = max(wobble - 1, 0)',
			_('Wobble') + ': ' + str(wobble),
			'wobble = min(wobble + 1, 15)',
		)
		use panel_row(params)
	
	vbox:
		align (0.5, 0.95)
		spacing panel_spacing
		
		$ btns = (
			[_('Restart') + ' (R)', start],
			[_('Hide agents' if show_agents else 'Show agents') + ' (X)', ToggleVariable('show_agents')],
			[_('Color') + ' (C)', ToggleVariable('use_colors')],
			[_('Random') + ' (A)', random_params],
			[_('Default') + ' (D)', default_params],
		)
		for text, action in btns:
			textbutton text:
				ground im.round_rect(panel_ground_color, panel_btn_width, panel_btn_height, 4)
				hover  im.round_rect(panel_hover_color,  panel_btn_width, panel_btn_height, 4)
				size  (panel_btn_width, panel_btn_height)
				action action
		
		key 'R' action start
		key 'X' action ToggleVariable('show_agents')
		key 'C' action ToggleVariable('use_colors')
		key 'A' action random_params
		key 'D' action default_params
		
