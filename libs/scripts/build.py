#!/usr/bin/env -S python3 -B

import os

from config import *
from progress import *

conf_cmd = './conf.sh 1>conf_out.txt 2>conf_err.txt'
make_cmd = './make.sh 1>make_out.txt 2>make_err.txt'


progress_list = read_progress()
for i in range(len(progress_list)):
	lib, progress = progress_list[i]
	if progress == 'maked': # start | downloaded | configurated | maked
		continue
	
	print('Build <' + lib + '>')
	if progress not in ('downloaded', 'configurated'):
		print('  You must download lib before building')
		os.sys.exit(1)
	
	for name in os.listdir(sources_path):
		if os.path.isdir(sources_path + name) and name.startswith(lib) and not name.startswith(lib + '_'):
			dir_name = sources_path + name
			break
	else:
		print('  Source dir not found')
		os.sys.exit(1)
	
	os.chdir(dir_name)
	
	if progress == 'downloaded':
		print(conf_cmd)
		error = os.system(conf_cmd)
		if error:
			print('Error on ./conf.sh of <' + lib + '>')
			os.sys.exit(1)
		
		progress_list[i] = lib, 'configurated'
		write_progress(progress_list)
	
	
	if lib == 'Python':
		print('  config py-modules')
		error = os.system(scripts_path + '/config_py_mods.py')
		if error:
			print('Error?! See scripts/config_py_mods.py')
			os.sys.exit(1)
	if lib == 'ffmpeg':
		print('  disable bcrypt')
		lines = []
		for line in open('./config.h', 'rb'):
			line = str(line, 'utf8')
			line = line.replace('HAVE_BCRYPT 1', 'HAVE_BCRYPT 0')
			lines.append(bytes(line, 'utf8'))
		f = open('./config.h', 'wb')
		f.writelines(lines)
		del f, line, lines
	
	
	print(make_cmd)
	error = os.system(make_cmd)
	if error:
		print('Error on ./make.sh of <' + lib + '>')
		os.sys.exit(1)
	
	progress_list[i] = lib, 'maked'
	write_progress(progress_list)
	
	os.chdir(sources_path)

print('Ok!')
