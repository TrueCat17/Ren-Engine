init -1000 python:
	rpg_event_processing = False
	
	def get_place_labels():
		quests = get_started_quests()
		usual_label = cur_location_name + '__' + (cur_place_name or 'unknown')
		
		res = []
		for quest in quests:
			res.extend(get_glob_labels(quest + '__' + usual_label))
		res.extend(get_glob_labels(usual_label))
		return res
	
	def get_unique_place_labels(labels):
		res = []
		for label in labels:
			if not label or not renpy.has_label(label): continue
			if label in res:
				res.remove(label)
			res.append(label)
		return res


label rpg_loop:
	while True:
		call rpg_update
		pause 1.0 / get_fps()


label rpg_update:
	if not get_rpg_control():
		return
	
	python:
		if rpg_events:
			rpg_event_processing = True
			
			rpg_cur_labels = get_unique_place_labels(get_place_labels())
			rpg_processing_events, rpg_events = rpg_events, set()
		else:
			rpg_processing_events = None
		
		if cur_exit and renpy.has_label('on__' + cur_location_name):
			renpy.call('on__' + cur_location_name)
	
	while rpg_processing_events:
		python:
			rpg_event = rpg_processing_events.pop()
			rpg_event_object = None
			if type(rpg_event) in (tuple, list):
				rpg_event, rpg_event_object = rpg_event
			
		$ rpg_event_label_index = 0
		while rpg_event_label_index < len(rpg_cur_labels):
			python:
				rpg_cur_label = rpg_cur_labels[rpg_event_label_index]
				rpg_event_label_index += 1
			
				rpg_event_stop = True
				renpy.call(rpg_cur_label)
			if rpg_event_stop:
				break
	
	python:
		rpg_event = rpg_event_object = None
		rpg_event_processing = False
	
