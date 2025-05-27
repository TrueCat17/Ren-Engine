screen slots(name):
	$ slots.check_update()
	
	image (gui.bg(screen.name + '_bg') or gui.bg('main_bg')):
		size 1.0
	
	text _(screen.name.title()):
		style style[screen.name + '_menu_title'] or style.menu_title
	
	vbox:
		style style[screen.name + '_pages_vbox'] or style.pages_vbox
		
		$ tmp_style = style[screen.name + '_page_button'] or style.page_button
		for page in slots.pages:
			if page == 'quick' and not config.has_quicksave:
				continue
			
			textbutton page:
				style tmp_style
				selected persistent.slot_page == page
				action slots.set_page(page)
	
	vbox:
		style 'slots_content'
		
		vbox:
			style 'slots_vbox'
			
			$ i = 0
			for y in range(gui.file_slot_rows):
				hbox:
					style 'slots_hbox'
					
					for x in range(gui.file_slot_cols):
						$ slot = slots.get(slots.slots[i])
						
						image (style.get_default_hover(None, slot.ground) if slots.hovered == i else slot.ground):
							xsize slot.xsize
							ysize slot.ysize
							
							text slot.desc:
								style 'slot_text'
							
							button:
								style 'slot_button'
								xsize slot.xsize
								ysize slot.ysize
								mouse  slot.mouse or screen.name == 'save'
								action slot.action
								
								hovered 'slots.hovered = i'
								unhovered 'if slots.hovered == i: slots.hovered = None'
						
						$ i += 1
		
		hbox:
			style 'slots_buttons'
			
			for text, tmp_style, alpha, mouse, action in slots.get_btns(screen.name):
				textbutton _(text):
					style tmp_style
					alpha  tmp_style.alpha * alpha
					mouse  mouse
					action action
	
	$ tmp_style = style[screen.name + '_return_button'] or style.return_button
	textbutton _('Return'):
		style tmp_style
		action hide_screen(screen.name)
	
	key 'ESCAPE' action hide_screen(screen.name)
