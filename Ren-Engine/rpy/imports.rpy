init -1000001 python:
	import os
	import sys
	import time
	import random
	import math
	import shutil
	import cPickle as pickle
	import traceback
	import itertools
	import types
	
	from collections import defaultdict
	
	if sys.platform in ('win32', 'msys', 'msys2', 'cygwin'):
		sys.path.append("../Ren-Engine/py_libs/win32.dll/")
	
	if 'linux' in sys.platform:
		sys.path.append("../Ren-Engine/py_libs/linux.so/")
