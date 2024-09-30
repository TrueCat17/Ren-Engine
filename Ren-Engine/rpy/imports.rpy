init -1000001 python:
	import os
	
	# Ren-Engine helpers for cygwin
	os.getcwd = _get_cwd
	
	def win32_start_file(path, vars = None):
		if path.startswith('/cygdrive/'):
			path = path[len('/cygdrive/'):]
			if not path:
				return False
			if len(path) == 1: # just disk letter
				path += '/'
			
			if path[1] != '/':
				return False
			
			path = path[0] + ':' + path[1:]
		
		return _start_file_win32(path, vars or [])
	
	def win32_is_abs_path(path):
		return path.startswith('/') or path.startswith(':/', 1)
	
	if os.sys.platform in ('win32', 'cygwin'):
		os.startfile = win32_start_file
		os.path.isabs = win32_is_abs_path
	
	
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
		sep = kwargs.get('sep', ' ')
		end = kwargs.get('end')
		if end is None:
			end = '\n'
		
		kwargs.setdefault('flush', True)
		
		data = ''
		for arg in args:
			data += str(arg) + sep
		if data:
			data = data[:-len(sep)]
		
		_log_str_with_end(data, end)
		__builtins__.print(data, **kwargs)
	
	
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
