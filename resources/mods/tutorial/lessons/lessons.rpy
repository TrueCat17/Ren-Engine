init -1 python:
	
	def lessons__add_current(tags_str):
		file = get_filename(1)
		file_name = os.path.basename(file)
		
		i = file_name.index('_')
		num = file_name[:i]
		num_int = int(num)
		name = file_name[i+1:].replace('.rpy', '')
		
		pretty_name = name.replace('_', ' ').capitalize()
		
		dirname = os.path.dirname(file)
		category = os.path.basename(dirname)
		
		i = dirname.rfind('lessons')
		image = dirname[:i] + 'lesson_images' + dirname[i + len('lessons'):] + '/%s.webp' % num
		if not os.path.exists(image):
			image = im.rect('#888', 448, 256)
		
		tags = tags_str.split(', ') if tags_str else []
		tags1, tags2 = [], []
		for i, tag in enumerate(tags):
			(tags2 if i % 2 else tags1).append(tag)
		
		category_lessons = lessons.by_category.setdefault(category, {})
		category_lessons[num_int] = (name, pretty_name, image, tags1, tags2)
	
	def lessons__start(category, num):
		category_lessons = lessons.by_category[category.lower()]
		name, pretty_name, image, tags1, tags2 = category_lessons[num]
		
		lessons.cur_label = name + '_' + config.language
		if not renpy.has_label(lessons.cur_label):
			lessons.cur_label = name
		
		renpy.call('start_lesson')
	
	build_object('lessons')
	lessons.by_category = {}
	lessons.cur_label = None


label start_lesson:
	window hide
	$ set_timeout(show_bg_entry, black_cover.appearance_time)
	$ tutorial_menu.hide()
	pause black_cover.appearance_time
	
	$ pause_screen.items.insert(0, ['Tutorial Menu', tutorial_menu.restart])
	
	$ db.skip_tab = False
	call expression lessons.cur_label
	$ db.skip_tab = False
	
	$ pause_screen.items.pop(0)
	
	$ tutorial_menu.start()
	pause black_cover.appearance_time
	window hide
	scene
	$ lessons.cur_label = None
