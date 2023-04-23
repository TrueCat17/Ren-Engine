init python:
	
	def help__get_data():
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
	
	def help__init():
		f = open('mods/' + get_current_mod() + '/' + _('readme.txt'))
		content = f.read().strip().replace('\t', '  ')
		f.close()
		
		help.groups = []
		help.xsizes = {}
		groups = content.split('\n\n\n')
		data = (help.get_data or dict)()
		for group in groups:
			new_str_pos = group.index('\n')
			
			name = group[0:new_str_pos]
			text = group[new_str_pos+1:] + '\n'
			
			end = -1
			while True:
				start = text.find('${', end + 1)
				if start == -1: break
				
				end = text.find('}', start, text.find('\n', start))
				if end == -1:
					out_msg('help.init, section %s' % name, 'Close-tag not found')
					break
				
				var_name = text[start + 2 : end]
				if var_name in data:
					text = text[:start] + data[var_name] + text[end + 1:]
					end += len(data[var_name]) - (end + 1 - start)
				else:
					out_msg('help.init, section %s' % name, 'var name <' + var_name + '> is undefined')
					end = len(text)
			
			help.groups.append((name, text.strip()))
			
			help.xsizes[name] = get_text_width(name, style.btn.text_size) + 10
		
		help.start_index = 0
		help.max_index = 0
		help.count_for_index = []
		if help.groups:
			help.text = help.groups[0][1]
			
			l = len(help.groups)
			for i in range(l):
				width = 0
				count = 0
				for name, text in help.groups[i:]:
					width += help.xsizes[name] + 10
					if width > get_stage_width() * 0.8 - 100: break
					count += 1
				count = max(1, count)
				help.count_for_index.append(count)
			
			i = l - 1
			while i != -1:
				if i + help.count_for_index[i] != l:
					help.max_index = i + 1
					break
				i -= 1
	
	def help__show():
		help.init()
		show_screen('help')
	
	def help__close():
		hide_screen('help')
	
	def help__on_resize():
		if has_screen('help'):
			help.init()
	
	
	build_object('help')
	help.indent = 10
	
	help.viewport_y = 0.15
	help.viewport_ysize = 1 - help.viewport_y * 2 - 0.05 # 0.05 for indents
	help.viewport_content_height = 3500
	
	slider_v_init('help', help.viewport_content_height, help.viewport_ysize)
	
	signals.add('resized_stage', help.on_resize)
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('help')


screen help:
	modal True
	zorder 1000001
	
	$ slider_v_set('help')
	
	
	key 'ESCAPE' action help.close
	
	button:
		ground im.rect('#000')
		hover  im.rect('#000')
		size (1.0, 1.0)
		alpha 0.3
		mouse False
		action help.close
	
	image im.rect('#000'):
		align (0.5, 0.5)
		xsize int(get_stage_width() * 0.8 + 10)
		ysize int(get_stage_height() * 0.8 + 10)
	
	image im.rect('#FFF'):
		align (0.5, 0.5)
		size (0.8, 0.8)
		
		vbox:
			ypos help.indent
			xsize 0.8
			spacing help.indent
			
			null:
				xalign 0.5
				xsize 0.8
				
				hbox:
					xalign 0.5
					spacing help.indent
					
					for name, text in help.groups[help.start_index : help.start_index + help.count_for_index[help.start_index]]:
						textbutton name:
							style 'btn'
							xsize help.xsizes[name]
							action [SetDict(help, 'text', text), slider_v_update(0)]
				
				if help.start_index != 0:
					textbutton '<':
						style  'btn'
						bold   True
						xalign 0.05
						xsize  style.btn.ysize
						action SetDict(help, 'start_index', max(0, help.start_index - 1))
				if help.start_index != help.max_index:
					textbutton '>':
						style  'btn'
						bold   True
						xalign 0.95
						xsize  style.btn.ysize
						action SetDict(help, 'start_index', min(help.start_index + 1, help.max_index))
			
			$ y = int(-slider_v_get_value('help') * (help.viewport_content_height - help.viewport_ysize * get_stage_height()))
			null:
				clipping True
				
				ypos help.viewport_y
				xsize 0.75
				ysize help.viewport_ysize
				
				text help.text:
					xpos 0.05
					ypos y
					xsize 0.7
					color 0x000000
					font 'Consola'
			
	null:
		align (0.88, 0.5)
		use slider_v
