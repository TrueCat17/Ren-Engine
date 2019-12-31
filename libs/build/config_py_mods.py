#!/bin/python2

import os
from shutil import copyfile

from config import *


libs_on = ['math', '_struct', 'time', 'operator', '_random', '_collections', 'itertools', '_md5', '_sha', '_sha256', '_sha512', 'binascii', 'cStringIO', 'cPickle']
libs_off = ['pwd']

for i in os.listdir(main):
	if i.startswith('Python-'):
		mods = main + '/' + i + '/Modules/'
		break
else:
	print 'Python-dir not found'
	os.sys.exit(1)

if not os.path.exists(mods + 'Setup'):
	copyfile(mods + 'Setup.dist', mods + 'Setup')

src = open(mods + 'Setup.dist', 'rb')
dst = open(mods + 'Setup', 'wb')
for s in src:
	for lib in libs_on:
		if s.startswith('#' + lib + ' '):
			s = s[1:]
			break
	for lib in libs_off:
		if s.startswith(lib + ' '):
			s = '#' + s
	dst.write(s)

