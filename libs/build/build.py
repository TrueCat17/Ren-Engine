#!/bin/python2

import os

from config import *
from progress_list import *

conf_cmd = './conf.sh 1>conf_out.txt 2>conf_err.txt'
make_cmd = './make.sh 1>make_out.txt 2>make_err.txt'


if not os.path.exists(main + '/000res'):
	os.mkdir(main + '/000res')


progress_list = read_progress()
for i in xrange(len(progress_list)):
	lib, progress = progress_list[i]
	if progress == 'maked': # start | configurated | maked
		continue
	
	print 'Build <' + lib + '>'
	
	for name in os.listdir(main):
		if os.path.isdir(main + '/' + name) and name.startswith(lib + '-'):
			dir_name = main + '/' + name
			break
	else:
		print 'Dir for <' + lib + '> not found'
		os.sys.exit(1)
	
	os.chdir(dir_name)
	
	if progress == 'start':
		print conf_cmd
		error = os.system(conf_cmd)
		if error:
			print 'Error on ./conf.sh of <' + lib + '>'
			os.sys.exit(1)
		
		progress = 'configurated'
		progress_list[i] = [lib, progress]
		write_progress(progress_list)
	
	
	if lib == 'Python':
		print 'config py-modules'
		error = os.system(build + '/config_py_mods.py')
		if error:
			print 'Error?! See build/config_py_mods.py'
			os.sys.exit(1)
	if lib == 'ffmpeg':
		print 'disable bcrypt'
		lines = []
		for line in open('./config.h'):
			line = line.replace('HAVE_BCRYPT 1', 'HAVE_BCRYPT 0')
			lines.append(line)
		f = open('./config.h', 'wb')
		f.writelines(lines)
		del f, line, lines
	
	
	print make_cmd
	error = os.system(make_cmd)
	if error:
		print 'Error on ./make.sh of <' + lib + '>'
		os.sys.exit(1)
	
	progress = 'maked'
	progress_list[i] = lib, progress
	write_progress(progress_list)
	
	os.chdir(main)

print 'Ok!'
