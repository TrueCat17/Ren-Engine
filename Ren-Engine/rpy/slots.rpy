init -10000 python:
	def slots__check_autosave():
		if not slots.last_autosave:
			slots.last_autosave = get_game_time()
		
		if not config.has_autosave: return
		if has_screen('pause'): return
		if config.autosave <= 0: return
		if get_game_time() - slots.last_autosave < config.autosave: return
		
		slots.last_autosave = get_game_time()
		slots.save('1', 'auto')
	
	signals.add('exit_frame', slots__check_autosave)
	
	
	def slots__check_update():
		if not slots.last_update:
			slots.last_update = get_game_time()
		dtime = get_game_time() - slots.last_update
		if dtime > 0.5:
			slots.update()
	def slots__update():
		slots.last_update = get_game_time()
		
		sw, sh = get_stage_size()
		k = get_from_hard_config("window_w_div_h", float)
		
		if gui.slot_width is None and gui.slot_height is None:
			xsize = int(sw / (gui.file_slot_cols + 2))
			ysize = int(sh / (gui.file_slot_rows + 2))
			
			if xsize < ysize * k:
				ysize = int(xsize / k)
			else:
				xsize = int(ysize * k)
		else:
			if gui.slot_width is None:
				ysize = gui.get_int('slot_height')
				xsize = int(ysize * k)
			else:
				xsize = gui.get_int('slot_width')
				ysize = int(xsize / k)
		slots.xsize, slots.ysize = xsize, ysize
		
		slots.exists = {}
		slots.cache = {}
		for page in slots.pages:
			exists_on_page = slots.exists[page] = []
			
			for slot in slots.slots:
				path = os.path.join(slots.directory, page, slot, 'py_globals')
				if not os.path.exists(path): continue
				
				exists_on_page.append(slot)
				
				mtime_utc = os.path.getmtime(path)
				dt = time.localtime(mtime_utc)
				data = (str(i) for i in (dt.tm_mday, dt.tm_mon, dt.tm_year % 100, dt.tm_hour, dt.tm_min))
				data = tuple(i if len(i) == 2 else '0' + i for i in data)
				
				mtime_formatted = '%s.%s.%s, %s:%s' % data
				slots.cache[(page, slot, 'mtime_utc')] = mtime_utc
				slots.cache[(page, slot, 'mtime_formatted')] = mtime_formatted
				
				screenshot = os.path.join(slots.directory, page, slot, 'screenshot.png')
				if os.path.exists(screenshot):
					screenshot += '?' + str(os.path.getmtime(screenshot))
				else:
					screenshot = None
				slots.cache[(page, slot, 'screenshot')] = screenshot
	
	def slots__update_on_show(name):
		if name in ('save', 'load'):
			slots.update()
	signals.add('show_screen', slots__update_on_show)
	signals.add('resized_stage', slots__update)
	
	
	def slots__get_page():
		return persistent.slot_page
	def slots__set_page(page):
		persistent.slot_page = page
	
	def slots__can_load(slot = None, page = None):
		page = page or persistent.slot_page
		if page not in slots.exists:
			out_msg('slots.can_load', 'For not standard page <' + str(page) + '> use function slots.list_slots(page)')
			return False
		slot = slot or persistent.slot_selected
		return slot in slots.exists[page]
	def slots__list_slots(page = None):
		page = page or persistent.slot_page
		if page in slots.pages:
			return slots.exists[page]
		res = []
		for slot in os.listdir(os.path.join(slots.directory, page)):
			if os.path.exists(os.path.join(slots.directory, page, slot, 'py_globals')):
				res.append(slot)
		return res
	
	def slots__mtime(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		return slots.cache.get((page, slot, 'mtime_utc'), None)
	def slots__mtime_formatted(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		return slots.cache.get((page, slot, 'mtime_formatted'), None)
	def slots__screenshot(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		return slots.cache.get((page, slot, 'screenshot'), None)
	
	def slots__image(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		
		selected = persistent.slot_selected == slot
		over = gui.bg('slot_selected' if selected else 'slot_hover')
		over = im.scale_without_borders(over, slots.xsize, slots.ysize, gui.slot_corner_sizes, need_scale = True)
		
		screenshot = slots.screenshot(slot, page)
		if not screenshot:
			return over
		
		screenshot = im.scale(screenshot, slots.xsize, slots.ysize)
		if gui.slot_image_processing:
			screenshot = gui.slot_image_processing(screenshot)
		
		return im.composite((slots.xsize, slots.ysize), (0, 0), screenshot, (0, 0), over)
	
	
	
	def slots__load(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		_load(str(page), str(slot))
	
	def slots__save(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		
		screenshot = os.path.join(slots.directory, page, slot, 'screenshot.png')
		screenshot_mtime = os.path.getmtime(screenshot) if os.path.exists(screenshot) else 0
		if time.time() - screenshot_mtime < 1:
			return
		
		if page in ('auto', 'quick'):
			slots.sort_by_time(page)
		
		global need_save, save_page, save_slot, screenshot_width, screenshot_height
		need_save = True
		save_page, save_slot = page, slot
		screenshot_width = config.thumbnail_width
		screenshot_height = int(screenshot_width / get_from_hard_config("window_w_div_h", float))
	
	def slots__delete(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		shutil.rmtree(os.path.join(slots.directory, page, slot))
		slots.update()
	
	
	def slots__sort_by_time(page):
		page_path = os.path.join(slots.directory, page)
		if not os.path.exists(page_path):
			return
		
		def is_digit(name):
			return name.isdigit() and int(name) >= 0 and int(name) < gui.file_slot_cols * gui.file_slot_rows
		
		saves = []
		for name in os.listdir(page_path):
			path = os.path.join(page_path, name)
			
			if os.path.isdir(path) and os.path.exists(os.path.join(path, 'screenshot.png')) and is_digit(name):
				saves.append(name)
			else:
				shutil.rmtree(path)
		
		def get_mtime(slot):
			dir_mtime = os.path.getmtime(os.path.join(page_path, slot))
			screenshot_mtime = os.path.getmtime(os.path.join(page_path, slot, 'screenshot.png'))
			return max(dir_mtime, screenshot_mtime)
		
		saves.sort(key = get_mtime, reverse = True)
		
		while saves:
			need_name = str(len(saves) + 1)
			name = saves.pop(-1)
			
			if need_name == str(gui.file_slot_cols * gui.file_slot_rows + 1):
				shutil.rmtree(os.path.join(page_path, name))
				continue
			
			if name == need_name:
				continue
			
			if need_name in saves:
				tmp_name = 'tmp_' + need_name
				index = saves.index(need_name)
				saves[index] = tmp_name
				os.rename(os.path.join(page_path, need_name), os.path.join(page_path, tmp_name))
			
			os.rename(os.path.join(page_path, name), os.path.join(page_path, need_name))
		
		slots.update()
	
	
	def slots__get(name):
		slot = Object()
		
		slot.xsize = slots.xsize
		slot.ysize = slots.ysize
		slot.ground = slots.image(name)
		slot.mouse  = slots.can_load(name)
		slot.action = [SetVariable('persistent.slot_selected', name), slots.update]
		
		slot.desc = slots.mtime_formatted(name) or ''
		
		return slot
	
	
	build_object('slots')
	slots.directory = '../var/saves/'

init -900 python:
	slots.pages = [str(i + 1) for i in range(gui.slot_pages)] + ['auto', 'quick']
	slots.slots = [str(i + 1) for i in range(gui.file_slot_cols * gui.file_slot_rows)]
	slots.update()
	
	if not persistent.slot_page:
		persistent.slot_page = '1'
	if not persistent.slot_selected:
		persistent.slot_selected = '1'
