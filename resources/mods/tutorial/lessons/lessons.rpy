init -1 python:
	
	def lessons__add_current(tags_str):
		file = get_filename(1)
		filename = os.path.basename(file)
		
		i = filename.index('_')
		num = filename[:i]
		num_int = int(num)
		name = filename[i+1:].replace('.rpy', '')
		
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
		lesson = category_lessons[num]
		name = lesson[0]
		
		lessons.cur_label = name
		renpy.call('start_lesson')
	
	build_object('lessons')
	lessons.by_category = {}
	lessons.cur_label = None


label start_lesson:
	$ db.skip_tab = False
	window hide
	$ set_timeout(show_bg_entry, black_cover.appearance_time)
	$ tutorial_menu.hide()
	pause black_cover.appearance_time
	
	$ pause_screen.items.insert(0, ['Tutorial Menu', tutorial_menu.restart])
	
	call expression lessons.cur_label
	$ db.skip_tab = False
	
	$ pause_screen.items.pop(0)
	
	$ tutorial_menu.start()
	pause black_cover.appearance_time
	window hide
	scene
	$ lessons.cur_label = None
