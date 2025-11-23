init -1 python:
	
	sc_buildings = SimpleObject()
	
	sc_buildings.on_resource = {}
	sc_buildings.on_resource['food'] = ['farm']
	sc_buildings.on_resource['wood'] = ['sawmill']
	
	for resource in ('stone', 'coal', 'metal'):
		sc_buildings.on_resource[resource] = ['career']
	
	sc_buildings.common = ('college', 'cement factory', 'metal factory', 'barracks')
	for array in sc_buildings.on_resource.values():
		array.extend(sc_buildings.common)
	
	
	sc_buildings.power = [None, 1, 2, 4, 7]
