init:
	style tutorial_btn is textbutton:
		font 'Fregat_Bold'
		text_size 0.03
		text_size_min 12
		color '#FFF'
		hover_color 0
		xsize 0.18
		xsize_min 125
		ysize 0.042
		ysize_min 20
	
	style tutorial_lesson_btn is button:
		ground im.rect('#00000002')
		hover  im.matrix_color('images/gui/save_load/hover.webp', im.matrix.brightness(+1))
		xsize 0.163
		ysize 0.34
	
	
	$ w, h = 448, 256
	$ k = get_from_hard_config('window_w_div_h', float)
	
	style tutorial_lesson_image is image:
		xalign 0.5
		xsize 0.15
		ysize style.tutorial_lesson_image.xsize * k / (w / h)
	
	style tutorial_lesson_name is text:
		font 'Fregat_Bold'
		text_size 0.03
		xalign 0.5
	
	style tutorial_tag_container is null:
		xalign 0.5
		xsize style.tutorial_lesson_image.xsize
	
	style tutorial_tag_text is text:
		font 'Fregat'
		text_size 0.03
		text_size_min 12
		color '#EEE'
	
	style tutorial_page_btn is textbutton:
		xsize 0.025
		ysize 0.025 * k
		font 'Fregat_Bold'
		text_size 0.03
		color '#FFF'
		hover_color 0


init 100 python:
	def tutorial_menu__start(from_middle = False):
		black_cover.start(from_middle = from_middle)
		if from_middle:
			show_screen('tutorial_menu')
		else:
			set_timeout(ShowScreen('tutorial_menu'), black_cover.appearance_time)
	
	def tutorial_menu__hide():
		# do nothing when lesson starts from init block on testing
		if has_screen('tutorial_menu'):
			black_cover.start()
			set_timeout(HideScreen('tutorial_menu'), black_cover.appearance_time)
	
	def tutorial_menu__restart():
		pause_screen.items.pop(0)
		hide_screen('pause')
		
		black_cover.start()
		set_timeout(Jump('start'), black_cover.appearance_time)
	
	
	def tutorial_menu__set_category(category):
		tutorial_menu.category_lessons = lessons.by_category.get(category.lower()) or {}
		tutorial_menu.count_pages = math.ceil(len(tutorial_menu.category_lessons) / 5)
		tutorial_menu.cur_page = persistent.tutorial_page = min(
			persistent.tutorial_page if persistent.tutorial_category == category else 0,
			max(tutorial_menu.count_pages - 1, 0)
		)
		tutorial_menu.cur_category = persistent.tutorial_category = category
		tutorial_menu.hovered_lesson = None
	
	
	def tutorial_menu__get_lesson_image(image):
		cache = tutorial_menu__get_lesson_image.__dict__
		
		sw = get_stage_width()
		if sw >= 1200:
			round_size = 16
		else:
			if sw >= 960:
				k = (sw - 960) / (1200 - 960)
				min_round_size = 16
				max_round_size = 32
			else:
				k = (sw - 640) / (960 - 640)
				min_round_size = 32
				max_round_size = 64
			
			round_size = round(min_round_size + (max_round_size - min_round_size) * (1 - k))
		
		key = (image, round_size)
		res = cache.get(key)
		if res:
			return res
		
		w, h = get_image_size(image)
		
		lesson_mask = im.round_rect('#000', w, h, round_size)
		cache[key] = im.mask(image, lesson_mask, 256, 'a', alpha_image = 2)
		return cache[key]
	
	tutorial_menu = SimpleObject()
	build_object('tutorial_menu')
	
	tutorial_menu.appearance_time = 0.5
	tutorial_menu.hovered_lesson = None
	
	tutorial_menu.categories = ('Base', 'More complex things', 'Extra', 'RPG', 'Configuring')
	
	tutorial_menu.set_category(persistent.tutorial_category or tutorial_menu.categories[0])
	#lessons.start('Base', 4)


screen tutorial_menu:
	button:
		ground 'images/bg/monitor.webp'
		hover  'images/bg/monitor.webp'
		size 1.0
		corner_sizes 0
		mouse False
		alternate pause_screen.show
	
	$ spacing = int(get_stage_width() * 0.01)
	vbox:
		pos (0.091, 0.174)
		xsize 0.8
		spacing spacing
		
		textbutton _('Exit'):
			style 'tutorial_btn'
			xalign 0.5
			xsize 0.1
			action start_mod('main_menu')
		
		vbox:
			align (0.5, 0.1)
			spacing spacing
			
			for i in (0, 1):
				hbox:
					align 0.5
					spacing spacing
					
					for category in tutorial_menu.categories[i * 3 : (i + 1) * 3]:
						textbutton _(category):
							style 'tutorial_btn'
							selected category == tutorial_menu.cur_category
							action tutorial_menu.set_category(category)
		
		hbox:
			xalign 0.5
			
			$ ypos = int((style.tutorial_lesson_btn.xsize - style.tutorial_lesson_image.xsize) * get_stage_width() / 2)
			for i in range(tutorial_menu.cur_page * 5, (tutorial_menu.cur_page + 1) * 5):
				null:
					style 'tutorial_lesson_btn'
					
					if i not in tutorial_menu.category_lessons:
						continue
					
					$ name, pretty_name, image, tags1, tags2 = tutorial_menu.category_lessons[i]
					button:
						style 'tutorial_lesson_btn'
						alpha 0 if lessons.cur_label else 1
						hovered   'tutorial_menu.hovered_lesson = name'
						unhovered 'if tutorial_menu.hovered_lesson == name: tutorial_menu.hovered_lesson = None'
						action lessons.start(tutorial_menu.cur_category, i)
					
					vbox:
						skip_mouse True
						spacing 0.01
						xalign 0.5
						ypos ypos
						
						image tutorial_menu.get_lesson_image(image):
							style 'tutorial_lesson_image'
						
						text _(pretty_name):
							style 'tutorial_lesson_name'
							underline tutorial_menu.hovered_lesson == name
						
						null:
							style 'tutorial_tag_container'
							alpha 1 if tutorial_menu.hovered_lesson == name else 0
							
							for xpos in (0.0, 0.6):
								vbox:
									xpos xpos
									
									for tag in (tags1 if xpos < 0.5 else tags2):
										hbox:
											spacing 0.001
											
											text '•':
												style 'tutorial_tag_text'
												color '#FFF'
											
											text tag:
												style 'tutorial_tag_text'
	
	null:
		#$ tutorial_menu.count_pages = 3
		pos (0.091, 0.76)
		xsize 0.8
		alpha 1 if tutorial_menu.count_pages > 1 else 0
		
		hbox:
			xalign 0.5
			spacing 0.01
			
			for page in range(tutorial_menu.count_pages):
				textbutton str(page + 1):
					style 'tutorial_page_btn'
					selected tutorial_menu.cur_page == page
					action 'tutorial_menu.cur_page = persistent.tutorial_page = page'
