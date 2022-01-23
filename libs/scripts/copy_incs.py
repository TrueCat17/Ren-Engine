#!/usr/bin/env -S python3 -B

import os
import shutil
from config import *

params = read_params()
platform = params['platform']


inc_path = scripts_path + '../' + platform + '/include/'
if os.path.exists(inc_path):
	shutil.rmtree(inc_path)



if platform == 'win32':
	shutil.copytree(scripts_path + '../py_win32/include', inc_path + '/python2.7')
else:
	for i in os.listdir(sources_path):
		if os.path.isdir(sources_path + i) and i.startswith('Python'):
			shutil.copytree(sources_path + i + '/Include', inc_path + '/python2.7')
			
			src_pyconfig = open(sources_path + i + '/pyconfig.h', 'rb')
			dst_pyconfig = open(inc_path + '/python2.7/pyconfig.h', 'wb')
			
			macros = '_POSIX_C_SOURCE _XOPEN_SOURCE _XOPEN_SOURCE_EXTENDED __BSD_VISIBLE __EXTENSIONS__'.split(' ')
			for line in src_pyconfig:
				line = str(line, 'utf8')
				
				for macro in macros:
					if line.startswith('#define ' + macro):
						line = '#ifndef ' + macro + '\n\t' + line + '#endif\n'
				
				dst_pyconfig.write(bytes(line, 'utf8'))
			
			src_pyconfig.close()
			dst_pyconfig.close()
			break
	else:
		print('Python sources not found')



for i in os.listdir(sources_path):
	if os.path.isdir(sources_path + i) and i.startswith('ffmpeg'):

		for lib in 'libavcodec libavformat libavutil libswresample'.split(' '):
			lib_path = sources_path + i + '/' + lib + '/'
			
			for path, ds, fs in os.walk(lib_path):
				path = os.path.relpath(path, lib_path)
				
				for f in fs:
					if f.endswith('.h'):
						if not os.path.exists(inc_path + lib + '/' + path):
							os.makedirs(inc_path + lib + '/' + path)
						shutil.copyfile(os.path.join(lib_path, path, f), os.path.join(inc_path + lib + '/', path, f))
		
		break
else:
	print('ffmpeg sources not found')



for i in os.listdir(sources_path):
	if os.path.isdir(sources_path + i) and i == 'SDL':
		shutil.copytree(sources_path + i + '/include', inc_path + 'SDL2')
		break
else:
	print('SDL sources not found')


for i in os.listdir(sources_path):
	if os.path.isdir(sources_path + i) and i == 'SDL_image':
		shutil.copyfile(sources_path + i + '/SDL_image.h', inc_path + 'SDL2/SDL_image.h')
		break
else:
	print('SDL_image sources not found')


for i in os.listdir(sources_path):
	if os.path.isdir(sources_path + i) and i == 'SDL_ttf':
		shutil.copyfile(sources_path + i + '/SDL_ttf.h', inc_path + 'SDL2/SDL_ttf.h')
		break
else:
	print('SDL_ttf sources not found')


