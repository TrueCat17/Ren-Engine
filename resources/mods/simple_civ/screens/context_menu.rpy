init python:
	
	class SC_MenuItem:
		def __init__(self, text, actions = None, key = '', no_delay = False):
			self.text = text
			if actions is not None:
				self.actions = list(actions) if type(actions) in (tuple, list) else [actions]
			else:
				self.actions = None
			self.key = key
			self.delay = 0 if no_delay else 0.5
	
	
	def sc_context_menu__show(items):
		if not items:
			return
		
		x, y = get_mouse()
		x = max(x, sc_context_menu.xsize)
		y = min(y + 1, get_stage_height() - sc_context_menu.text_size * len(items))
		sc_context_menu.pos = (x, y)
		
		sc_context_menu.show_time = get_game_time()
		sc_context_menu.items = items
		sc_context_menu.key_items = [item for item in items if item.key]
		
		show_screen('sc_context_menu')
	
	def sc_context_menu__hide():
		hide_screen('sc_context_menu')
		sc_info.set_msg('')
	
	
	def sc_context_menu__exec(item):
		sc_context_menu.hide()
		exec_funcs(item.actions)
	
	
	build_object('sc_context_menu')
	sc_context_menu.xsize = 250
	sc_context_menu.text_size = 20
	sc_context_menu.font = 'Calibri'
	sc_context_menu.ground = im.rect('#FFF')
	sc_context_menu.hover  = im.rect('#06F')
	
	sc_context_menu.pos = (0, 0)
	sc_context_menu.items = []
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('sc_context_menu')


screen sc_context_menu:
	modal True
	zorder 100
	
	key 'ESCAPE' action sc_context_menu.hide
	if get_game_time() - sc_context_menu.show_time > 0.25:
		for key in ('MENU', 'SPACE', 'RETURN'):
			key key action sc_context_menu.hide
	
	button:
		size 1.0
		alpha 0.01
		
		ground sc_black_bg
		hover  sc_black_bg
		mouse  False
		action    sc_context_menu.hide
		alternate sc_context_menu.hide
	
	image sc_white_bg:
		xsize sc_context_menu.xsize
		ysize len(sc_context_menu.items) * sc_context_menu.text_size
		
		xanchor 1.0
		pos sc_context_menu.pos
		
		vbox:
			for item in sc_context_menu.items:
				if item.actions is None:
					text _(item.text):
						xsize sc_context_menu.xsize
						font      sc_context_menu.font
						text_size sc_context_menu.text_size
						color    '#555'
						text_align 'center'
				else:
					textbutton (' ' + _(item.text)):
						xsize sc_context_menu.xsize
						ysize sc_context_menu.text_size
						
						font      sc_context_menu.font
						text_size sc_context_menu.text_size
						color     0
						text_align 'left'
						
						ground sc_context_menu.ground
						hover  sc_context_menu.hover
						
						hovered   sc_info.set_msg(item.key)
						unhovered sc_info.set_msg_if_prev('', item.key)
						
						action sc_context_menu.exec(item)
	
	for item in sc_context_menu.key_items:
		key item.key action sc_context_menu.exec(item)
