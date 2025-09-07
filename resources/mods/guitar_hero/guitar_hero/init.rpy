init -100 python:
	
	def guitar_hero__init():
		if guitar_hero.inited: return
		guitar_hero.inited = True
		
		guitar_hero.initing = True
		
		try:
			songs = []
			for note_dir in guitar_hero.note_dirs:
				note_dir = make_sure_dir(note_dir)
				
				for i in os.listdir(note_dir):
					if os.path.isdir(note_dir + i):
						songs.append(i)
			songs.sort()
			
			guitar_hero.song_paths = {}
			for song in songs:
				paths = guitar_hero.get_paths(song)
				if paths:
					guitar_hero.song_paths[song] = paths
					guitar_hero.get_name_for_song(song) # caching
			
			guitar_hero.update()
			
			if persistent.guitar_hero_song and persistent.guitar_hero_song in guitar_hero.cur_songs:
				paths = guitar_hero.get_paths(persistent.guitar_hero_song, False)
				if paths:
					guitar_hero.set_song(persistent.guitar_hero_song)
			
			if not guitar_hero.song and guitar_hero.cur_songs:
				guitar_hero.set_song(guitar_hero.cur_songs[0])
		
		finally:
			guitar_hero.initing = False
	
	
	def guitar_hero__show():
		guitar_hero.show_time = get_game_time()
		guitar_hero.hide_time = None
		guitar_hero.init()
		show_screen('guitar_hero')
	
	def guitar_hero__hide():
		guitar_hero.hide_time = get_game_time()
		guitar_hero.stop()
	
	
	def guitar_hero__get_audio_path(song):
		for note_dir in guitar_hero.note_dirs:
			note_dir = make_sure_dir(note_dir) + song
			
			if not os.path.isdir(note_dir):
				continue
			
			for f in os.listdir(note_dir):
				name, ext = os.path.splitext(f)
				path = note_dir + '/' + f
				if name == song and os.path.isfile(path):
					return path
		
		for fallback_dir in guitar_hero.fallback_sound_dirs:
			for path, ds, fs in os.walk(fallback_dir):
				path = make_sure_dir(path)
				
				for f in fs:
					name, ext = os.path.splitext(f)
					if name == song:
						return path + f
		
		return None
	
	def guitar_hero__get_note_dir(song):
		for note_dir in guitar_hero.note_dirs:
			note_dir = make_sure_dir(note_dir) + song
			if os.path.isdir(note_dir):
				return note_dir + '/'
		
		return None # for removed selected song (from persistent)
	
	def guitar_hero__get_paths(song, not_found_is_error = True):
		audio_path = guitar_hero.get_audio_path(song)
		note_dir = guitar_hero.get_note_dir(song)
		
		if not audio_path or not note_dir:
			if not_found_is_error:
				out_msg('guitar_hero.get_paths', 'Path for song <%s> not found', song)
			return None
		
		return audio_path, note_dir
	
	
	def guitar_hero__set_seed(seed):
		guitar_hero.start_seed = guitar_hero.seed = seed
		for i in range(5):
			guitar_hero.get_next_random()
	
	def guitar_hero__get_next_random():
		s = guitar_hero.seed = (guitar_hero.seed * 1103515245 + 12345) % (1<<32)
		return (((s << 16) + (s >> 16)) % guitar_hero.string_count) + 1
	
	
	def guitar_hero__read_commands():
		guitar_hero.set_seed(0)
		guitar_hero.notes = []
		
		for infix in (str(guitar_hero.string_count), ''):
			path = guitar_hero.note_path = guitar_hero.note_dir + 'notes' + infix + '.txt'
			if os.path.exists(path):
				break
		else:
			if not guitar_hero.recording:
				out_msg('guitar_hero.read_commands', 'File with notes for song <%s> not found', guitar_hero.song)
			return
		
		with open(path, 'rb') as f:
			content = f.read().decode('utf-8')
		
		strs = content.split('\n')
		
		seed = 0
		i_add = 1
		if strs[0].startswith('seed '):
			seed = strs[0][5:].strip()
			if seed.isdigit():
				seed = int(seed)
			else:
				out_msg('guitar_hero.read_commands', 'Invalid seed <%s> in file <%s>', seed, path)
				seed = 0
			strs.pop(0)
			i_add += 1
		guitar_hero.set_seed(seed)
		
		for i, s in enumerate(strs):
			s = s.strip()
			if not s: continue
			
			try:
				start_time, string = s.split(' ')
				guitar_hero.notes.append((float(start_time), int(string)))
			except:
				out_msg('guitar_hero.read_commands', 'Invalid str %s:%s\n<%s>', path, i + i_add, s)
	
	
	def guitar_hero__play():
		if not guitar_hero.note_dirs:
			out_msg('guitar_hero.play', 'guitar_hero.note_dirs is empty')
			return
		
		if not guitar_hero.song or guitar_hero.block_playing:
			return
		
		guitar_hero.unpause()
		guitar_hero.play_timeout_id = set_timeout(guitar_hero.real_play, guitar_hero.waiting_before_start)
		guitar_hero.start_time = get_game_time()
		guitar_hero.mus_pos = None
		
		guitar_hero.score = 0
		guitar_hero.score_color = guitar_hero.score_color_usual
		guitar_hero.errors_in_row = 0
		guitar_hero.ok_in_row = 0
		guitar_hero.last_game_quality = None
	
	def guitar_hero__real_play():
		guitar_hero.play_timeout_id = None
		
		renpy.play(guitar_hero.audio_path, guitar_hero.channel)
		renpy.music.set_pos(guitar_hero.pos or 0, guitar_hero.channel)
		
		guitar_hero.read_commands()
		guitar_hero.cur_note_index = 0
		guitar_hero.cur_notes.clear()
	
	def guitar_hero__get_time():
		if guitar_hero.start_time:
			dtime = get_game_time() - guitar_hero.start_time
			if 0 < dtime < guitar_hero.waiting_before_start:
				return '-%.1f' % (guitar_hero.waiting_before_start - dtime)
			if guitar_hero.pos:
				return '%.1f' % guitar_hero.pos
		return ' '
	
	def guitar_hero__stop_by_key():
		if guitar_hero.stop_btn or (guitar_hero.record_btn and guitar_hero.recording):
			guitar_hero.stop()
	
	def guitar_hero__stop():
		guitar_hero.start_time = None
		if guitar_hero.play_timeout_id is not None:
			clear_timeout(guitar_hero.play_timeout_id)
			guitar_hero.play_timeout_id = None
		
		renpy.stop(guitar_hero.channel)
		
		guitar_hero.last_game_quality = 1.0
		
		if guitar_hero.recording:
			guitar_hero.recording = False
			guitar_hero.recording_end_time = guitar_hero.pos
			input.confirm(guitar_hero.save_recorded, _('Save') + '?')
		else:
			if guitar_hero.notes:
				count = len(guitar_hero.notes)
				
				if guitar_hero.ok_in_row_for_x2 > 0 and count > guitar_hero.ok_in_row_for_x2:
					count = guitar_hero.ok_in_row_for_x2 + 2 * (count - guitar_hero.ok_in_row_for_x2)
				
				guitar_hero.last_game_quality = guitar_hero.score / count
		
		guitar_hero.pos = None
	
	def guitar_hero__playing():
		return guitar_hero.start_time is not None
	
	
	def guitar_hero__pause():
		guitar_hero.prev_game_time = None
		guitar_hero.paused = True
		renpy.music.set_pause(True, guitar_hero.channel)
	def guitar_hero__unpause():
		guitar_hero.paused = False
		renpy.music.set_pause(False, guitar_hero.channel)
	
	def guitar_hero__on_screen_show(name):
		if name == 'pause':
			guitar_hero.pause()
	def guitar_hero__on_screen_hide(name):
		if name == 'pause':
			guitar_hero.unpause()
	
	
	def guitar_hero__record():
		if not guitar_hero.song or guitar_hero.block_playing:
			return
		
		input.ask_str(
			guitar_hero.real_record,
			'Start time of recording fragment',
			str(guitar_hero.recording_start_time or 0),
			allow = '0123456789'
		)
	
	def guitar_hero__real_record(start_time):
		start_time = int(start_time)
		song_len_sec = int(renpy.music.get_audio_len(guitar_hero.audio_path))
		if start_time > song_len_sec:
			notification.out(_('Expected time <= %s'), song_len_sec)
			return
		
		guitar_hero.recording = True
		guitar_hero.recording_start_time = guitar_hero.pos = start_time
		guitar_hero.play()
		guitar_hero.recording_commands = []
	
	def guitar_hero__save_recorded(save):
		if not save:
			return
		
		start_time = guitar_hero.recording_start_time
		end_time   = guitar_hero.recording_end_time
		
		cmds = []
		if guitar_hero.start_seed:
			cmds.append(('seed', guitar_hero.start_seed))
		
		for pos, string in guitar_hero.notes:
			if pos <= start_time:
				cmds.append((pos, string))
		
		cmds.extend(guitar_hero.recording_commands)
		
		for pos, string in guitar_hero.notes:
			if pos > end_time:
				cmds.append((pos, string))
		
		with open(guitar_hero.note_path, 'wb') as f:
			for a, b in cmds:
				if type(a) is float:
					a = '%.1f' % a
				
				t = '%s %s\n' % (a, b)
				t = t.encode('utf-8')
				f.write(t)
	
	
	def guitar_hero__record_key(key):
		if guitar_hero.pos is None or guitar_hero.pos < guitar_hero.note_moving_time or key not in '1234560':
			return
		
		string = int(key)
		
		guitar_hero.add_note(string or guitar_hero.get_next_random(), guitar_hero.pos)
		guitar_hero.recording_commands.append((guitar_hero.pos, string))
	
	
	def guitar_hero__process_key(key):
		if guitar_hero.paused:
			return
		
		string = int(key)
		if string < 1 or string > guitar_hero.string_count:
			return
		
		min_dtime = guitar_hero.note_ignore_time
		min_dtime_string = None
		
		notes = guitar_hero.cur_notes[string]
		if notes:
			start_time = notes[0]
			end_time = start_time + guitar_hero.note_moving_time
			dtime = end_time - guitar_hero.pos
			
			if dtime <= min_dtime:
				min_dtime = dtime
				min_dtime_string = string
		
		if min_dtime_string is None:
			for tmp_string, notes in guitar_hero.cur_notes.items():
				if not notes: continue
				
				start_time = notes[0]
				end_time = start_time + guitar_hero.note_moving_time
				dtime = end_time - guitar_hero.pos
				
				if dtime < min_dtime:
					min_dtime = dtime
					min_dtime_string = tmp_string
		
		if min_dtime_string:
			ok = abs(min_dtime) < guitar_hero.error_time and min_dtime_string == string
			guitar_hero.add_score(1 if ok else -1)
			guitar_hero.cur_notes[min_dtime_string].pop(0)
	
	
	def guitar_hero__add_difficulty(v):
		guitar_hero.set_difficulty(guitar_hero.difficulty + v)
	
	def guitar_hero__set_difficulty(v):
		if not guitar_hero.block_difficulty:
			persistent.guitar_hero_difficulty = guitar_hero.difficulty = in_bounds(v, 0, len(guitar_hero.difficulty_names) - 1)
			guitar_hero.update_difficulty()
	
	def guitar_hero__update_difficulty():
		difficulty = guitar_hero.difficulty
		guitar_hero.string_count = guitar_hero.string_counts[difficulty]
		guitar_hero.error_time   = guitar_hero.error_times[difficulty]
	
	
	def guitar_hero__add_note(string, start_time):
		string = (string - 1) % guitar_hero.string_count + 1
		
		start_value = string
		while start_time in guitar_hero.cur_notes[string]:
			string += 1
			if string == guitar_hero.string_count + 1:
				string = 1
			
			if string == start_value:
				return # no free strings
		
		guitar_hero.cur_notes[string].append(start_time)
	
	def guitar_hero__get_notes(string):
		res = []
		if not guitar_hero.pos:
			return res
		
		for start_time in guitar_hero.cur_notes[string]:
			dtime = guitar_hero.pos - start_time
			y = in_bounds(dtime / guitar_hero.note_moving_time, 0.0, 1.0)
			if guitar_hero.recording:
				y = 1 - y
			
			alpha = y / 0.2
			res.append((y, alpha))
		
		return res
	
	
	def guitar_hero__next_commands(to_pos):
		while guitar_hero.cur_note_index < len(guitar_hero.notes):
			end_time, string = guitar_hero.notes[guitar_hero.cur_note_index]
			start_time = end_time - guitar_hero.note_moving_time
			
			if start_time > to_pos:
				return
			
			guitar_hero.add_note(string or guitar_hero.get_next_random(), start_time)
			
			guitar_hero.cur_note_index += 1
	
	def guitar_hero__update():
		if guitar_hero.hide_time:
			dtime = get_game_time() - guitar_hero.hide_time
			guitar_hero.alpha = 1 - dtime / guitar_hero.disappearance_time
			if guitar_hero.alpha <= 0:
				hide_screen('guitar_hero')
		else:
			dtime = get_game_time() - guitar_hero.show_time
			guitar_hero.alpha = dtime / guitar_hero.appearance_time
		
		btn_ysize = style.guitar_hero_btn.get_current('ysize')
		song_btn_ysize = style.guitar_hero_song_btn.get_current('ysize')
		
		spacing = get_absolute(guitar_hero.spacing, get_stage_height())
		small_spacing = get_absolute(guitar_hero.small_spacing, get_stage_height())
		
		guitar_hero.panel_songs_ysize = get_stage_height() - spacing * 6 - 2 - btn_ysize
		count_top_btns = 1 # play / stop
		if guitar_hero.record_btn or guitar_hero.pause_btn:
			count_top_btns += 1
		if guitar_hero.close_btn:
			count_top_btns += 1
		guitar_hero.panel_songs_ysize -= btn_ysize * count_top_btns + small_spacing * (count_top_btns - 1)
		
		guitar_hero.count_songs = (guitar_hero.panel_songs_ysize + small_spacing) / (song_btn_ysize + small_spacing)
		
		guitar_hero.panel_songs_ysize = int(guitar_hero.panel_songs_ysize)
		guitar_hero.count_songs = int(guitar_hero.count_songs)
		
		guitar_hero.update_allowed_songs()
		if guitar_hero.count_songs < len(guitar_hero.cur_songs):
			guitar_hero.count_songs -= 2
			guitar_hero.show_arrows = True
		else:
			guitar_hero.show_arrows = False
		guitar_hero.fix_select_scroll()
		
		if guitar_hero.playing():
			guitar_hero.score_text = str(guitar_hero.score)
		else:
			guitar_hero.score_text = ' '
		
		if guitar_hero.playing() and guitar_hero.ok_in_row_for_x2 > 0:
			now = min(guitar_hero.ok_in_row, guitar_hero.ok_in_row_for_x2)
			guitar_hero.combo_text = 'x2: %s/%s' % (now, guitar_hero.ok_in_row_for_x2)
			
			if now == 0:
				guitar_hero.combo_color = guitar_hero.score_color_error
			elif now == guitar_hero.ok_in_row_for_x2:
				guitar_hero.combo_color = guitar_hero.score_color_good
			else:
				guitar_hero.combo_color = guitar_hero.score_color_usual
		else:
			guitar_hero.combo_text = ' '
			guitar_hero.combo_color = 0
		
		
		if guitar_hero.paused:
			return
		
		prev_mus_pos = guitar_hero.mus_pos
		mus_pos = guitar_hero.mus_pos = renpy.music.get_pos(guitar_hero.channel)
		if mus_pos is None:
			if prev_mus_pos is not None:
				guitar_hero.stop()
			return
		
		prev_pos = guitar_hero.pos or 0
		if mus_pos == prev_mus_pos and guitar_hero.prev_game_time is not None:
			pos = mus_pos + (get_game_time() - guitar_hero.prev_game_time)
		else:
			pos = mus_pos
			guitar_hero.prev_game_time = get_game_time()
		guitar_hero.pos = pos = max(pos, prev_pos)
		
		if not guitar_hero.recording:
			guitar_hero.next_commands(pos)
		
		for string_notes in guitar_hero.cur_notes.values():
			i = 0
			while i < len(string_notes):
				start_time = string_notes[i]
				end_time = start_time + guitar_hero.note_moving_time
				if end_time + guitar_hero.error_time <= pos:
					string_notes.pop(i)
					if not guitar_hero.recording:
						guitar_hero.add_score(-1)
				else:
					i += 1
		
		if guitar_hero.score_color_changed_start_time is not None:
			dtime = get_game_time() - guitar_hero.score_color_changed_start_time
			if dtime > guitar_hero.score_color_changed_time:
				guitar_hero.score_color = guitar_hero.score_color_usual
				guitar_hero.score_color_changed_start_time = None
	
	
	def guitar_hero__add_score(v):
		k = 1
		
		if v < 0:
			guitar_hero.ok_in_row = 0
			guitar_hero.errors_in_row += 1
			if guitar_hero.errors_in_row_for_stopping > 0 and guitar_hero.errors_in_row >= guitar_hero.errors_in_row_for_stopping:
				guitar_hero.stop()
		
		else:
			if guitar_hero.ok_in_row_for_x2 > 0 and guitar_hero.ok_in_row >= guitar_hero.ok_in_row_for_x2:
				k = 2
			guitar_hero.errors_in_row = 0
			guitar_hero.ok_in_row += 1
		
		guitar_hero.score = max(guitar_hero.score + v * k, 0)
		guitar_hero.score_color = guitar_hero['score_color_error' if v < 0 else 'score_color_good']
		guitar_hero.score_color_changed_start_time = get_game_time()
	
	
	def guitar_hero__get_name_for_song(song):
		audio_path, note_dir = guitar_hero.song_paths[song]
		name_path = note_dir + 'name'
		if os.path.isfile(name_path):
			return get_name_from_file(name_path)
		return song
	
	
	
	build_object('guitar_hero')
	
	guitar_hero.start_song_index = 0
	guitar_hero.cur_notes = defaultdict(list)
	
	
	# .append('your_path') / .insert(0, 'your_path')
	guitar_hero.note_dirs = [os.path.dirname(get_filename(0)) + '/songs/']
	guitar_hero.fallback_sound_dirs = ['sound/']
	
	# if len(allowed) > 0:
	#   only allowed
	# else:
	#   all found, except for disallowed
	guitar_hero.allowed = []
	guitar_hero.disallowed = []
	
	
	guitar_hero.screen_size_without_panel = True
	
	guitar_hero.close_btn = False
	guitar_hero.close_btn_text = 'Close'
	
	guitar_hero.block_difficulty = False
	guitar_hero.block_playing = False # on True, guitar_hero.play() does nothing
	guitar_hero.stop_btn = True
	
	guitar_hero.errors_in_row = 0
	guitar_hero.errors_in_row_for_stopping = 5 # 0 = disabled
	
	guitar_hero.ok_in_row = 0
	guitar_hero.ok_in_row_for_x2 = 3 # 0 = disabled
	
	guitar_hero.record_btn = True
	guitar_hero.pause_btn = True
	guitar_hero.paused = False
	
	guitar_hero.appearance_time = 0.5
	guitar_hero.disappearance_time = 0.5
	
	guitar_hero.waiting_before_start = 0.5
	
	guitar_hero.difficulty = persistent.get('guitar_hero_difficulty', 0)
	guitar_hero.difficulty_names = ['easy', 'medium', 'hard']
	
	guitar_hero.shadow_bg = im.rect('0005')
	guitar_hero.shadow_xpadding = 70 / 1200
	guitar_hero.shadow_ypadding = 40 / 675
	
	
	guitar_hero.channel = 'guitar_hero_music'
	renpy.music.register_channel(guitar_hero.channel, 'music', False)
	
	guitar_hero.bg = im.rect('#888')
	
	guitar_hero.panel_bg = im.rect('#DDD')
	guitar_hero.panel_size = 250
	
	guitar_hero.spacing = 12 / 675
	guitar_hero.small_spacing = 5 / 675
	
	guitar_hero.separator_bg = im.rect('#000')
	
	guitar_hero.string_spacing = 60 / 1200
	guitar_hero.string_colors = ('#C00', '#0C0', '#00C', '#CC0', '#C0C', '#0CC')
	guitar_hero.string_xsize = 6 / 1200
	guitar_hero.string_ysize = 225 / 675
	guitar_hero.string_light_border_size = 1
	
	def guitar_hero__gen_string_images(add_to_light_border):
		border_size = guitar_hero.string_light_border_size
		
		res = [None]
		for color in guitar_hero.string_colors:
			r, g, b, a = renpy.easy.color(color)
			light_color = [i + add_to_light_border for i in (r, g, b)]
			
			image = im.composite(
				(6, 6),
				(0, 0), im.rect(light_color, 6, 6),
				(border_size, border_size), im.rect(color, 6 - border_size * 2, 6 - border_size * 2),
			)
			res.append(image)
		return res
	guitar_hero.string_images = guitar_hero__gen_string_images(+128)
	
	
	guitar_hero.string_counts = [3, 4, 6]
	guitar_hero.string_count # calc in guitar_hero.update_difficulty()
	
	guitar_hero.error_times = [0.3, 0.25, 0.2]
	guitar_hero.error_time   # calc in guitar_hero.update_difficulty()
	
	guitar_hero.note_ignore_time = 1.0
	
	
	guitar_hero.note_moving_time = 2.0
	guitar_hero.note_xsize = 40 / 1200
	guitar_hero.note_ysize = 40 / 675
	guitar_hero.note_border_size = 4
	guitar_hero.note_light_border_size = 2
	
	def guitar_hero__gen_note_images(add_to_light_border, add_to_center):
		light_border_image_size = 64
		
		border_image_size = light_border_image_size - 2 * guitar_hero.note_light_border_size
		border_pos = guitar_hero.note_light_border_size
		
		size = border_image_size - guitar_hero.note_border_size * 2
		pos = guitar_hero.note_border_size + border_pos
		
		res = [None]
		for color in guitar_hero.string_colors:
			r, g, b, a = renpy.easy.color(color)
			light_color  = [i + add_to_light_border for i in (r, g, b)]
			center_color = [i + add_to_center       for i in (r, g, b)]
			
			light_border_image = im.circle(light_color, light_border_image_size, light_border_image_size)
			border_image       = im.circle(color, border_image_size, border_image_size)
			center_image       = im.circle(center_color, size, size)
			
			image = im.composite(
				(light_border_image_size, light_border_image_size),
				(0, 0),                   light_border_image,
				(border_pos, border_pos), border_image,
				(pos, pos),               center_image,
			)
			res.append(image)
		return res
	guitar_hero.note_images        = guitar_hero__gen_note_images(   0, +192)
	guitar_hero.note_target_images = guitar_hero__gen_note_images(+128, -176)
	
	
	guitar_hero.score = 0
	guitar_hero.score_color_usual = '#FF0'
	guitar_hero.score_color_error = '#F00'
	guitar_hero.score_color_good = '#0F0'
	guitar_hero.score_color = guitar_hero.score_color_usual
	guitar_hero.score_color_changed_start_time = -100
	guitar_hero.score_color_changed_time = 0.5
	
	
	guitar_hero.update_difficulty()
	
	
	
	signals.add('show_screen', guitar_hero.on_screen_show)
	signals.add('hide_screen', guitar_hero.on_screen_hide)
