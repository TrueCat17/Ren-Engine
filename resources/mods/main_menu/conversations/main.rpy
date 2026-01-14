init python:
	persistent.setdefault('ended_conversations', [])
	
	
	def conversation__show():
		conversation.show_time = get_game_time()
		conversation.hide_time = None
		conversation.update()
		show_screen('conversation')
		show_screen('civ_screen')
	
	def conversation__hide():
		conversation.hide_time = get_game_time()
	
	
	def conversation__set_random_pose():
		conversation.prev_pose = conversation.get('cur_pose')
		
		conversation.cur_pose = cur_pose = random.randint(0, 4)
		conversation.cur_pose_time = random.randint(30, 60)
		conversation.cur_pose_changed_time = get_game_time()
		
		conversation.prev_image = conversation.cur_image
		conversation.prev_image_pos = conversation.cur_image_pos
		
		conversation.cur_image = conversation.image_template % cur_pose
		conversation.cur_image_pos = conversation.pose_coords[cur_pose]
	
	
	def conversation__prepare(wait):
		parts = []
		part_num = 0
		while True:
			part_num += 1
			if part_num in persistent.ended_conversations:
				continue
			if not renpy.has_label('conv_part_%02i' % part_num):
				break
			parts.append(part_num)
		
		if not parts:
			conversation.label = None
			conversation.wait_before_time = None
			return
		
		conversation.part_num = random.choice(parts)
		conversation.label = 'conv_part_%02i' % conversation.part_num
		
		conversation.wait_before_time = get_game_time()
		if wait:
			conversation.wait_before_time += random.randint(20, 40)
	
	
	def conversation__start():
		if conversation.label:
			renpy.call('conversation_start')
	
	
	def conversation__update():
		if not has_screen('conversation'): return
		if conversation.cur_pose is None: return
		
		if not conversation.hide_time:
			dtime = get_game_time() - conversation.show_time
			conversation.alpha = dtime / conversation.appearance_time
		else:
			dtime = get_game_time() - conversation.hide_time
			conversation.alpha = 1 - dtime / conversation.appearance_time
		conversation.alpha = in_bounds(conversation.alpha, 0, 1)
		
		dtime = get_game_time() - conversation.cur_pose_changed_time
		if dtime > conversation.cur_pose_time:
			conversation.set_random_pose()
			dtime = 0
		if conversation.prev_image == conversation.cur_image:
			conversation.cur_image_alpha = 1
		else:
			conversation.cur_image_alpha = in_bounds(dtime / conversation.appearance_time, 0, 1)
		conversation.prev_image_alpha = 1 - conversation.cur_image_alpha
		
		if conversation.cur_pose == 1:
			conversation.civ_screen_alpha = conversation.cur_image_alpha
		elif conversation.prev_pose == 1:
			conversation.civ_screen_alpha = conversation.prev_image_alpha
		else:
			conversation.civ_screen_alpha = 0
		
		
		conversation.zorder = 1 if conversation.cur_pose > 1 else -1
		conversation.has_action = False
		
		if conversation.cur_pose == 0: return
		if conversation.alpha != 1: return
		if conversation.wait_before_time is None: return
		if get_game_time() < conversation.wait_before_time: return
		
		conversation.zorder = 1
		conversation.has_action = True
		
		alpha = 0.33 if get_game_time() - conversation.wait_before_time < 2 else 0.01
		
		image = conversation.cur_image
		if image.endswith('/4.webp'):
			image = image[:-len('/4.webp')] + '/4_marker.webp'
		
		conversation.cur_pose_ground = im.recolor(image, 0, 128, 255, 255 * alpha)
		conversation.cur_pose_hover  = im.recolor(image, 0, 128, 255, 64)
	
	
	conversation = SimpleObject()
	build_object('conversation')
	
	conversation.image_template = 'mods/main_menu/conversations/poses/%s.webp'
	
	conversation.appearance_time = 0.5
	
	conversation.label = None
	conversation.wait_before_time = None
	
	conversation.pose_coords = (
		( 137, 791),
		( 459, 751),
		(1504, 750),
		(2206, 810),
		(2195, 761),
	)
	
	conversation.civ_screen_image = conversation.image_template % 'civ_screen'
	conversation.civ_screen_pos = (737, 631)
	
	conversation.cur_image = None
	conversation.cur_image_pos = None
	conversation.set_random_pose()
	
	signals.add('enter_frame', conversation.update)


label conversation_start:
	$ hide_screen('btns')
	$ conversation.hide()
	$ db.skip_tab = False
	
	show rn 2 smile with dissolve
	
	$ renpy.call(conversation.label)
	
	hide rn with dissolve
	window hide
	
	$ persistent.ended_conversations = persistent.ended_conversations + [conversation.part_num]
	
	$ conversation.cur_image = None
	$ conversation.set_random_pose()
	
	$ conversation.prepare(wait = True)
	
	$ conversation.show()
	$ show_screen('btns')
	$ db.skip_tab = False


screen civ_screen:
	zorder -1
	
	xzoom get_stage_width()  / 2560
	yzoom get_stage_height() / 1440
	
	if conversation.civ_screen_alpha:
		image conversation.civ_screen_image:
			skip_mouse True
			pos conversation.civ_screen_pos
			size get_image_size(conversation.civ_screen_image)
			alpha conversation.civ_screen_alpha


screen conversation:
	xzoom get_stage_width()  / 2560
	yzoom get_stage_height() / 1440
	
	zorder conversation.zorder
	alpha  conversation.alpha
	
	if conversation.prev_image:
		image conversation.prev_image:
			skip_mouse True
			pos conversation.prev_image_pos
			size get_image_size(conversation.prev_image)
			alpha conversation.prev_image_alpha
	
	if conversation.cur_image:
		image conversation.cur_image:
			skip_mouse True
			pos conversation.cur_image_pos
			size get_image_size(conversation.cur_image)
			alpha conversation.cur_image_alpha
		
		if conversation.has_action:
			button:
				pos conversation.cur_image_pos
				
				corner_sizes 0
				size get_image_size(conversation.cur_image)
				
				ground conversation.cur_pose_ground
				hover  conversation.cur_pose_hover
				
				action conversation.start
