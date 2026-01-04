init python:
	persistent.setdefault('ended_conversations', [])
	
	
	def conversation__show():
		conversation.show_time = get_game_time()
		conversation.hide_time = None
		show_screen('conversation')
	
	def conversation__hide():
		conversation.hide_time = get_game_time()
	
	
	def conversation__set_random_pose():
		conversation.cur_pose = cur_pose = random.randint(0, 4)
		conversation.cur_pose_time = random.randint(10, 50)
		conversation.cur_pose_changed_time = get_game_time()
		
		conversation.prev_image = conversation.image
		conversation.prev_image_pos = conversation.image_pos
		
		conversation.image = 'mods/main_menu/conversations/poses/%s.webp' % cur_pose
		conversation.image_pos = conversation.pose_coords[cur_pose]
	
	
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
		screen_tmp.image = None
		
		cur_pose = conversation.cur_pose
		if cur_pose is None: return
		
		if not conversation.hide_time:
			dtime = get_game_time() - conversation.show_time
			screen_tmp.alpha = dtime / conversation.appearance_time
		else:
			dtime = get_game_time() - conversation.hide_time
			screen_tmp.alpha = 1 - dtime / conversation.appearance_time
		screen_tmp.alpha = in_bounds(screen_tmp.alpha, 0, 1)
		
		dtime = get_game_time() - conversation.cur_pose_changed_time
		if dtime > conversation.cur_pose_time:
			conversation.set_random_pose()
			dtime = 0
		if conversation.prev_image == conversation.image:
			screen_tmp.image_alpha = 1
		else:
			screen_tmp.image_alpha = in_bounds(dtime / conversation.appearance_time, 0, 1)
		screen_tmp.prev_image_alpha = 1 - screen_tmp.image_alpha
		
		screen_tmp.zorder = 1 if cur_pose > 1 else -1
		screen_tmp.has_action = False
		
		if cur_pose == 0: return
		if screen_tmp.alpha != 1: return
		if conversation.wait_before_time is None: return
		if get_game_time() < conversation.wait_before_time: return
		
		screen_tmp.zorder = 1
		screen_tmp.has_action = True
		
		alpha = 0.33 if get_game_time() - conversation.wait_before_time < 2 else 0.01
		
		image = conversation.image
		if image.endswith('/4.webp'):
			image = image[:-len('/4.webp')] + '/4_marker.webp'
		
		screen_tmp.cur_pose_ground = im.recolor(image, 0, 128, 255, 255 * alpha)
		screen_tmp.cur_pose_hover  = im.recolor(image, 0, 128, 255, 64)
	
	
	conversation = SimpleObject()
	build_object('conversation')
	
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
	
	conversation.image = None
	conversation.image_pos = None
	conversation.set_random_pose()


label conversation_start:
	$ hide_screen('btns')
	$ conversation.hide()
	$ db.skip_tab = False
	
	show rn 2 smile with dissolve
	
	$ renpy.call(conversation.label)
	
	hide rn with dissolve
	window hide
	
	$ persistent.ended_conversations = persistent.ended_conversations + [conversation.part_num]
	
	$ conversation.image = None
	$ conversation.set_random_pose()
	
	$ conversation.prepare(wait = True)
	
	$ conversation.show()
	$ show_screen('btns')
	$ db.skip_tab = False


screen conversation:
	xzoom get_stage_width()  / 2560
	yzoom get_stage_height() / 1440
	
	$ screen_tmp = SimpleObject()
	$ conversation.update()
	
	zorder screen_tmp.zorder
	alpha  screen_tmp.alpha
	
	if conversation.prev_image:
		image conversation.prev_image:
			skip_mouse True
			pos conversation.prev_image_pos
			size get_image_size(conversation.prev_image)
			alpha screen_tmp.prev_image_alpha
	
	if conversation.image:
		image conversation.image:
			skip_mouse True
			pos conversation.image_pos
			size get_image_size(conversation.image)
			alpha screen_tmp.image_alpha
		
		if screen_tmp.has_action:
			button:
				pos conversation.image_pos
				
				corner_sizes 0
				size get_image_size(conversation.image)
				
				ground screen_tmp.cur_pose_ground
				hover  screen_tmp.cur_pose_hover
				
				action conversation.start
