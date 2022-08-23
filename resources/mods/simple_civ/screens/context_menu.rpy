init python:
	
	class MenuItem:
		def __init__(self, text, actions = None, key = ''):
			self.text = text
			if actions is not None:
				self.actions = list(actions) if isinstance(actions, (tuple, list)) else [actions]
			else:
				self.actions = None
			self.key = key
	
	
	def context_menu__show(items):
		if not items:
			return
		
		x, y = get_mouse()
		x = max(x, context_menu.xsize)
		y = min(y, get_stage_height() - context_menu.text_size * len(items))
		context_menu.pos = x, y
		
		context_menu.show_time = get_game_time()
		context_menu.items = items
		context_menu.key_items = [item for item in items if item.key]
		context_menu.selected_index = None
		
		show_screen('context_menu')
	
	
	def context_menu__up():
		if not context_menu.selected_index:
			context_menu.selected_index = len(context_menu.items) - 1
		else:
			context_menu.selected_index -= 1
		
		start_index = context_menu.selected_index
		while True:
			if context_menu.items[context_menu.selected_index].actions:
				break
			context_menu.selected_index -= 1
			if context_menu.selected_index == -1:
				context_menu.selected_index = len(context_menu.items) - 1
			if context_menu.selected_index == start_index:
				context_menu.selected_index = None
				break
	def context_menu__down():
		if context_menu.selected_index in (None, len(context_menu.items) - 1):
			context_menu.selected_index = 0
		else:
			context_menu.selected_index += 1
		
		start_index = context_menu.selected_index
		while True:
			if context_menu.items[context_menu.selected_index].actions:
				break
			context_menu.selected_index += 1
			if context_menu.selected_index == len(context_menu.items):
				context_menu.selected_index = 0
			if context_menu.selected_index == start_index:
				context_menu.selected_index = None
				break
	
	def context_menu__select():
		hide_screen('context_menu')
		info.set_msg('')
		exec_funcs(context_menu.items[context_menu.selected_index].actions)
	
	
	build_object('context_menu')
	context_menu.xsize = 250
	context_menu.text_size = 20
	context_menu.ground = im.rect('#FFF')
	context_menu.hover = im.rect('#06F')
	
	context_menu.pos = 0, 0
	context_menu.items = []
	context_menu.selected_index = None
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('context_menu')


screen context_menu:
	modal True
	zorder 100
	
	key 'ESCAPE' action [hide_screen('context_menu'), info.set_msg('')]
	if get_game_time() - context_menu.show_time > 0.25:
		key 'MENU' action [hide_screen('context_menu'), info.set_msg('')]
	
	key 'UP'   action context_menu.up
	key 'DOWN' action context_menu.down
	
	button:
		size 1.0
		alpha 0.01
		
		ground im.rect('#000')
		hover  im.rect('#000')
		mouse  False
		action [hide_screen('context_menu'), info.set_msg('')]
	
	image im.rect('#FFF'):
		xsize context_menu.xsize
		ysize len(context_menu.items) * context_menu.text_size
		
		xanchor 1.0
		pos context_menu.pos
	
		vbox:
			$ selected_index = context_menu.selected_index
			for index, item in enumerate(context_menu.items):
				if item.actions is None:
					text _(item.text):
						xsize      context_menu.xsize
						text_size  context_menu.text_size
						text_align 'center'
						color      0x555555
				else:
					$ image = context_menu.ground if index != selected_index else context_menu.hover
					textbutton (' ' + _(item.text)):
						xsize  context_menu.xsize
						ysize  context_menu.text_size
						
						text_size  context_menu.text_size
						text_align 'left'
						color      0
						
						ground image
						hover  image
						
						hovered   [info.set_msg(item.key), SetDict(context_menu, 'selected_index', index)]
						unhovered info.set_msg_if_prev('', item.key)
						
						action context_menu.select
	
	if context_menu.selected_index is not None:
		key 'RETURN' action context_menu.select
	
	for item in context_menu.key_items:
		key item.key:
			action [hide_screen('context_menu'), info.set_msg('')] + item.actions
