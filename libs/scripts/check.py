#!/usr/bin/env -S python3 -B

import os
from config import *


for prog in ('nasm', 'libtoolize', 'automake', 'autoconf', 'make'):
	if os.system('command -v ' + prog + ' > /dev/null'):
		print('You must install <' + prog + '>')


for i in os.listdir(sources_path):
	if os.path.isfile(i):
		continue
	
	for lib in libs:
		if i.startswith(lib) and not i.startswith(lib + '_'):
			libs.remove(lib)
			break

if libs:
	print('Download and unpack to <' + sources_path + '> next libs:')
	for lib in libs:
		print('  ' + lib)
else:
	print('All libs exists.')
