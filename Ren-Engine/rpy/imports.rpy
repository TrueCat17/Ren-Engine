init -1000001 python:
	import os
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
	
	
	class SslImporter:
		def find_module(self, fullname, path = None):
			if fullname == '_ssl':
				return self
			return None
		
		def load_ssl(self, ver):
			if 'linux' in sys.platform:
				bits = 64 if sys.maxsize > 2**32 else 32
				shared_dir = 'linux-' + ('x86_64' if bits == 64 else 'i686')
			else:
				shared_dir = 'win32'
			path = '../Ren-Engine/py_libs/dyn/%s/_ssl_v%s.so' % (shared_dir, ver)
			
			import importlib.util
			spec = importlib.util.spec_from_file_location('_ssl', path)
			_ssl = importlib.util.module_from_spec(spec)
			sys.modules['_ssl'] = _ssl
			spec.loader.exec_module(_ssl)
		
		def load_module(self, fullname):
			try:
				self.load_ssl('3') # linked with new lib: libssl.so.3
			except:
				self.load_ssl('1') # linked with old lib: libssl.so.1.1
	
	sys.meta_path.insert(0, SslImporter())
