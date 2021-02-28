# sl = save_load
init -2 python:
	save_dir = '../var/saves/'
	
	sl_cur_table = '0'
	sl_cur_save  = '0'
	
	sl_count_tables = 10
	sl_saves_in_table = 12
	
	
	sl_last_autosave = get_game_time()
	def sl_check_autosave():
		global sl_last_autosave
		if get_can_autosave() and not has_screen('pause') and config.autosave > 0:
			if get_game_time() - max(sl_last_autosave, get_mod_start_time()) > config.autosave:
				sl_last_autosave = get_game_time()
				sl_save('auto', '0')
	
	
	sl_inited = False
	def init_sl():
		global sl_inited
		global sl_btn_hover, sl_btn_selected
		
		sl_inited = True
		
		sl_btn_hover    = get_back_with_color('images/gui/save_load/hover.png')
		sl_btn_selected = get_back_with_color('images/gui/save_load/selected.png')
	
	
	def sl_get_dirs(path):
		if not os.path.exists(path):
			return []
		dirs_files = os.listdir(path)
		dirs = [name for name in dirs_files if os.path.isdir(os.path.join(path, name))]
		return dirs
	
	def sl_get_tables():
		res = [str(i) for i in xrange(sl_count_tables)] + ['auto', 'quick']
		dirs = sl_get_dirs(save_dir)
		for i in dirs:
			if i not in res:
				res.append(i)
		return res
	def sl_get_table_saves(table):
		res = [str(i) for i in xrange(sl_saves_in_table)]
		dirs = sl_get_dirs(os.path.join(save_dir, table))
		for i in dirs:
			if i not in res:
				res.append(i)
		return res
	
	def sl_get_screenshot(table, save, save_exists):
		selected = sl_cur_save == save
		over = sl_btn_selected if selected else sl_btn_hover
		
		if save_exists:
			screenshot = os.path.join(save_dir, table, save, 'screenshot.png')
			screenshot += "?" + str(os.path.getmtime(screenshot))
			w, h = get_image_size(screenshot)
			return im.Composite((w, h), (0, 0), screenshot, (0, 0), im.Scale(over, w, h))
		
		return over
	
	sl_datetimes_cache = {}
	def sl_get_datetime(table, save):
		path = os.path.join(save_dir, table, save, 'py_globals')
		utc = os.path.getmtime(path)
		
		if sl_datetimes_cache.has_key(utc):
			res = sl_datetimes_cache[utc]
		else:
			dt = time.gmtime(utc)
			
			data = (str(i) for i in (dt.tm_mday, dt.tm_mon, dt.tm_year % 100, dt.tm_hour, dt.tm_min))
			data = tuple(i if len(i) == 2 else '0' + i for i in data)
			
			res = '%s.%s.%s, %s:%s' % data
			sl_datetimes_cache[utc] = res
		return res
	
	def sl_save(table, save):
		if table in ('auto', 'quick'):
			sl_sort_time(table)
		
		global need_save, save_table, save_name, screenshot_width, screenshot_height
		need_save = True
		save_table, save_name = table, save
		screenshot_width = config.save_screenshot_width
		screenshot_height = int(screenshot_width / get_from_hard_config("window_w_div_h", float))
		
	
	def sl_delete_save(table, save):
		shutil.rmtree(os.path.join(save_dir, table, save))
		sl_update_table_saves()
	
	def sl_update_table_saves():
		global sl_tables, sl_table_saves, sl_table_saves_exists
		
		sl_tables = sl_get_tables()
		sl_table_saves = sl_get_table_saves(sl_cur_table)
		
		sl_table_saves_exists = {}
		for save_name in sl_table_saves:
			sl_table_saves_exists[save_name] = os.path.exists(os.path.join(save_dir, sl_cur_table, save_name, 'screenshot.png'))
	sl_update_table_saves()
	
	
	def sl_sort_time(table):
		table_path = os.path.join(save_dir, table)
		if not os.path.exists(table_path):
			return
		
		def is_digit(name):
			return name.isdigit() and int(name) >= 0 and int(name) < sl_saves_in_table
		
		dir_contains = os.listdir(table_path)
		saves = []
		for name in dir_contains:
			path = os.path.join(table_path, name)
			
			if os.path.isdir(path) and os.path.exists(os.path.join(path, 'screenshot.png')) and is_digit(name):
				saves.append(name)
			else:
				shutil.rmtree(path)
		
		def get_mtime(name):
			dir_mtime = os.path.getmtime(os.path.join(table_path, name))
			screenshot_mtime = os.path.getmtime(os.path.join(table_path, name, 'screenshot.png'))
			return max(dir_mtime, screenshot_mtime)
		
		saves.sort(key = get_mtime, reverse = True)
		
		while saves:
			name = saves[-1]
			need_name = str(len(saves))
			saves = saves[0:-1]
			
			if need_name == str(sl_saves_in_table):
				shutil.rmtree(os.path.join(table_path, name))
				continue
			
			if name == need_name:
				continue
			
			if need_name in saves:
				tmp_name = 'tmp_' + need_name
				index = saves.index(need_name)
				saves[index] = tmp_name
				os.rename(os.path.join(table_path, need_name), os.path.join(table_path, tmp_name))
			
			os.rename(os.path.join(table_path, name), os.path.join(table_path, need_name))
		
		sl_update_table_saves()

