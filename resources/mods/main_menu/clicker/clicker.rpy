init -1 python:
	def clicker__show():
		global quick_menu
		quick_menu = False
		
		show_screen('clicker')
		clicker.show_time = get_game_time()
		clicker.hide_time = None
		clicker.msg = ''
		clicker.update_info()
	
	def clicker__close():
		if clicker.hide_time: return
		if get_game_time() - clicker.show_time < clicker.appearance_time: return
		
		clicker.hide_time = get_game_time()
	
	
	def clicker__set_msg(msg):
		clicker.msg = msg
	def clicker__clear_msg(prev_msg):
		if clicker.msg == prev_msg:
			clicker.msg = ''
	
	def clicker__int_to_parted_str(v):
		v = str(v)
		parts = []
		while v:
			parts.insert(0, v[-3:])
			v = v[:-3]
		return ' '.join(parts)
	
	
	def clicker__update():
		if clicker.hide_time:
			dtime = get_game_time() - clicker.hide_time
			screen_tmp.alpha = 1 - dtime / clicker.disappearance_time
			if screen_tmp.alpha < 0:
				hide_screen('clicker')
				global quick_menu
				quick_menu = True
		else:
			dtime = get_game_time() - clicker.show_time
			screen_tmp.alpha = dtime / clicker.appearance_time
		
		reward = clicker.int_to_parted_str(clicker.get_cur_reward())
		money = clicker.int_to_parted_str(clicker.data['money'])
		crd = _('crd.')
		
		screen_tmp.crd_reward_text = '+%s %s/%s' % (reward, crd, _('trip'))
		screen_tmp.crd_text = '%s %s' % (money, crd)
	
	def clicker__save():
		if persistent.clicker_data != clicker.data:
			persistent.clicker_data = clicker.data.copy()
	set_interval(clicker__save, 1.0)
	
	
	def clicker__get_upgrade_price(equipment_name, equipment_level):
		base = clicker.base * clicker.equipment_unlock_price[equipment_name]
		
		k = clicker.equipment_price_k_for_one_level[equipment_name]
		price = k ** equipment_level
		
		discount_level = clicker.data['upgrade skills']
		discount = (1 - clicker.percentage_discount_for_one_level / 100) ** discount_level
		
		return round(base * price * discount)
	
	def clicker__get_cur_reward():
		res = clicker.base
		for name, k in clicker.equipment_prod_k_for_one_level.items():
			level = clicker.data[name]
			res *= 1 + k * level
		return round(res)
	
	def clicker__get_diff_reward(equipment_name):
		cur_reward = clicker.get_cur_reward()
		
		clicker.data[equipment_name] += 1
		next_reward = clicker.get_cur_reward()
		clicker.data[equipment_name] -= 1
		
		return next_reward - cur_reward
	
	
	def clicker__update_info():
		prices = {}
		max_price_len = 0
		
		diff_rewards = {}
		max_diff_reward_len = 0
		
		for equipment_name, equipment_level in clicker.data.items():
			if equipment_name in ('money', 'trips', 'success', 'version'):
				continue
			
			price = clicker.get_upgrade_price(equipment_name, equipment_level)
			price_str = clicker.int_to_parted_str(price)
			
			prices[equipment_name] = price_str
			max_price_len = max(max_price_len, len(price_str))
			
			diff_reward = clicker.get_diff_reward(equipment_name)
			diff_reward_str = '+' + clicker.int_to_parted_str(diff_reward)
			
			diff_rewards[equipment_name] = diff_reward_str
			max_diff_reward_len = max(max_diff_reward_len, len(diff_reward_str))
		
		info = clicker.info = {}
		for equipment_name, price_str in prices.items():
			price_str = price_str.rjust(max_price_len)
			diff_reward_str = diff_rewards[equipment_name].rjust(max_diff_reward_len)
			
			res = price_str
			if diff_reward_str.strip() != '+0':
				res += ', %s/%s' % (diff_reward_str, _('trip'))
#				res += ', %s' % (int(price_str.replace(' ', '')) // int(diff_reward_str.replace(' ', '')))
			else:
				res += ' {color=#CCC}(%s){/color}' % _('add discount')
			
			info[equipment_name] = res
	
	def clicker__upgrade(equipment_name, equipment_level):
		if equipment_level:
			price = clicker.get_upgrade_price(equipment_name, equipment_level)
		else:
			unlock_price = clicker.equipment_unlock_price[equipment_name]
			price = clicker.base * unlock_price
		
		if price > clicker.data['money']:
			clicker.no_money_time = get_game_time()
			return
		
		clicker.data['money'] -= price
		clicker.data[equipment_name] += 1
		
		clicker.update_info()
	
	
	def clicker__make_trip():
		if get_game_time() - clicker.last_trip_time < clicker.trip_time:
			return
		
		clicker.last_trip_time = get_game_time()
		clicker.data['trips'] += 1
		clicker.data['money'] += clicker.get_cur_reward()
		
		if clicker.data['money'] >= clicker.target_millions * 1e6:
			if not clicker.data['success']:
				clicker.data['success'] = True
				notification.out('Win')
	
	
	def clicker__reset():
		clicker.data = {
			'repair':         0,
			'mining':         0,
			'shield':         0,
			'transport':      0,
			'upgrade skills': 0,
			'storage':        0,
			'market':         0,
			'bank':           0,
			'production':     0,
			'extra ships':    0,
			
			'money':   0,
			'trips':   0,
			'success': False,
			'version': clicker.version,
		}
		persistent.clicker_data = clicker.data.copy()
		
		clicker.update_info()
	
	
	clicker = SimpleObject()
	build_object('clicker')
	
	clicker.version = 1
	
	clicker.show_time = 0
	clicker.hide_time = None
	clicker.msg = ''
	
	clicker.no_money_time = -1
	clicker.last_trip_time = 0
	
	clicker.equipment_prod_k_for_one_level = {
		'repair':      0.10,
		'mining':      0.15,
		'shield':      0.20,
		'transport':   0.25,
		'storage':     0.30,
		'market':      0.35,
		'bank':        0.40,
		'production':  0.45,
		'extra ships': 0.50,
	}
	clicker.percentage_discount_for_one_level = 5
	
	clicker.equipment_price_k_for_one_level = {
		'repair':         1.5,
		'mining':         2.0,
		'shield':         2.5,
		'transport':      3.0,
		'upgrade skills': 3.5,
		'storage':        4.0,
		'market':         4.5,
		'bank':           5.0,
		'production':     5.5,
		'extra ships':    6.0,
	}
	
	clicker.equipment_unlock_price = {
		'repair':          5,
		'mining':         10,
		'shield':         20,
		'transport':      40,
		'upgrade skills': 80,
		'storage':       160,
		'market':        320,
		'bank':          640,
		'production':   1280,
		'extra ships':  2560,
	}
	
	
	clicker.base = 10
	clicker.target_millions = 999
	
	if persistent.get('clicker_data', {}).get('version', 0) != clicker.version:
		clicker.reset()
	else:
		clicker.data = persistent.clicker_data.copy()
	
	
	clicker.trip_time = 0.25
	
	clicker.appearance_time = 0.35
	clicker.disappearance_time = 0.35
	
	clicker.fog = im.rect('#0002')
	clicker.bg  = im.rect('#08F7')
	
	clicker.trip_progress_bar = style.clicker_button.hover


screen clicker:
	modal True
	zorder 10
	
	$ screen_tmp = SimpleObject()
	$ clicker.update()
	
	alpha screen_tmp.alpha
	
	button:
		ground clicker.fog
		hover  clicker.fog
		size   1.0
		mouse  False
		action clicker.close
	
	key 'ESCAPE' action clicker.close
	
	image clicker.bg:
		size 0.82
		align 0.5
		
		vbox:
			xsize 0.78
			pos 0.02
			spacing 0.01
			
			text (_('Equipment levels') + ':'):
				style 'clicker_text'
			
			for equipment_name in clicker.info.keys():
				$ screen_tmp.level = clicker.data[equipment_name]
				
				hbox:
					xpos 50 / 1200
					spacing 0.02
					
					text (_(equipment_name) + ':'):
						style 'clicker_text'
						xsize 250 / 1200
						yalign 0.5
					
					text str(screen_tmp.level):
						style 'clicker_text'
						font 'Consola'
						text_align 'right'
						xsize 70 / 1200
						yalign 0.5
					
					if screen_tmp.level:
						textbutton '+':
							style 'clicker_upgrade_button'
							action clicker.upgrade(equipment_name, screen_tmp.level)
						
						text clicker.info[equipment_name]:
							style 'clicker_text'
							font 'Consola'
							yalign 0.5
					
					else:
						$ screen_tmp.unlock_price = clicker.equipment_unlock_price[equipment_name]
						$ screen_tmp.price = clicker.base * screen_tmp.unlock_price
						$ screen_tmp.price_str = clicker.int_to_parted_str(screen_tmp.price)
						textbutton ('%s: %s %s' % (_('Unlock'), screen_tmp.price_str, _('crd.'))):
							style 'clicker_button'
							xsize 450 / 1200
							action clicker.upgrade(equipment_name, 0)
		
		
		text clicker.msg:
			style 'clicker_text'
			text_align 'right'
			xpos 0.98
			ypos 0.02
			xanchor 1.0
		
		vbox:
			xpos 0.02
			ypos 0.98
			yanchor 1.0
			spacing 0.02
			
			text ('%s: %s' % (_('Trips'), clicker.data['trips'])):
				style 'clicker_text'
			
			textbutton _('Make a trip'):
				style 'clicker_button'
				
				hovered   clicker.set_msg('Shift+Enter')
				unhovered clicker.clear_msg('Shift+Enter')
				action clicker.make_trip
		
		if hotkeys.shift:
			key 'RETURN' action clicker.make_trip
		
		
		textbutton _('Reset'):
			style 'clicker_button'
			xalign 0.5
			ypos 0.98
			yanchor 1.0
			
			hovered   clicker.set_msg('R')
			unhovered clicker.clear_msg('R')
			action clicker.reset
		
		key 'R' action clicker.reset
		
		
		vbox:
			pos 0.98
			anchor 1.0
			spacing 0.02
			
			$ screen_tmp.xsize = style.clicker_button.get_current('xsize')
			$ screen_tmp.ysize = style.clicker_button.get_current('ysize')
			
			null:
				xsize screen_tmp.xsize
				xalign 1.0
				text (_('%i of %s million') % (clicker.data['money'] // 1e6, clicker.target_millions)):
					style 'clicker_text'
					color '#0F0' if clicker.data['success'] else '#FF0'
					xalign 1.0
			
			null:
				xsize screen_tmp.xsize
				xalign 1.0
				text screen_tmp.crd_text:
					style 'clicker_text' if get_game_time() - clicker.no_money_time > 0.5 else 'clicker_text_error'
					xalign 1.0
			
			null:
				xsize screen_tmp.xsize
				ysize screen_tmp.ysize
				
				null:
					clipping True
					xalign 1.0
					
					$ screen_tmp.k = 1 - (get_game_time() - clicker.last_trip_time) / clicker.trip_time
					xsize screen_tmp.xsize * screen_tmp.k
					ysize screen_tmp.ysize
					alpha 1 if screen_tmp.k > 0 else 0
					
					image clicker.trip_progress_bar:
						corner_sizes -1
						xalign 1.0
						xsize screen_tmp.xsize
						ysize screen_tmp.ysize
				
				text screen_tmp.crd_reward_text:
					style 'clicker_text'
					xalign 1.0
					text_valign 'center'
