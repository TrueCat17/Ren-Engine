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
		if get_game_time() - slots.last_update > 0.5:
			slots.update()
	
	def slots__update_sizes():
		sw, sh = get_stage_size()
		k = get_from_hard_config('window_w_div_h', float)
		
		if gui.slot_width is None and gui.slot_height is None:
			xsize = sw / (gui.file_slot_cols + 2)
			ysize = sh / (gui.file_slot_rows + 2)
			
			if xsize < ysize * k:
				ysize = xsize / k
			else:
				xsize = ysize * k
		else:
			if gui.slot_width is None:
				ysize = gui.get_int('slot_height')
				xsize = ysize * k
			else:
				xsize = gui.get_int('slot_width')
				ysize = xsize / k
		slots.xsize, slots.ysize = int(xsize), int(ysize)
	
	def slots__update():
		slots.last_update = get_game_time()
		
		slots.update_sizes()
		
		exists = dont_save.slots_exists = {}
		cache = dont_save.slots_cache = {}
		for page in slots.pages:
			page_path = slots.directory + page + '/'
			exists_on_page = exists[page] = []
			
			for slot in slots.slots:
				slot_path = page_path + slot + '/'
				
				py_globals_path = slot_path + 'py_globals'
				if not os.path.exists(py_globals_path): continue
				
				exists_on_page.append(slot)
				
				mtime_utc = os.path.getmtime(py_globals_path)
				tm = time.localtime(mtime_utc)
				
				mtime_formatted = time.strftime('%d.%m.%y-%H:%M', tm)
				cache[(page, slot, 'mtime_utc')] = mtime_utc
				cache[(page, slot, 'mtime_formatted')] = mtime_formatted
				
				screenshot = slot_path + 'screenshot.png'
				if os.path.exists(screenshot):
					screenshot += '?' + str(os.path.getmtime(screenshot))
				else:
					screenshot = None
				cache[(page, slot, 'screenshot')] = screenshot
	
	def slots__update_on_show(screen_name):
		if screen_name in ('save', 'load'):
			slots.update()
	signals.add('show_screen', slots__update_on_show)
	
	def slots__update_size_on_resized_stage():
		if has_screen('load') or has_screen('save'):
			slots.update_sizes()
	signals.add('resized_stage', slots__update_size_on_resized_stage)
	
	
	def slots__get_page():
		return persistent.slot_page
	def slots__set_page(page):
		persistent.slot_page = page
	
	def slots__can_load(slot = None, page = None):
		page = page or persistent.slot_page
		if page not in dont_save.slots_exists:
			out_msg('slots.can_load', 'For a non standard page <%s> use function slots.list_slots(page)' % (page, ))
			return False
		slot = slot or persistent.slot_selected
		return slot in dont_save.slots_exists[page]
	def slots__list_slots(page = None):
		page = page or persistent.slot_page
		if page in slots.pages:
			return dont_save.slots_exists[page]
		res = []
		page_path = slots.directory + page + '/'
		for slot in os.listdir(page_path):
			if os.path.exists(page_path + slot + '/py_globals'):
				res.append(slot)
		return res
	
	def slots__mtime(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		return dont_save.slots_cache.get((page, slot, 'mtime_utc'), None)
	def slots__mtime_formatted(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		return dont_save.slots_cache.get((page, slot, 'mtime_formatted'), None)
	def slots__screenshot(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		return dont_save.slots_cache.get((page, slot, 'screenshot'), None)
	
	def slots__image(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		
		xsize, ysize = slots.xsize, slots.ysize
		
		selected = persistent.slot_selected == slot
		over = gui.bg('slot_selected' if selected else 'slot_hover')
		over = im.scale_without_borders(over, xsize, ysize, gui.slot_corner_sizes, need_scale = True)
		
		screenshot = slots.screenshot(slot, page)
		if not screenshot:
			return over
		
		slot_image_processing = gui.slot_image_processing
		
		cache = slots__image.__dict__
		key = screenshot, over, xsize, ysize, slot_image_processing
		if key in cache:
			return cache[key]
		
		screenshot = im.scale(screenshot, xsize, ysize)
		if slot_image_processing:
			screenshot = slot_image_processing(screenshot)
		
		cache[key] = im.composite((xsize, ysize), (0, 0), screenshot, (0, 0), over)
		return cache[key]
	
	
	
	def slots__load(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		_load(str(page), str(slot))
	
	def slots__save(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		
		screenshot = slots.directory + page + '/' + slot + '/screenshot.png'
		screenshot_mtime = os.path.getmtime(screenshot) if os.path.exists(screenshot) else 0
		if time.time() - screenshot_mtime < 1:
			return
		
		if page in ('auto', 'quick') and not has_screen('save'):
			slots.sort_by_time(page)
		
		global need_save, save_page, save_slot, screenshot_width, screenshot_height
		need_save = True
		save_page, save_slot = page, slot
		screenshot_width = config.thumbnail_width
		screenshot_height = int(screenshot_width / get_from_hard_config('window_w_div_h', float))
	
	def slots__delete(slot = None, page = None):
		page = page or persistent.slot_page
		slot = slot or persistent.slot_selected
		shutil.rmtree(slots.directory + page + '/' + slot)
		slots.update()
	
	
	def slots__sort_by_time(page):
		page_path = slots.directory + page + '/'
		if not os.path.exists(page_path):
			return
		
		def is_digit(name):
			return name.isdigit() and int(name) >= 0 and int(name) < gui.file_slot_cols * gui.file_slot_rows
		
		saves = []
		for slot in os.listdir(page_path):
			slot_path = page_path + slot + '/'
			
			if os.path.exists(slot_path + 'screenshot.png') and is_digit(slot):
				saves.append(slot)
			else:
				shutil.rmtree(slot_path)
		
		def get_mtime(slot):
			dir_mtime        = os.path.getmtime(page_path + slot)
			screenshot_mtime = os.path.getmtime(page_path + slot + '/screenshot.png')
			return max(dir_mtime, screenshot_mtime)
		
		saves.sort(key = get_mtime, reverse = True)
		
		while saves:
			need_slot = str(len(saves) + 1)
			slot = saves.pop(-1)
			
			if need_slot == str(gui.file_slot_cols * gui.file_slot_rows + 1):
				shutil.rmtree(page_path + slot)
				continue
			
			if slot == need_slot:
				continue
			
			if need_slot in saves:
				tmp_slot = 'tmp_' + need_slot
				index = saves.index(need_slot)
				saves[index] = tmp_slot
				os.rename(page_path + need_slot, page_path + tmp_slot)
			
			os.rename(page_path + slot, page_path + need_slot)
		
		slots.update()
	
	
	def slots__get(name):
		slot = SimpleObject()
		
		slot.xsize = slots.xsize
		slot.ysize = slots.ysize
		slot.ground = slots.image(name)
		slot.mouse  = slots.can_load(name)
		slot.action = [SetVariable('persistent.slot_selected', name), slots.update]
		
		slot.desc = slots.mtime_formatted(name) or ''
		
		return slot
	
	def slots__get_btns(screen_name):
		btns = []
		
		if screen_name == 'load':
			tmp_style_enabled = style.enabled_load_button or style.menu_button
			if renpy.can_load():
				btns.append(['Load', tmp_style_enabled, 1, True, renpy.load])
			else:
				tmp_style_disabled = style.disabled_load_button or tmp_style_enabled
				alpha = 0.7 if tmp_style_disabled is tmp_style_enabled else 1
				btns.append(['Load', tmp_style_disabled, alpha, False, None])
		else:
			tmp_style = style.save_button or style.menu_button
			btns.append(['Save', tmp_style, 1, True, renpy.save])
		
		tmp_style_enabled = style.enabled_delete_button or style.menu_button
		if renpy.can_load():
			btns.append(['Delete', tmp_style_enabled, 1, True, renpy.unlink_save])
		else:
			tmp_style_disabled = style.disabled_delete_button or tmp_style_enabled
			alpha = 0.7 if tmp_style_disabled is tmp_style_enabled else 1
			btns.append(['Delete', tmp_style_disabled, alpha, False, None])
		
		return btns
	
	
	build_object('slots')
	slots.directory = '../var/saves/'

init -900 python:
	slots.pages = [str(i + 1) for i in range(gui.slot_pages)] + ['auto', 'quick']
	slots.slots = [str(i + 1) for i in range(gui.file_slot_cols * gui.file_slot_rows)]
	slots.update()
	
	persistent.setdefault('slot_page', '1')
	persistent.setdefault('slot_selected', '1')
