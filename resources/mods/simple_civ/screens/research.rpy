init python:
	technology_names = []
	technology_extra = {}
	
	for resource in info.simple_resources:
		technology_names.append(resource)
		
		building_list = buildings[resource]
		building = building_list[0]
		technology_extra[resource] = building
	
	for building in common_buildings:
		technology_names.append(building)
	
	technology_costs = [0, 100, 300, 900, 2700]
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('research')


screen research:
	modal True
	zorder 100
	
	key 'ESCAPE' action hide_screen('research')
	
	button:
		size   1.0
		alpha  0.3
		mouse  False
		ground im.rect('#000')
		hover  im.rect('#000')
		
		action hide_screen('research')
	
	$ technology_progress = sc_map.player.technology_progress
	image im.rect('#DDA'):
		size (600, max(350, int(get_stage_height() * 0.7)))
		align 0.5
		
		text _('Research'):
			xalign 0.5
			color 0xFFFFFF
			outlinecolor 0
			text_size info.text_size * 1.5
			font 'Arial'
		
		hbox:
			align 0.5
			spacing 30
			
			vbox:
				spacing 10
				
				text _('Name'):
					xalign 0.5
					text_size info.text_size
					color 0x0000FF
				
				for building in technology_names:
					python:
						name = _(building)
						if technology_extra.has_key(building):
							name += ' (' + _(technology_extra[building]) + ')'
					
					text name:
						font 'Consola'
						text_size info.text_size
						color 0
			
			vbox:
				spacing 10
				
				text _('Progress'):
					xalign 0.5
					text_size info.text_size
					color 0x0000FF
				
				for building in technology_names:
					text (str(technology_progress[building]) + '/4'):
						xalign 0.5
						font 'Consola'
						text_size info.text_size
						color 0x00FF00
						outlinecolor 0
						ysize info.text_size
			
			vbox:
				spacing 10
				
				text _('Cost'):
					xalign 0.5
					text_size info.text_size
					color 0x0000FF
				
				for building in technology_names:
					if technology_progress[building] < 4:
						$ cost = technology_costs[technology_progress[building] + 1]
						text str(cost):
							xalign 0.5
							text_size info.text_size
							color 0xFFFF00 if cost <= sc_map.player.science else 0xFF0000
							outlinecolor 0
							ysize info.text_size
					else:
						null:
							size info.text_size
			
			vbox:
				spacing 10
				
				text _('Explore'):
					xalign 0.5
					text_size info.text_size
					color 0x0000FF
				
				for building in technology_names:
					if technology_progress[building] < 4 and sc_map.player.science >= technology_costs[technology_progress[building] + 1]:
						textbutton '+':
							style 'btn'
							xalign 0.5
							size info.text_size
							text_size info.text_size - 2
							color 0x00FF00
							outlinecolor 0
							action sc_map.player.explore(building)
					else:
						null:
							size info.text_size
