init -200 python:
	def guitar_hero__set_song(song):
		if song not in guitar_hero.cur_songs:
			out_msg('guitar_hero.set_song', 'Song <%s> not found or not allowed', song)
			return
		
		guitar_hero.song_index = guitar_hero.cur_songs.index(song)
		persistent.guitar_hero_song = guitar_hero.song = song
		
		guitar_hero.audio_path, guitar_hero.note_dir = guitar_hero.song_paths[song]
	
	
	def guitar_hero__select_prev_song():
		if guitar_hero.song_index == 0:
			return
		
		guitar_hero.song_index -= 1
		guitar_hero.set_song(guitar_hero.cur_songs[guitar_hero.song_index])
		if guitar_hero.start_song_index > guitar_hero.song_index:
			guitar_hero.start_song_index = guitar_hero.song_index
	
	def guitar_hero__select_next_song():
		if guitar_hero.song_index == len(guitar_hero.cur_songs) - 1:
			return
		
		guitar_hero.song_index += 1
		guitar_hero.set_song(guitar_hero.cur_songs[guitar_hero.song_index])
		if guitar_hero.song_index > guitar_hero.start_song_index + (guitar_hero.count_songs - 1):
			guitar_hero.start_song_index = guitar_hero.song_index - (guitar_hero.count_songs - 1)
	
	
	def guitar_hero__update_allowed_songs():
		if guitar_hero.allowed:
			guitar_hero.cur_songs = [song for song in guitar_hero.song_paths.keys() if song in guitar_hero.allowed]
		else:
			guitar_hero.cur_songs = [song for song in guitar_hero.song_paths.keys() if song not in guitar_hero.disallowed]
	
	def guitar_hero__fix_select_scroll():
		if guitar_hero.initing: return
		
		if guitar_hero.song not in guitar_hero.cur_songs:
			if guitar_hero.cur_songs:
				guitar_hero.set_song(guitar_hero.cur_songs[0])
			else:
				guitar_hero.song = None
		
		if len(guitar_hero.cur_songs) > guitar_hero.count_songs:
			max_start_index = len(guitar_hero.cur_songs) - guitar_hero.count_songs
			guitar_hero.start_song_index = min(guitar_hero.start_song_index, max_start_index)
		
		if guitar_hero.start_song_index > guitar_hero.song_index:
			guitar_hero.start_song_index = guitar_hero.song_index
		if guitar_hero.song_index > guitar_hero.start_song_index + (guitar_hero.count_songs - 1):
			guitar_hero.start_song_index = guitar_hero.song_index - (guitar_hero.count_songs - 1)
	
	
	def guitar_hero__get_songs():
		start = guitar_hero.start_song_index
		end = start + guitar_hero.count_songs
		
		res = guitar_hero.cur_songs[start:end]
		return res
