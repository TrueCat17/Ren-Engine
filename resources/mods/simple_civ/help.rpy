init python:
	
	def sc_get_help_data():
		res = {}
		
		green  = res['green']  = '{color=#0F0}{outlinecolor=0}'
		red    = res['red']    = '{color=#F00}{outlinecolor=0}'
		blue   = res['blue']   = '{color=#0FF}{outlinecolor=0}'
		yellow = res['yellow'] = '{color=#FF0}{outlinecolor=0}'
		orange = res['orange'] = '{color=#F80}{outlinecolor=0}'
		end    = res['end'] = '{/outlinecolor}{/color}'
		
		
		res['building_powers'] = str(tuple('x%s' % k for k in sc_buildings.power[2:]))
		
		
		building_list = []
		for resource in sc_info.simple_resources:
			building = sc_buildings.on_resource[resource][0]
			if building not in building_list:
				building_list.append(building)
		building_list.extend(sc_buildings.common)
		
		building_str = ''
		for building in building_list:
			for level in (1, 2, 3, 4):
				building_str += '[%s%s-%s%s]:\n' % (green, _(building), level, end)
				for resource, count in sc_buildings.price['%s-%s' % (building, level)]:
					building_str += '    %s: %s%s%s\n' % (_(resource), yellow, count, end)
			building_str += '\n'
		res['building_price'] = building_str.rstrip()
		
		support_str = ''
		for building in building_list:
			support_str += '[%s%s%s]:\n' % (green, _(building), end)
			for resource, count in sc_buildings.support[building]:
				support_str += '    %s: %s%s%s\n' % (_(resource), yellow, count, end)
			support_str += '\n'
		res['building_support'] = support_str.rstrip()
		
		production_str = ''
		for building in building_list:
			takes = sc_buildings.takes.get(building)
			makes = sc_buildings.makes.get(building)
			if not takes and not makes: continue
			
			production_str += '[%s%s%s]:\n' % (yellow, _(building), end)
			
			if takes:
				production_str += '  %s:\n' % _('Takes')
				for resource, count in takes:
					production_str += '    %s (%s%s%s)\n' % (_(resource), orange, count, end)
			
			production_str += '  %s:\n' % _('Makes')
			
			if building == 'barracks':
				production_str += '    %s%s%s\n' % (green, _('Big Force'), end)
			else:
				resource, count = makes
				production_str += '    %s (%s%s%s)\n' % (_(resource), green, count, end)
			production_str += '\n'
		res['building_production'] = production_str.rstrip()
		
		achivement_str = ''
		for name, description in sc_ach.descriptions.items():
			add = '[%s]' % _(name)
			if description:
				add += ': ' + _(description)
			
			if name in persistent.sc_ach:
				add = yellow + add + end
			
			achivement_str += add + '\n\n'
		res['achivements'] = achivement_str.rstrip()
		
		return res
	
	help.set_file(make_vars = sc_get_help_data)
