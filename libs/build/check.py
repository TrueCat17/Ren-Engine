#!/bin/python2

import os
from config import *

for i in os.listdir(main):
	if os.path.isfile(i):
		continue
	
	for lib in libs:
		if i.startswith(lib + '-'):
			libs.remove(lib)
			break

if libs:
	print 'Download and unpack to <' + main + '> next libs:'
	for lib in libs:
		print '  ' + lib
else:
	print 'All libs exists.'
