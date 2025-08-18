init python:
	research_bg = im.rect('#DDA')
	
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
	
	button:
		ground black_bg
		hover  black_bg
		corner_sizes 0
		size   1.0
		alpha  0.3
		mouse  False
		action hide_screen('research')
	
	key 'ESCAPE' action hide_screen('research')
	
	
	$ technology_progress = sc_map.player.technology_progress
	$ bg_ysize = max(350, int(get_stage_height() * 0.7))
	$ title_text_size = get_absolute(info.text_size, get_stage_height()) * 1.5
	
	image research_bg:
		xsize 600
		ysize bg_ysize
		align 0.5
		
		text _('Research'):
			xalign 0.5
			color '#FFF'
			outlinecolor 0
			text_size title_text_size
			font 'Arial'
		
		null:
			xalign 0.5
			ypos title_text_size
			ysize bg_ysize - title_text_size
			
			hbox:
				align 0.5
				spacing 30
				
				vbox:
					spacing 10
					
					text _('Name'):
						xalign 0.5
						text_size info.text_size
						color '#00F'
					
					for building in technology_names:
						python:
							name = _(building)
							if building in technology_extra:
								name += ' (' + _(technology_extra[building]) + ')'
						
						text name:
							text_size info.text_size
							color 0
				
				vbox:
					spacing 10
					
					text _('Progress'):
						xalign 0.5
						text_size info.text_size
						color '#00F'
					
					for building in technology_names:
						text ('%s/4' % technology_progress[building]):
							xalign 0.5
							text_size info.text_size
							color '#0F0'
							outlinecolor 0
							ysize info.text_size
				
				vbox:
					spacing 10
					
					text _('Cost'):
						xalign 0.5
						text_size info.text_size
						color '#00F'
					
					for building in technology_names:
						if technology_progress[building] < 4:
							$ cost = technology_costs[technology_progress[building] + 1]
							text str(cost):
								xalign 0.5
								text_size info.text_size
								color '#FF0' if cost <= sc_map.player.science else '#F00'
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
						color '#00F'
					
					for building in technology_names:
						if technology_progress[building] < 4 and sc_map.player.science >= technology_costs[technology_progress[building] + 1]:
							textbutton '+':
								style 'btn'
								xalign 0.5
								size info.text_size
								text_size info.text_size - 2
								font 'Calibri'
								color '#0F0'
								outlinecolor 0
								action sc_map.player.explore(building)
						else:
							null:
								size info.text_size
