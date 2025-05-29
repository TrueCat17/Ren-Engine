# reus = Ren-Engine Update System

# API:
#  funcs:
#   reus.scan_links()             - on first show <preferences> screen
#   reus.check(path = None)       - on btn <check> (if path is None - check all paths)
#   reus.check_and_load(path)     - on btn <update>
#   reus.stop_loading()           - on btn <cancel>
#   reus.get_loading_progress() -> None or (loaded:str, size_to_load:str) (in MB, '%.1f')
#  vars:
#   dont_save_reus.loading      - True / False
#   persistent.reus_storage     - .keys() - paths for checking, .values() - list[ReusInfo]
#   info.has_changes            - update available


init -100000 python:
	# make class before loading <persistent> object
	
	class ReusInfo(Object):
		def __init__(self, root):
			Object.__init__(
				self,
				root = root,
				link = None,
				cur_version = None,
				new_version = None,
				data = {}, # key - path:str, value - (size:str, hash:str)
				to_load = [],
				size_to_load = 0,
				has_changes = False,
			)
		
		def recalc(self):
			to_load = self.to_load = []
			size_to_load = 0
			has_changes = False
			
			for path, (size, hash) in self.data.items():
				if path.startswith('../') or '/../' in path:
					continue
				
				is_dir = path.endswith('/')
				is_removed = size == ''
				exists = os.path.exists(self.root + path)
				
				if is_removed and exists:
					has_changes = True
				
				if is_removed: continue
				if is_dir: continue
				
				if reus.get_hash(path) != hash:
					has_changes = True
				
				available_somewhere = reus.get_path(size, hash) != ''
				# available somewhere - can just copy - no need load - no need to consider size
				if not available_somewhere:
					size_to_load += int(size)
					to_load.append(path)
			
			to_load.sort()
			self.size_to_load = size_to_load
			self.has_changes = has_changes


init python:
	def reus__scan_links():
		root = get_root_dir()
		root_len = len(root)
		
		storage = persistent.setdefault('reus_storage', Object())
		
		paths = list(storage.keys())
		for path in paths:
			if not os.path.exists(root + path + reus.upd_link_fn):
				del storage[path]
		
		empty = True
		for path, ds, fs in os.walk(root):
			if reus.upd_link_fn not in fs:
				continue
			
			empty = False
			
			path = path[root_len:]
			if path:
				path = make_sure_dir(path)
			info = storage.setdefault(path, ReusInfo(path))
			
			with open(root + path + reus.upd_link_fn, 'rb') as f:
				link = f.read().decode('utf-8').strip()
			
			sep = '://'
			index = link.find(sep)
			if index != -1:
				link = link[index + len(sep):]
			
			info.link = make_sure_dir(link)
		
		if not empty:
			reus.scan_file_data()
	
	def reus__scan_file_data():
		old_files = persistent.get('reus_files', {})
		files = {} # new - for remove all removed files
		
		root = get_root_dir()
		root_len = len(root)
		for path, ds, fs in os.walk(root):
			path = make_sure_dir(path[root_len:])
			
			for f in fs:
				f = path + f
				
				stat = os.stat(root + f)
				mtime = stat.st_mtime
				size = str(stat.st_size)
				
				data = old_files.get(f)
				if data:
					prev_mtime, prev_size, prev_hash = data
					if (prev_mtime, prev_size) != (mtime, size):
						data = None
				
				if not data:
					data = [mtime, size, '']
				
				files[f] = data
		
		persistent.reus_files = files
	
	def reus__get_hash(path):
		reus_files = persistent.reus_files
		data = reus_files.get(path)
		
		full_path = get_root_dir() + path
		exists = os.path.exists(full_path)
		if not exists:
			if data:
				del reus_files[path]
			return ''
		
		if data:
			mtime, size, hash = data
			stat = os.stat(full_path)
			if mtime != stat.st_mtime or size != str(stat.st_size):
				data = None
		
		if not data:
			stat = os.stat(full_path)
			data = reus_files[path] = [stat.st_mtime, str(stat.st_size), '']
			mtime, size, hash = data
		
		if hash:
			return hash
		
		with open(full_path, 'rb') as f:
			content = f.read()
		import hashlib
		h = hashlib.new('sha256')
		h.update(content)
		hash = h.hexdigest()
		
		data[2] = hash
		global persistent_need_save
		persistent_need_save = True
		
		return hash
	
	def reus__get_path(file_size, file_hash):
		reus_files = persistent.reus_files
		paths = list(reus_files.keys()) # persistent.reus_files may change
		
		for path in paths:
			_mtime, size, hash = reus_files[path]
			if size != file_size: continue
			if not hash:
				hash = reus.get_hash(path)
			if hash == file_hash:
				return path
		return ''
	
	
	
	def reus__check(path = None, load_after = False):
		if dont_save_reus.get('loading'):
			return
		dont_save_reus.loading = True
		
		dont_save_reus.paths_to_check = [path] if path is not None else sorted(persistent.reus_storage.keys())
		dont_save_reus.check_index = 0
		dont_save_reus.load_after = load_after
		dont_save_reus.there_are_updates = False
		dont_save_reus.cur_info = None
		
		reus.check_next()
	
	def reus__check_and_load(path):
		reus.check(path, True)
	
	
	def reus__check_next():
		loaded_without_data = 'check_index' not in dont_save_reus
		if loaded_without_data:
			return
		
		if dont_save_reus.cur_info:
			dont_save_reus.cur_info.recalc()
			if dont_save_reus.cur_info.has_changes:
				dont_save_reus.there_are_updates = True
		
		if dont_save_reus.check_index >= len(dont_save_reus.paths_to_check):
			dont_save_reus.loading = False
			reus.scan_file_data()
			if dont_save_reus.there_are_updates:
				if dont_save_reus.load_after:
					reus.suggest()
				else:
					notification.out('There are updates')
			else:
				notification.out('No updates')
			return
		
		cur_path = dont_save_reus.paths_to_check[dont_save_reus.check_index]
		dont_save_reus.check_index += 1
		
		info = dont_save_reus.cur_info = persistent.reus_storage[cur_path]
		
		url = info.link + reus.version_fn
		dont_save_reus.local_path = get_root_dir() + reus.var_dir + reus.version_fn
		
		https.get_file(url, dont_save_reus.local_path, reus.on_version_loaded, reus.on_version_error, clean = True)
	
	
	def reus__on_check_error(fn):
		info = dont_save_reus.cur_info
		name = get_directory_name(info.root)
		url = info.link + fn
		notification.out('Error on check %s, url: %s' % (name, url))
		dont_save_reus.load_after = False
		reus.check_next()
	
	def reus__on_version_error():
		reus.on_check_error(reus.version_fn)
	def reus__on_info_error():
		reus.on_check_error(reus.info_fn)
	
	
	def reus__on_version_loaded():
		info = dont_save_reus.cur_info
		
		with open(dont_save_reus.local_path, 'rb') as f:
			dont_save_reus.tmp_version = f.read().decode('utf-8').strip()
		
		if dont_save_reus.tmp_version == info.new_version:
			reus.check_next()
			return
		
		params = dict(
			files_path = 'files',
			compress_info = 'false',
		)
		strs = dont_save_reus.tmp_version.split('\n')
		for s in strs:
			i = s.find('=')
			if i != -1:
				prop, value = s[:i].strip(), s[i+1:].strip()
				params[prop] = value
		
		info.files_path = make_sure_dir(params['files_path'])
		info.compress_info = params['compress_info'] == 'true'
		
		z_ext = '.z' if info.compress_info else ''
		url = info.link + reus.info_fn + z_ext
		
		dont_save_reus.local_path = get_root_dir() + reus.var_dir + reus.info_fn + z_ext
		https.get_file(url, dont_save_reus.local_path, reus.on_info_loaded, reus.on_info_error, clean = True)
	
	
	def reus__on_info_loaded():
		info = dont_save_reus.cur_info
		info.new_version = dont_save_reus.tmp_version
		
		with open(dont_save_reus.local_path, 'rb') as f:
			info_content = f.read()
			if info.compress_info:
				import zlib
				info_content = zlib.decompress(info_content)
			info_content = info_content.decode('utf-8').strip()
		
		data = info.data
		data.clear()
		for s in info_content.split('\n'):
			path, hash, size = s.split('|')
			if not path.startswith('var/'):
				data[path] = (size, hash)
		
		reus.check_next()
	
	
	def reus__suggest():
		info = dont_save_reus.cur_info
		
		size = info.size_to_load / (1 << 20)
		size = '%.1f' % size
		prompt = _('Need to download %s MB. Continue?') % size
		
		input.confirm(reus.suggest_answer, prompt)
	
	def reus__suggest_answer(yes):
		if yes:
			reus.load()
		else:
			notification.out('Cancel')
	
	
	def reus__load(recalc = True):
		if recalc:
			dont_save_reus.cur_info.recalc()
		
		if dont_save_reus.cur_info.to_load:
			dont_save_reus.loading = True
			dont_save_reus.loaded = 0
			dont_save_reus.to_load_index = 0
			dont_save_reus.local_path = ''
			persistent.setdefault('reus_unfinished_loadings', Object())
			reus.load_next_file()
			signals.send('reus_load')
		else:
			reus.replace(False)
	
	
	def reus__get_short_hash(file_hash):
		res = file_hash[0:4]
		for _mtime, _size, hash in persistent.reus_files.values():
			if hash.startswith(res):
				res += file_hash[len(res)]
		return res
	
	def reus__load_next_file():
		loaded_without_data = dont_save_reus.get('to_load_index') is None
		if loaded_without_data:
			return
		
		info = dont_save_reus.cur_info
		path = dont_save_reus.local_path
		if path:
			prev_file_size = os.stat(path).st_size
			size = str(prev_file_size)
			root = get_root_dir()
			path = path[len(root):]
			hash = reus.get_hash(path)
			data = (size, hash)
			
			del persistent.reus_unfinished_loadings[data]
			
			if data != (dont_save_reus.need_size, dont_save_reus.need_hash):
				notification.out(_('File <%s> has an invalid hash') % (path, ))
				reus.stop_loading(False)
				return
		else:
			prev_file_size = 0
		
		if dont_save_reus.to_load_index >= len(info.to_load):
			reus.stop_loading(False)
			reus.replace()
			return
		
		dont_save_reus.loaded += prev_file_size
		
		path = info.to_load[dont_save_reus.to_load_index]
		url = info.link + info.files_path + path
		dont_save_reus.to_load_index += 1
		
		data = info.data[path]
		size, hash = data
		dont_save_reus.need_size, dont_save_reus.need_hash = data
		
		unfinished_loadings = persistent.reus_unfinished_loadings
		if data in unfinished_loadings:
			name = unfinished_loadings[data]
		else:
			name = reus.get_short_hash(dont_save_reus.need_hash)
			unfinished_loadings[data] = name
		dont_save_reus.local_path = get_root_dir() + reus.var_dir + name
		
		https.get_file(url, dont_save_reus.local_path, reus.load_next_file, reus.stop_loading)
	
	def reus__get_loading_progress():
		loaded_without_data = dont_save_reus.get('to_load_index') is None
		if loaded_without_data or not dont_save_reus.loading:
			return None
		
		loaded = dont_save_reus.loaded
		if os.path.exists(dont_save_reus.local_path):
			loaded += os.stat(dont_save_reus.local_path).st_size
		size_to_load = dont_save_reus.cur_info.size_to_load
		
		round = lambda v: '%.1f' % (v / (1 << 20))
		return round(loaded), round(size_to_load)
	
	def reus__stop_loading(close_https = True):
		dont_save_reus.loading = False
		dont_save_reus.to_load_index = None
		if close_https:
			https.close()
	
	
	def reus__replace(recalc = True):
		info = dont_save_reus.cur_info
		if recalc:
			info.recalc()
		
		if info.to_load:
			reus.load(False)
			return
		
		dont_save_reus.loading = False
		
		root = get_root_dir()
		root_len = len(root)
		
		var_dir = root + reus.var_dir
		
		reus_files = persistent.reus_files
		
		def get_hash(path):
			return reus_files[path[root_len:]][2]
		def del_hash(path):
			del reus_files[path[root_len:]]
		def set_hash(path, hash):
			stat = os.stat(path)
			reus_files[path[root_len:]] = [stat.st_mtime, str(stat.st_size), hash]
		
		def safe_fs_op(op, *paths):
			ok = True
			for path in paths:
				if not path.startswith(root):
					ok = False
					break
				if '/../' in path:
					ok = False
					break
			if not ok:
				out_msg('reus.safe_fs_op (%s)' % (op, ), 'Path <%s> is not safe', path)
				return
			
			if op in ('mv', 'cp'):
				src_path, dst_path = paths
				hash = get_hash(src_path)
				
				if op == 'mv':
					del_hash(src_path)
					os.rename(src_path, dst_path)
				else:
					shutil.copyfile(src_path, dst_path)
				set_hash(dst_path, hash)
			
			elif op == 'rmdir':
				path, = paths
				if len(os.listdir(path)) == 0:
					os.rmdir(path)
			
			else:
				out_msg('reus.safe_fs_op', 'Unknown operation %s', op)
		
		# simple and incorrect way: 1 -> 2 with deleting 2 if exists
		#   problem case: 1, 2 -> 2, 1
		# correct way: 1, 2 -> _rm_1, _rm_2 -> 2, 1
		def set_tmp_name(path):
			while True:
				new_path = var_dir + '_remove_' + str(random.randint(0, 999999))
				if not os.path.exists(new_path):
					break
			safe_fs_op('mv', path, new_path)
		
		
		# preparing
		
		to_remove_dirs = []
		
		# key = (size, hash), value = [path1, path2...]
		to_remove = defaultdict(list)
		to_copy = defaultdict(list)
		
		info_root = root + info.root
		for path, data in info.data.items():
			full_path = info_root + path
			
			size, hash = data
			is_removed = size == ''
			is_dir = path.endswith('/')
			
			if is_removed:
				if os.path.exists(full_path):
					if is_dir:
						to_remove_dirs.append(full_path)
					else:
						to_remove[data].append(full_path)
				continue
			
			if is_dir:
				os.makedirs(full_path, exist_ok = True)
				continue
			
			if reus.get_hash(path) == hash:
				continue
			
			if os.path.exists(full_path):
				set_tmp_name(full_path)
			to_copy[data].append(full_path)
		
		for path, ds, fs in os.walk(var_dir):
			path = make_sure_dir(path[root_len:])
			
			for f in fs:
				if f == reus.info_fn: continue
				if f == reus.info_fn + '.z': continue
				if f == reus.version_fn: continue
				if f.startswith('_remove_'): continue
				
				f = path + f
				_mtime, size, hash = reus_files[f]
				to_remove[(size, hash)].append(root + f)
		
		to_move = []
		for data, paths_to_remove in to_remove.items():
			paths_to_copy = to_copy[data]
			for path in paths_to_remove.copy():
				if paths_to_copy:
					paths_to_remove.remove(path)
					dst_path = paths_to_copy.pop()
					to_move.append((path, dst_path))
		
		# ready, go:
		#   copy, move, remove, remove dirs
		
		for (size, hash), paths_to_copy in to_copy.items():
			src_path = root + reus.get_path(size, hash)
			
			for dst_path in paths_to_copy:
				safe_fs_op('cp', src_path, dst_path)
		
		for src_path, dst_path in to_move:
			safe_fs_op('mv', src_path, dst_path)
		
		for paths_to_remove in to_remove.values():
			for path in paths_to_remove:
				set_tmp_name(path)
		
		for path in to_remove_dirs:
			safe_fs_op('rmdir', path)
		
		
		def mark_exec(path):
			if os.path.exists(path):
				old_mode = os.stat(path).st_mode
				os.chmod(path, old_mode | 0o111)
		
		name = get_from_hard_config('window_title', str)
		for f in reus.exec_files:
			if '%s' in f:
				f = f % name
			mark_exec(root + f)
		
		
		info.cur_version = info.new_version
		info.recalc()
		
		persistent.reus_last_cleaning = 0
		
		notification.out('Updated\nIt is recommended to restart')
	
	
	def reus__clear_prev():
		now = time.time()
		day = 60 * 60 * 24
		if now - persistent.get('reus_last_cleaning', 0) < day:
			return
		persistent.reus_last_cleaning = now
		
		var_dir = get_root_dir() + reus.var_dir
		if not os.path.exists(var_dir):
			return
		
		for f in os.listdir(var_dir):
			full_path = var_dir + f
			
			remove = '_remove_' in f
			
			if not remove:
				stat = os.stat(full_path)
				last_change = max(stat.st_mtime, stat.st_ctime) # mtime - for modified files, ctime - for changed (renamed/moved)
				remove = now - last_change > 7 * day # remove old files, but not new (case: close app on downloading time)
			
			if remove:
				try:
					os.remove(full_path)
				except:
					pass # ignore errors with open files
	
	
	build_object('reus')
	
	reus.var_dir = 'var/reus/'
	reus.upd_link_fn = 'upd_link.txt'
	reus.version_fn = 'version.txt'
	reus.info_fn = 'info.txt'
	
	reus.exec_files = [
		'%s.exe',
		'%s.sh',
		'Ren-Engine/%s.exe',
		'Ren-Engine/linux-i686',
		'Ren-Engine/linux-x86_64',
	]
	
	dont_save_reus = DontSave()
	
	if get_current_mod_index() == 0:
		reus.clear_prev() # on each restart
