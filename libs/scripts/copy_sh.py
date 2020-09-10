#!/usr/bin/env -S python3 -B

import os
import shutil
from config import *

def copy(cmd):
	dirs = {}
	for i in os.listdir(sources_path):
		if not os.path.isdir(sources_path + '/' + i):
			continue
			
		for lib in libs:
			if i.startswith(lib) and not i.startswith(lib + '_'):
				dirs[lib] = i
	
	exit = False
	for lib in libs:
		if lib not in dirs.keys():
			print('<' + lib + '> path not found')
			exit = True
	if exit:
		os.sys.exit(1)
	
	
	for lib in os.listdir(scripts_path + cmd):
		name = lib[:-3]
		for i in os.listdir(sources_path):
			if i.startswith(name) and not i.startswith(name + '_'):
				name = i
				break
		else:
			continue
		
		src = scripts_path + cmd + '/' + lib
		dst = sources_path + name + '/' + cmd + '.sh'
		
		f = open(src, 'rb')
		content = str(f.read(), 'utf8')
		
		content = content.replace('CC="gcc"', 'CC="' + cc + '"')
		
		if os.sys.platform not in ('win32', 'msys', 'msys2'):
			content = content.replace('--host=mingw32', '')
			content = content.replace('--target-os=mingw32', '')
		
		if lto != 'enabled':
			content = content.replace('--with-lto', '--without-lto')
			content = content.replace('-flto', '')
			content = content.replace('--enable-optimizations', '--disable-optimizations')
		
		content = content.replace('000res', '../' + path)
		
		content = content.replace('-j4', '-j' + str(count_threads))
		
		for script in ('autogen.sh', 'configure'):
			content = content.replace('\n./' + script, '\nchmod +x ./' + script + '\n./' + script)
		
		for dep in dirs:
			lib_var = '\n' + dep.upper() + '_DIR='
			start = content.find(lib_var)
			if start != -1:
				end = content.find('\n', start + len(lib_var))
				content = content[:start] + lib_var + '"' + dirs[dep] + '"' + content[end:]
		
		f = open(dst, 'wb')
		f.write(bytes(content, 'utf8'))
		
		os.system('chmod +x "' + dst + '"')


def set_platform():
	global platform, cc
	print('1/5: Platform')
	
	if 'linux' in os.sys.platform:
		print('Choose platform:')
		print('1. i686   (x32)')
		print('2. x86_64 (x64)')
		
		action = input('Input number of action (empty = auto): ')
		if action == '1':
			platform = 'linux-i686'
		elif action == '2':
			platform = 'linux-x86_64'
		else:
			platform = 'linux-' + ('x86_64' if os.sys.maxsize > 2**32 else 'i686')
		
		if platform == 'linux-i686':
			cc = 'i686-linux-gnu-gcc'
		else:
			cc = 'x86_64-linux-gnu-gcc'
	
	else:
		platform = 'win32'
		cc = 'gcc'
	
	print('  platform  =', platform)
	print('  compilier =', cc)
	print()
	
	if os.system('which ' + cc + ' > /dev/null'):
		print('Compilier <' + cc + '> not found')
		os.sys.exit(1)


def set_lto():
	print('2/5: LTO')
	print('Link Time Optimization make building very slow.')
	
	global lto
	if input('Input 1 for enable LTO: ') == '1':
		lto = 'enabled'
	else:
		lto = 'disabled'
	
	global path
	path = platform + '/' + ('lto' if lto == 'enabled' else 'no_lto')
	if not os.path.exists(scripts_path + '../' + path):
		os.makedirs(scripts_path + '../' + path)
	
	if platform == 'win32':
		shutil.copyfile(scripts_path + '../py_win32/msvcr90.dll',  scripts_path + '../' + path + '/msvcr90.dll')
		shutil.copyfile(scripts_path + '../py_win32/python27.dll', scripts_path + '../' + path + '/python27.dll')
	
	print('  LTO:', lto)
	print()


def set_multithreading():
	global count_threads
	print('3/5: Multithreading')
	print('  Recommended use N build-threads on N-core PC.')
	print('  But more threads require more memory.')
	
	answer = input('Input count threads for building (empty = auto): ')
	if answer.isdigit() and int(answer) > 0:
		count_threads = int(answer)
	else:
		count_threads = os.cpu_count()
	print('  Using', count_threads, 'threads')
	print()



set_platform()
set_lto()
set_multithreading()

write_params({
	'platform': platform,
})


print('4/5: Copy files to configure libs')
copy('conf')
print('5/5: Copy files to build libs')
copy('make')
