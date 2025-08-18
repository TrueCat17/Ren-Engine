init python:
	
	class MenuItem:
		def __init__(self, text, actions = None, key = ''):
			self.text = text
			if actions is not None:
				self.actions = list(actions) if type(actions) in (tuple, list) else [actions]
			else:
				self.actions = None
			self.key = key
	
	
	def context_menu__show(items):
		if not items:
			return
		
		x, y = get_mouse()
		x = max(x, context_menu.xsize)
		y = min(y + 1, get_stage_height() - context_menu.text_size * len(items))
		context_menu.pos = x, y
		
		context_menu.show_time = get_game_time()
		context_menu.items = items
		context_menu.key_items = [item for item in items if item.key]
		
		show_screen('context_menu')
	
	def context_menu__hide():
		hide_screen('context_menu')
		info.set_msg('')
	
	
	def context_menu__exec(item):
		context_menu.hide()
		exec_funcs(item.actions)
	
	
	build_object('context_menu')
	context_menu.xsize = 250
	context_menu.text_size = 20
	context_menu.ground = im.rect('#FFF')
	context_menu.hover  = im.rect('#06F')
	
	context_menu.pos = (0, 0)
	context_menu.items = []
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('context_menu')


screen context_menu:
	modal True
	zorder 100
	
	key 'ESCAPE' action context_menu.hide
	if get_game_time() - context_menu.show_time > 0.25:
		key 'MENU' action context_menu.hide
	
	button:
		size 1.0
		alpha 0.01
		
		ground black_bg
		hover  black_bg
		mouse  False
		action context_menu.hide
	
	image white_bg:
		xsize context_menu.xsize
		ysize len(context_menu.items) * context_menu.text_size
		
		xanchor 1.0
		pos context_menu.pos
		
		vbox:
			for item in context_menu.items:
				if item.actions is None:
					text _(item.text):
						xsize      context_menu.xsize
						text_size  context_menu.text_size
						text_align 'center'
						color      '#555'
				else:
					textbutton (' ' + _(item.text)):
						xsize context_menu.xsize
						ysize context_menu.text_size
						
						text_size  context_menu.text_size
						text_align 'left'
						color      0
						
						ground context_menu.ground
						hover  context_menu.hover
						
						hovered   info.set_msg(item.key)
						unhovered info.set_msg_if_prev('', item.key)
						
						action context_menu.exec(item)
	
	for item in context_menu.key_items:
		key item.key action context_menu.exec(item)
