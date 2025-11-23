init python:
	if 'sc_ach' not in persistent:
		persistent.sc_ach = []
	
	
	def sc_ach__winner():
		for player in sc_map.players:
			if player.bot and player.building_cells:
				return False
		return True
	
	def sc_ach__loser():
		player = sc_map.players[0]
		return not player.building_cells
	
	def sc_ach__normal_one():
		return sc_map.bonus_for_bots == 50 and sc_ach.winner()
	
	def sc_ach__scientist():
		player = sc_map.players[0]
		for level in player.technological_progress.values():
			if level != 4:
				return False
		return True
	
	def sc_ach__factory_worker():
		player = sc_map.players[0]
		for cell in player.building_cells:
			if cell.disabled: continue
			if cell.building != 'cement factory': continue
			return True
		return False
	
	def sc_ach__economist():
		player = sc_map.players[0]
		for cell in player.building_cells:
			if cell.disabled: continue
			if cell.building != 'sawmill': continue
			if cell.building_level != 4: continue
			return True
		return False
	
	def sc_ach__professor():
		player = sc_map.players[0]
		for cell in player.building_cells:
			if cell.disabled: continue
			if cell.building != 'college': continue
			if cell.building_level != 4: continue
			return True
		return False
	
	def sc_ach__conqueror():
		return sc_map.xsize == 80 and sc_map.ysize == 80 and sc_ach.winner()
	
	def sc_ach__mad_scientist():
		player = sc_map.players[0]
		count = 0
		for cell in player.building_cells:
			if cell.disabled: continue
			if cell.building != 'college': continue
			if cell.building_level != 4: continue
			count += 1
		return count >= 100
	
	def sc_ach__industrialist():
		player = sc_map.players[0]
		count = 0
		for cell in player.building_cells:
			if cell.disabled: continue
			if cell.building != 'metal factory': continue
			if cell.building_level != 4: continue
			count += 1
		return count >= 20
	
	
	def sc_ach__check():
		if sc_map.count_of_bots == 4 and 'observer' not in persistent.sc_ach:
			persistent.sc_ach = persistent.sc_ach + ['observer']
			notification.out('%s\n[%s]', _('Achivement'), _('observer'))
			return
		
		if sc_map.count_of_players != 4 or sc_map.count_of_bots != 3:
			return
		
		for name in sc_ach.descriptions:
			if name == 'observer': continue
			if name in persistent.sc_ach: continue
			
			func = sc_ach[name.replace(' ', '_')]
			if not func(): continue
			
			persistent.sc_ach = persistent.sc_ach + [name]
			notification.out('%s\n[%s]', _('Achivement'), _(name))
	
	
	sc_ach = SimpleObject()
	build_object('sc_ach')
	
	sc_ach.descriptions = {
		'winner': '',
		'loser':  '',
		'normal one': 'Defeat bots on normal (10) difficulty level',
		'observer':   'Start a game where all 4 players are bots',
		
		'scientist':      'Learn all technologies',
		'factory worker': 'Build cement factory',
		'economist': 'Upgrade the sawmill to level 4',
		'professor': 'Build a level 4 college',
		
		'conqueror': 'Victory on the 80x80 map',
		'mad scientist': 'Build 100 level 4 colleges',
		'industrialist': 'Simultaneous operation of 20 level 4 metal factories',
	}
