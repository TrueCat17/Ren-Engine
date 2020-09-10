init -1002 python:
	def load_object(path):
		try:
			if (not os.path.exists(path)) or os.path.getsize(path) == 0:
				res = dict()
				out_msg('Persistent, load_object', 'File <' + path + '> is not exists or empty')
			else:
				tmp_file = open(path, 'rb')
				
				persistent_updates = True
				res = pickle.load(tmp_file)
				persistent_updates = False
				
				tmp_file.close()
			return res
		except:
			persistent_updates = False
			out_msg('Persistent, load_object', 'Error on loading object from file <' + path + '>')
			raise
	
	def save_object(path, obj):
		global persistent_updates
		
		try:
			dirname = os.path.dirname(path)
			if dirname and not os.path.isdir(dirname):
				os.mkdir(dirname)
			
			tmp_file = open(path, 'wb')
			
			persistent_updates = True
			pickle.dump(obj, tmp_file)
			persistent_updates = False
			
			tmp_file.close()
		except:
			persistent_updates = False
			out_msg('Persistent, save_object', 'Error on saving object to file <' + path + '>')
			raise
	
	def load_global_vars(path):
		g = globals()
		obj = load_object(path)
		for k in obj.keys():
			g[k] = obj[k]
	
	def save_global_vars(path):
		g = globals()
		obj = dict()
		
		class TmpClass: pass
		tmp_instance = TmpClass()
		
		simple_types = (type(None), bool, int, float, long, str, tuple)
		safe_types = (list, set, dict, type(TmpClass), type(tmp_instance))
		
		persistent_values = []
		for prop in persistent.get_props():
			o = persistent[prop]
			if type(o) not in simple_types:
				persistent_values.append(o)
		
		for k in g.keys():
			o = g[k]
			
			# don't save persistent
			if o is persistent:
				continue
			in_persistent = False
			for persistent_value in persistent_values:
				if o is persistent_value:
					in_persistent = True
					break
			if in_persistent:
				continue
			
			# renpy contains module <random>, modules can't saves
			# console_to_watch contains code-objects, that can't saves and
			# reference to globals() can't saves too
			if o is renpy or o is console_to_watch or o is g:
				continue
			
			t = type(o)
			if t in simple_types or t in safe_types:
				obj[k] = o
		
		save_object(path, obj)



init -1001 python:
	persistent_path = 'saves/persistent'
	
	try:
		if (not os.path.exists(persistent_path)) or os.path.getsize(persistent_path) == 0:
			persistent = Object()
		else:
			persistent_updates = True
			persistent = load_object(persistent_path)
			persistent_updates = False
	except:
		persistent_updates = False
		persistent = Object()
		raise


init -1000 python:
	persistent_updates = False
	persistent.in_persistent = True
	
	if not persistent.has_attr('config'):
		persistent.config = Object()
	
	for prop in ('_seen_labels', '_seen_images', '_seen_audio'):
		if not persistent.has_attr(prop):
			persistent[prop] = Object()
	
	if not persistent._seen_labels.has_key(get_current_mod()):
		persistent._seen_labels[get_current_mod()] = Object()
	
	persistent_need_save = False
	def persistent_save():
		global persistent_need_save
		if persistent_need_save:
			persistent_need_save = False
			save_object(persistent_path, persistent)

