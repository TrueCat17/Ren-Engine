init python:
	def default_get_place_label():
		return cur_location_name + '__' + (cur_place_name or 'unknown')

label rpg_loop:
	while True:
		call rpg_update
		pause 1.0 / get_fps()


label rpg_update:
	if not get_rpg_control():
		return
	
	if inventory_action is not None:
		$ cur_quests_labels = quest_get_object_labels(inventory_action, inventory_action_object['name'])
		
		$ me.move_kind = 'stay'
		$ set_rpg_control(False)
		if len(cur_quests_labels) == 1:
			call expression cur_quests_labels[0][1]
		elif len(cur_quests_labels) > 1:
			$ me.set_pose("stance")
			window show
			
			narrator (_("Several active quests intersect on action <%s> for item") % _(inventory_action)) + " (" + str(len(cur_quests_labels)) + ")."
			
			$ choose_menu_variants = [name for name, label in cur_quests_labels]
			$ renpy.call_screen('choose_menu', 'choose_menu_result')
			
			window hide
			$ name, label = cur_quests_labels[choose_menu_result]
			call expression label
		$ set_rpg_control(True)
		
		$ inventory_action = None
	
	python:
		exit = get_location_exit()
		if exit:
			set_location(exit.to_location_name, exit.to_place_name)
			
			if renpy.has_label('on__' + cur_location_name):
				renpy.call('on__' + cur_location_name)
	
	python:
		near_location_object = None
		if exec_action:
			near_location_object = get_near_location_object_for_inventory()
			if near_location_object is not None:
				left = add_to_inventory(near_location_object.type, 1)
				if left == 0:
					remove_location_object(cur_location_name, me, near_location_object.type, 1)
					inventory_do_action('taking', location_objects[near_location_object.type])
				else:
					near_location_object = None
		
		cur_place_name = get_location_place() if near_location_object is None else None
		cur_exec_label = cur_location_name + '__' + (cur_place_name or 'unknown')
		
		if sit_action:
			if me.get_pose() == 'sit':
				stand_up = True
				me.stand_up()
			else:
				objs = get_near_sit_objects()
				if objs:
					sit_down = True
					obj, point = objs[0]
					me.sit_down(obj)
	
	if (exec_action or sit_action) and renpy.has_label(cur_exec_label):
		call expression cur_exec_label
	else:
		$ cur_label = globals().get('get_place_label', default_get_place_label)()
		if renpy.has_label(cur_label):
			call expression cur_label
	
	python:
		cur_quests_labels = quest_get_labels(cur_location_name, cur_place_name)
		if cur_quests_labels:
			save_rpg_control()
			set_rpg_control(False)
	
	if len(cur_quests_labels) == 1:
		call expression cur_quests_labels[0][1]
	elif len(cur_quests_labels) > 1:
		$ me.set_pose("stance")
		window show
		narrator _("Several active quests intersect in this place") + " (" + str(len(cur_quests_labels)) + ")."
		
		$ choose_menu_variants = [name for name, label in cur_quests_labels]
		$ renpy.call_screen('choose_menu', 'choose_menu_result')
		
		""
		$ name, label = cur_quests_labels[choose_menu_result]
		call expression label
		
		window hide
	
	python:
		return_prev_rpg_control()
		
		sit_action = False
		sit_down = False
		stand_up = False
		
		exec_action = False

