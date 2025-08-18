init python:
	def get_dcoords(src, dst):
		if not dst:
			return _('No')
		dx, dy = dst.x - src.x, dst.y - src.y
		sx = '+' if dx > 0 else ''
		sy = '+' if dy > 0 else ''
		return '[%s%i, %s%i]' % (sx, dx, sy, dy)


screen control:
	python:
		control.indent = min(get_stage_height() // 40, 15)
		control.text_size = min(get_stage_height() // 20, 20)
	
	image control.back:
		size (control.xsize, get_stage_height() - info.ysize)
		xalign 1.0
	
	vbox:
		spacing control.indent
		size (control.xsize - 2 * control.indent, get_stage_height() - info.ysize)
		xpos get_stage_width() - control.xsize + control.indent
		ypos control.indent
		
		null:
			xsize control.xsize - 2 * control.indent
			
			text (_('Step') + ': ' + str(control.step)):
				yalign 0.5
				color 0
				text_size control.text_size
			
			textbutton _('Next'):
				style 'btn'
				align (1.0, 0.5)
				xsize 100
				text_size control.text_size
				
				hovered   info.set_msg('Shift+Enter')
				unhovered info.set_msg('')
				
				action sc_map.next_step
			if hotkeys.shift:
				key 'RETURN' action sc_map.next_step
		
		
		$ cell = control.selected_cell
		if cell:
			text ('%s <%s> (%s)' % (_('Field'), _(cell.resource), cell.resource_count)):
				color 0
				text_size control.text_size
			
			if cell.building:
				python:
					text = ('%s: %s #%s' % (_('Building'), _(cell.building), cell.building_level))
					if cell.building in ('district', 'storage'):
						if cell.building == 'storage' and cell.enabled_level == 0:
							text += '\n{color=#F80}{outlinecolor=0}%s{/outlinecolor}{/color}' % _('No road to\nmain storage')
					else:
						text += '\n%s: %s/%s' % (_('Workers'), len(cell.workers), cell.enabled_level)
						if cell.enabled_level != cell.building_level:
							text += '\n{color=#FF0}{outlinecolor=0}%s{/outlinecolor}{/color}' % _('Storage < Building')
				
				text text:
					color 0
					text_size control.text_size
		
		if control.selected_unit_array:
			vbox:
				xalign 0.5
				spacing 5
				
				text ('%s (%s):' % (_('Units'), len(control.selected_unit_array))):
					color 0
					text_size control.text_size
				
				$ control.unit_size = min(get_stage_height() // 20, 32)
				image control.unit_back:
					xsize control.xsize - 2 * min(control.indent, 10)
					ysize control.unit_size + control.indent
					xalign 0.5
					
					hbox:
						align 0.5
						spacing 10
						
						for i in range(control.selected_unit_array_start, control.selected_unit_array_end):
							$ unit = control.selected_unit_array[i]
							image control['selected_unit_bg' if unit is control.selected_unit else 'unselected_unit_bg']:
								size control.unit_size
								
								text unit.get_symbol():
									text_size control.text_size
									bold      True
									color     '#FFF'
									outlinecolor 0
									xpos    unit.symbol_offset
									xanchor 0.5
								
								button:
									ground get_back_with_color(unit.image)
									size   control.unit_size
									action [control.select_unit(unit), context_menu.show(control.selected_unit_menu_items)]
				
				hbox:
					xalign 0.5
					spacing control.indent
					
					$ allow_arrows()
					if control.selected_unit_index > 0:
						key 'LEFT' action control.prev_unit
					if control.selected_unit_index < len(control.selected_unit_array) - 1:
						key 'RIGHT' action control.next_unit
					
					textbutton '<':
						style     'btn'
						bold      True
						alpha     1 if control.selected_unit_index > 0 else 0
						size      control.text_size
						text_size control.text_size - 2
						action    control.prev_unit
					text ('[%s/%s]' % (control.selected_unit_index + 1, len(control.selected_unit_array))):
						color 0
						text_size control.text_size
					textbutton '>':
						style     'btn'
						bold      True
						alpha     1 if control.selected_unit_index < len(control.selected_unit_array) - 1 else 0
						size      control.text_size
						text_size control.text_size - 2
						action    control.next_unit
				
				if isinstance(control.selected_unit, Worker):
					text ('%s: %s' % (_('Work place'), get_dcoords(control.selected_unit, control.selected_unit.work_cell))):
						color     0
						text_size control.text_size
				
				if control.picking:
					key 'ESCAPE' action control.unpick
				else:
					for item in control.selected_unit_menu_items:
						if item.key:
							key item.key action item.actions
					key 'MENU' action context_menu.show(control.selected_unit_menu_items)
		
		if cell.building not in (None, 'district', 'storage'):
			$ dcoords = [get_dcoords(cell, unit) for unit in cell.workers] or [_('No')]
			text ('%s:\n{size=-5}%s{/size}' % (_('Worker districts'), ', '.join(dcoords))):
				color     0
				text_size control.text_size
