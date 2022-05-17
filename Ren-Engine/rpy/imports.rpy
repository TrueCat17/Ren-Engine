init -100001 python:
	import os
	import sys
	import time
	import random
	import math
	import shutil
	import cPickle as pickle
	import traceback
	import itertools
	
	if sys.platform in ('win32', 'msys', 'msys2', 'cygwin'):
		sys.path.append("../Ren-Engine/py_libs/win32.dll/")
	
	if 'linux' in sys.platform:
		if sys.maxsize > 2**32:
			sys.path.append("../Ren-Engine/py_libs/linux-x86_64.so/")
		else:
			sys.path.append("../Ren-Engine/py_libs/linux-i686.so/")
