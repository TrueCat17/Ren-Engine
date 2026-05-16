init -1 python:
	def sc_buildings__get_short_name(building_full_name):
		if 'career' in building_full_name:
			return 'career'
		return building_full_name
	def sc_buildings__get_full_name(resource, building):
		if building == 'career':
			return resource + ' career'
		return building
	
	
	sc_buildings = SimpleObject()
	build_object('sc_buildings')
	
	
	sc_buildings.full_building_for_resource = {
		'food': 'farm',
		'wood': 'sawmill',
		'stone': 'stone career',
		'coal':   'coal career',
		'metal': 'metal career',
		'cement': 'cement factory',
		'steel':  'metal factory',
		'science': 'college',
	}
	
	sc_buildings.building_for_resource = {}
	for resource, building_full_name in sc_buildings.full_building_for_resource.items():
		sc_buildings.building_for_resource[resource] = sc_buildings.get_short_name(building_full_name)
	
	
	sc_buildings.on_resource = {}
	sc_buildings.on_resource['food'] = ['farm']
	sc_buildings.on_resource['wood'] = ['sawmill']
	
	for resource in ('stone', 'coal', 'metal'):
		sc_buildings.on_resource[resource] = ['career']
	
	sc_buildings.common = ('college', 'cement factory', 'metal factory', 'barracks')
	for array in sc_buildings.on_resource.values():
		array.extend(sc_buildings.common)
	
	
	sc_buildings.power = [None, 1, 2, 4, 7]
