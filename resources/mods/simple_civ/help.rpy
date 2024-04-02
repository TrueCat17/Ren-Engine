init python:
	
	def get_help_data():
		res = {}
		
		res['building_powers'] = str(tuple('x' + str(k) for k in building_powers[2:]))
		
		building_list = []
		for resource in info.simple_resources:
			building = buildings[resource][0]
			if building not in building_list:
				building_list.append(building)
		building_list.extend(common_buildings)
		
		building_str = '[{color=0x00FF00}{outlinecolor=0}%s{/outlinecolor}{/color}]' % _('road') + ':\n'
		for resource in info.resources:
			if resource not in building_cost['road-1']: continue
			building_str += '    %s: {color=0xFFFF00}{outlinecolor=0}%s{/outlinecolor}{/color}\n' % (_(resource), building_cost['road-1'][resource])
		building_str += '\n'
		for building in building_list:
			for i in (1, 2, 3, 4):
				building_str += '[{color=0x00FF00}{outlinecolor=0}%s-%s{/outlinecolor}{/color}]:\n' % (_(building), i)
				resources = building_cost[building + '-' + str(i)]
				for resource in info.resources:
					if resource not in resources: continue
					building_str += '    %s: {color=0xFFFF00}{outlinecolor=0}%s{/outlinecolor}{/color}\n' % (_(resource), resources[resource])
			if building != building_list[-1]:
				building_str += '\n'
		res['building_cost'] = building_str
		
		support_str = '[{color=0x00FF00}{outlinecolor=0}%s{/outlinecolor}{/color}]' % _('road') + ':\n'
		for resource in info.resources:
			if resource not in support_cost['road']: continue
			support_str += '    %s: {color=0xFFFF00}{outlinecolor=0}%s{/outlinecolor}{/color}\n' % (_(resource), support_cost['road'][resource])
		support_str += '\n'
		for building in building_list:
			support_str += '[{color=0x00FF00}{outlinecolor=0}%s{/outlinecolor}{/color}]:\n' % _(building)
			resources = support_cost[building]
			for resource in info.resources:
				if resource not in resources: continue
				support_str += '    %s: {color=0xFFFF00}{outlinecolor=0}%s{/outlinecolor}{/color}\n' % (_(resource), resources[resource])
			if building != building_list[-1]:
				support_str += '\n'
		res['support_cost'] = support_str
		
		production_str = ''
		for building in building_list:
			if building not in building_production: continue
			
			production_str += _(building) + ':\n'
			resources = building_production[building]
			if resources['from']:
				from_resources = resources['from']
				production_str += '  ' + _('From') + ':\n'
				for resource in info.resources:
					if resource not in from_resources: continue
					production_str += '    %s ({color=0xFF8000}{outlinecolor=0}%s{/outlinecolor}{/color})\n' % (_(resource), from_resources[resource])
			to_resources = resources['to']
			production_str += '  ' + _('Makes') + ':\n'
			for resource in info.resources:
				if resource not in to_resources: continue
				production_str += '    %s ({color=0x00FF00}{outlinecolor=0}%s{/outlinecolor}{/color})\n' % (_(resource), to_resources[resource])
			production_str += '\n'
		res['building_production'] = production_str[:-1]
		
		return res
	
	help.set_file(make_vars = get_help_data)
