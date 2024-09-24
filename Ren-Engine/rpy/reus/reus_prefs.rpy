init python:
	def reus_update_prefs(screen_name = None):
		lang_changing = screen_name is None
		added_to_pref_tabs = 'Updates' in preferences.tabs
		
		if lang_changing:
			if not added_to_pref_tabs:
				return
		else:
			if screen_name != 'preferences':
				return
		
		cur_lang = config.language
		if dont_save_reus.get('prev_lang') == cur_lang:
			return
		
		dont_save_reus.prev_lang = cur_lang
		path_names = dont_save_reus.path_names = {}
		
		reus.scan_links()
		paths = dont_save_reus.paths = list(persistent.reus_storage.keys())
		if not paths:
			if added_to_pref_tabs:
				preferences.tabs.remove('Updates')
				del preferences.content['Updates']
				if preferences.tab == 'Updates':
					preferences.tab = preferences.tabs[0]
			return
		
		for path in paths:
			name = get_directory_name(path)
			
			if not name:
				name = '-'
			if len(name) > 22:
				name = name[:20] + '..'
			
			name = name.replace('[', '[[') # escape tag interpolation
			path_names[path] = name
		
		if not added_to_pref_tabs:
			preferences.tabs.append('Updates')
			preferences.content['Updates'] = [reus_get_prefs]
		
		dont_save_reus.prefs_page_index = 0
		dont_save_reus.prefs_page_max = math.ceil(len(paths) / gui.prefs_update_buttons_in_page) - 1
	
	signals.add('show_screen', reus_update_prefs)
	signals.add('language', reus_update_prefs)
	
	
	def reus_get_prefs():
		if 'paths' not in dont_save_reus:
			reus_update_prefs()
			if not dont_save_reus.paths: # loaded after removing all <upd_link.txt> files
				return []
		
		res = []
		
		res.append([
			['str', '{u}' + _('No auto-updates, no reminders.\nThe update will return the edited files.'), 1, -1, 'center']
		])
		
		prefs_page_max = dont_save_reus.get('prefs_page_max', -1)
		if prefs_page_max > 0:
			var = 'dont_save_reus.prefs_page_index'
			prev_action = '%s = max(%s - 1, 0)' % (var, var)
			next_action = '%s = min(%s + 1, dont_save_reus.prefs_page_max)' % (var, var)
			
			prev = ['btn', gui.back_button_text, None, prev_action, (1, 1)]
			page = ['str', str(dont_save_reus.prefs_page_index + 1)]
			next = ['btn', gui.next_button_text, None, next_action, (1, 1)]
			
			res.append([prev, page, next])
		
		storage = persistent.reus_storage
		
		count = gui.prefs_update_buttons_in_page
		index = dont_save_reus.prefs_page_index
		paths = dont_save_reus.paths[index * count : (index + 1) * count]
		
		for path in paths:
			info = storage[path]
			if info.has_changes:
				size = info.size_to_load / (1 << 20)
				size = '%.1f' % size + ' ' + _('MB')
			else:
				size = '{alpha=0}' + '000.0 ' + _('MB')
			
			if len(size) < 8:
				size = '{alpha=0}' + '0' * (8 - len(size)) + '{/alpha}' + size
			
			name = dont_save_reus.path_names.get(path, '-')
			
			res.append([
				['str', name, 1, 0.3, 'center'],
				['btn', _('Check'), None, Function(reus.check, path), (4, 1)],
				['str', size],
				['btn', _('Update'), None, Function(reus.check_and_load, path), (4, 1)],
			])
		
		btn_style = style.prefs_menu_button or style.menu_button
		btn_size = btn_style.get_current('ysize')
		text_size = style.menu_text.get_current('text_size')
		k_text_size = max(btn_size / text_size, 1)
		
		empty_line = ['str', ' ', k_text_size]
		for i in range(count - len(paths)):
			res.append([empty_line])
		
		progress = reus.get_loading_progress()
		if progress:
			loaded, size_to_load = progress
			res.append([
				['str', _('Loaded') + ':'],
				['str', '%s/%s %s' % (loaded, size_to_load, _('MB')), 1, 0.15, 'right'],
				['btn', _('Cancel'), None, reus.stop_loading],
			])
		else:
			res.append([empty_line])
		
		if len(dont_save_reus.paths) > 1:
			res.append([
				['btn', _('Check all'), None, reus.check, (6, 1)]
			])
		
		return res
