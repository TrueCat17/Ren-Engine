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
		
		building_str = '[{color=#0F0}{outlinecolor=0}%s{/outlinecolor}{/color}]:\n' % _('road')
		for resource, count in building_cost['road-1'].items():
			building_str += '    %s: {color=#FF0}{outlinecolor=0}%s{/outlinecolor}{/color}\n' % (_(resource), count)
		building_str += '\n'
		for building in building_list:
			for i in (1, 2, 3, 4):
				building_str += '[{color=#0F0}{outlinecolor=0}%s-%s{/outlinecolor}{/color}]:\n' % (_(building), i)
				for resource, count in building_cost[building + '-' + str(i)].items():
					building_str += '    %s: {color=#FF0}{outlinecolor=0}%s{/outlinecolor}{/color}\n' % (_(resource), count)
			building_str += '\n'
		res['building_cost'] = building_str.rstrip()
		
		support_str = ''
		for building in ['road'] + building_list:
			support_str += '[{color=#0F0}{outlinecolor=0}%s{/outlinecolor}{/color}]:\n' % _(building)
			for resource, count in support_cost[building].items():
				support_str += '    %s: {color=#FF0}{outlinecolor=0}%s{/outlinecolor}{/color}\n' % (_(resource), count)
			support_str += '\n'
		res['support_cost'] = support_str.rstrip()
		
		production_str = ''
		for building in building_list:
			if building not in building_production: continue
			
			production_str += _(building) + ':\n'
			resources = building_production[building]
			if resources['from']:
				production_str += '  ' + _('From') + ':\n'
				for resource, count in resources['from'].items():
					production_str += '    %s ({color=#F80}{outlinecolor=0}%s{/outlinecolor}{/color})\n' % (_(resource), count)
			production_str += '  ' + _('Makes') + ':\n'
			for resource, count in resources['to'].items():
				production_str += '    %s ({color=#0F0}{outlinecolor=0}%s{/outlinecolor}{/color})\n' % (_(resource), count)
			production_str += '\n'
		res['building_production'] = production_str.rstrip()
		
		return res
	
	help.set_file(make_vars = get_help_data)
