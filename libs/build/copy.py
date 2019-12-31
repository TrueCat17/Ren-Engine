#!/bin/python2

import os
from config import *

def copy(cmd, lto = True):
	dirs = {}
	for i in os.listdir(main):
		if not os.path.isdir(main + '/' + i):
			continue
			
		for lib in libs:
			if i.startswith(lib + '-'):
				dirs[lib] = i
	
	exit = False
	for lib in libs:
		if not dirs.has_key(lib):
			print '<' + lib + '> path not found'
			exit = True
	if exit:
		os.sys.exit(1)
	
	
	for lib in os.listdir(build + '/' + cmd):
		name = lib[:-3] + '-'
		for i in os.listdir(main):
			if i.startswith(name):
				name = i
				break
		
		src = build + '/' + cmd + '/' + lib
		dst = main + '/' + name + '/' + cmd + '.sh'
		
		f = open(src, 'rb')
		content = f.read()
		
		if not lto:
			content = content.replace('--with-lto', '--without-lto').replace('-flto', '')
		content = content.replace('CC="gcc"', 'CC="' + cc + '"')
		
		if os.sys.platform != 'win32':
			content = content.replace('--host=mingw32', '')
			content = content.replace('--target-os=mingw32', '')
		
		for dep in dirs:
			lib_var = '\n' + dep.upper() + '_DIR='
			start = content.find(lib_var)
			if start != -1:
				end = content.find('\n', start + len(lib_var))
				content = content[:start] + lib_var + '"' + dirs[dep] + '"' + content[end:]
		
		f = open(dst, 'wb')
		f.write(content)
		
		os.system('chmod +x "' + dst + '"')

if __name__ == '__main__':
	copy('conf')
	copy('make')

