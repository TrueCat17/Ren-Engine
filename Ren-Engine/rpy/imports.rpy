init -1000001 python:
	import os
	
	# Ren-Engine helpers for cygwin
	os.getcwd = _get_cwd
	if os.sys.platform in ('win32', 'cygwin'):
		os.startfile = _start_file_win32
		
		def _is_abs_path(path):
			return path.startswith('/') or path.startswith(':/', 1)
		os.path.isabs = _is_abs_path
	
	
	import sys
	import time
	import random
	import math
	import shutil
	import pickle
	import traceback
	import types
	
	from collections import defaultdict
	
	def print(*args, **kwargs):
		if 'flush' not in kwargs:
			kwargs['flush'] = True
		__builtins__.print(*args, **kwargs)
	
	
	class HashlibImporter:
		def find_module(self, fullname, path = None):
			if fullname == 'hashlib':
				return self
			return None
		
		def load_module(self, fullname):
			sys.meta_path.remove(self)
			import importlib
			hashlib = importlib.import_module(fullname)
			
			# hashlib.md5 = hashlib.new("md5")
			def make_func(algo):
				def tmp(*args, **kwargs):
					return hashlib.new(algo, *args, **kwargs)
				return tmp
			for algo in ('md5', 'sha1', 'sha224', 'sha256', 'sha384', 'sha512'):
				setattr(hashlib, algo, make_func(algo))
	
	sys.meta_path.insert(0, HashlibImporter())
