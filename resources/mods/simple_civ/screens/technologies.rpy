init python:
	sc_technologies = SimpleObject()
	sc_technologies.bg = im.rect('#DDA')
	
	sc_technologies.names = []
	sc_technologies.extra = {}
	
	for resource in sc_info.simple_resources:
		sc_technologies.names.append(resource)
		
		array = sc_buildings.on_resource[resource]
		building = array[0]
		sc_technologies.extra[resource] = building
	
	sc_technologies.names.extend(sc_buildings.common)
	
	sc_technologies.price = [None, 100, 300, 900, 2700]
	
	hotkeys.disable_key_on_screens['ESCAPE'].append('sc_technologies')


screen sc_technologies:
	modal True
	zorder 100
	
	button:
		ground sc_black_bg
		hover  sc_black_bg
		corner_sizes 0
		size   1.0
		alpha  0.3
		mouse  False
		action hide_screen('sc_technologies')
	
	key 'ESCAPE' action hide_screen('sc_technologies')
	
	python:
		screen_tmp = SimpleObject()
		screen_tmp.is_bot = sc_map.player.bot is not None
		screen_tmp.technological_progress = sc_map.player.technological_progress
		screen_tmp.bg_ysize = in_bounds(get_stage_height() * 3 // 4, 330, 430)
		screen_tmp.text_size = min(get_stage_height() // 20, 24)
		screen_tmp.title_text_size = screen_tmp.text_size * 3 // 2
		screen_tmp.indent = 10
	
	image sc_technologies.bg:
		xsize 600
		ysize screen_tmp.bg_ysize
		align 0.5
		
		text _('Technologies'):
			xalign 0.5
			ypos screen_tmp.indent
			font 'Arial'
			text_size screen_tmp.title_text_size
			color '#FFF'
			outlinecolor 0
		
		null:
			xalign 0.5
			ypos screen_tmp.title_text_size + screen_tmp.indent * 2
			ysize screen_tmp.bg_ysize - screen_tmp.title_text_size - screen_tmp.indent * 3
			
			hbox:
				align 0.5
				spacing 30
				
				vbox:
					spacing screen_tmp.indent
					
					text _('Name'):
						xalign 0.5
						text_size screen_tmp.text_size
						color '#00F'
					
					for building in sc_technologies.names:
						python:
							name = _(building)
							if building in sc_technologies.extra:
								name += ' (%s)' % _(sc_technologies.extra[building])
						
						text name:
							text_size screen_tmp.text_size
							color 0
				
				vbox:
					spacing screen_tmp.indent
					
					text _('Progress'):
						xalign 0.5
						text_size screen_tmp.text_size
						color '#00F'
					
					for building in sc_technologies.names:
						text ('%s/4' % screen_tmp.technological_progress[building]):
							xalign 0.5
							text_size screen_tmp.text_size
							color '#0F0'
							outlinecolor 0
							ysize screen_tmp.text_size
				
				vbox:
					spacing screen_tmp.indent
					
					text _('Price'):
						xalign 0.5
						text_size screen_tmp.text_size
						color '#00F'
					
					for building in sc_technologies.names:
						$ level = screen_tmp.technological_progress[building]
						if level < 4:
							$ price = sc_technologies.price[level + 1]
							text str(price):
								xalign 0.5
								text_size screen_tmp.text_size
								color '#FF0' if price <= sc_map.player.science else '#F00'
								outlinecolor 0
								ysize screen_tmp.text_size
						else:
							null:
								size screen_tmp.text_size
				
				vbox:
					spacing screen_tmp.indent
					
					text _('Explore'):
						xalign 0.5
						text_size screen_tmp.text_size
						color '#00F'
					
					for building in sc_technologies.names:
						$ level = screen_tmp.technological_progress[building]
						if level < 4 and sc_map.player.science >= sc_technologies.price[level + 1] and not screen_tmp.is_bot:
							textbutton '+':
								style 'sc_tech_explore_btn'
								size screen_tmp.text_size
								text_size screen_tmp.text_size - 2
								action sc_map.player.explore(building)
						else:
							null:
								size screen_tmp.text_size
