screen slots(name):
	has hbox
	align (0.5, 0.5)
	spacing 30
	
	$ slots.check_update()
	
	vbox:
		yalign 0.5
		spacing gui.get_int('page_spacing')
		
		for page in slots.pages:
			if page == 'quick' and not config.has_quicksave:
				continue
			
			textbutton page:
				font      gui.page_button_text_font
				text_size gui.get_int('page_button_text_size')
				color        gui.get_int('page_button_text_color')
				outlinecolor gui.get_int('page_button_text_outlinecolor')
				text_align gui.page_button_text_xalign
				
				xsize gui.get_int('page_button_width')
				ysize gui.get_int('page_button_height')
				
				ground gui.bg('page_button_' + ('ground' if page != slots.get_page() else 'hover'))
				hover  gui.bg('page_button_hover')
				action slots.set_page(page)
	
	vbox:
		yalign 0.5
		spacing 20
		
		vbox:
			spacing gui.get_int('slot_spacing')
			
			$ i = 0
			for y in xrange(gui.file_slot_rows):
				hbox:
					spacing gui.get_int('slot_spacing')
					
					for x in xrange(gui.file_slot_cols):
						$ slot = slots.get(slots.slots[i])
						$ i += 1
						
						null:
							xsize slot.xsize
							ysize slot.ysize
							
							button:
								xsize  slot.xsize
								ysize  slot.ysize
								ground slot.ground
								mouse  slot.mouse
								action slot.action
							
							text slot.desc:
								xpos gui.get_int('slot_text_xpos')
								ypos gui.get_int('slot_text_ypos')
								font gui.slot_text_font
								text_size gui.get_int('slot_text_size')
								color        gui.get_int('slot_text_color')
								outlinecolor gui.get_int('slot_text_outlinecolor')
		
		hbox:
			xalign 0.5
			spacing 5
			
			python:
				btns = []
				if screen.name == 'load':
					if renpy.can_load():
						btns.append(['Load game', 1.0, True, slots.load])
					else:
						btns.append(['Load game', 0.7, False, None])
				else:
					btns.append(['Save game', 1.0, True, slots.save])
				
				if renpy.can_load(slots.selected):
					btns.append(['Delete', 1.0, True, slots.delete])
				else:
					btns.append(['Delete', 0.7, False, None])
			
			for text, alpha, mouse, action in btns:
				textbutton _(text):
					ground gui.bg('button_ground')
					hover  gui.bg('button_ground')
					xsize gui.get_int('button_width')
					ysize gui.get_int('button_height')
					font       gui.button_text_font
					text_size  gui.get_int('button_text_size')
					text_align gui.button_text_xalign
					color        gui.get_int('button_text_color')
					outlinecolor gui.get_int('button_text_outlinecolor')
					
					alpha  alpha
					mouse  mouse
					action action
