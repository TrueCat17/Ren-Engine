init -1 python:
	
	buildings = {}
	buildings['food'] = ['farm']
	buildings['wood'] = ['sawmill']
	
	for i in ('stone', 'coal', 'metal'):
		buildings[i] = ['career']
	
	common_buildings = ('storage', 'district', 'college', 'cement factory', 'metal factory')
	for i in buildings:
		buildings[i].extend(common_buildings)
	
	
	building_powers = [0, 1, 2, 4, 7]
	