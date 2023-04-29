#!/usr/bin/env -S python3 -B

import os
import shutil

was_error = False
for prog in ('g++', 'make', 'cmake'):
	if os.system('which ' + prog + ' > /dev/null'):
		was_error = True
		print('You must install <' + prog + '>')

if was_error:
	os.sys.exit(1)



build = os.path.dirname(os.path.abspath(__file__)) + '/'
build = build.replace('\\', '/')



def set_platform():
	global platform
	print('1/4: Platform')
	
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
	
	else:
		platform = 'win32'
		
		shutil.copyfile(build + '../libs/py_win32/msvcr90.dll',  build + 'msvcr90.dll')
		shutil.copyfile(build + '../libs/py_win32/python27.dll', build + 'python27.dll')
	
	print('  platform =', platform)
	print()
	
	if not os.path.exists(build + '../libs/' + platform):
		if platform == 'win32':
			for_platform = ' '
		else:
			for_platform = ' for platform <' + platform + '> '
		print('You must build libs' + for_platform + 'before (see libs/readme)')
		os.sys.exit(1)


def set_lto():
	global lto
	lto = 'disabled'
	
	print('2/4: LTO')
	if platform == 'win32':
		print('Disabled for win32')
	else:
		print('Link Time Optimization make building very slow.')
		if input('Input 1 for enable LTO: ') == '1':
			lto = 'enabled'
	
	print('  LTO:', lto)
	print()
	
	if not os.path.exists(build + '../libs/' + platform + '/' + ('lto' if lto == 'enabled' else 'no_lto')):
		print('You must build libs ' + ('with' if lto == 'enabled' else 'without') + ' LTO before (see libs/readme)')
		os.sys.exit(1)


def set_pgo():
	print('3/4: PGO')
	
	if platform == 'win32':
		print('Disabled for win32')
		mode = '0'
	else:
		print('Profile-Guided Optimization require 2 builds. How use it:')
		print(' * build with <profile-generate>')
		print('   * run compiled Ren-Engine and start some hard mods')
		print(' * build with <profile-use>')
		print('   * got final version')
		
		print('Choose PGO mode:')
		print('0. Disable')
		print('1. Generate')
		print('2. Use')
		
		while True:
			mode = input('Input 1 or 2 for generate or use PGO: ')
			if mode == '':
				mode = '0'
			if mode in ('0', '1', '2'):
				break
			print('Expected 0, 1 or 2')
	
	global pgo
	pgo = ['', 'generate', 'use'][int(mode)]
	print('  PGO:', pgo or 'disabled')
	print()


def set_multithreading():
	global count_threads
	print('4/4: Multithreading')
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
set_pgo()
set_multithreading()

cmake_cmd = (
	'cmake ../src ' +
		'-DCMAKE_BUILD_TYPE=Release ' +
		'-G "Unix Makefiles"' + ' ' +
		'-DPLATFORM=' + platform + ' ' +
		'-DLTO=' + ('1' if lto == 'enabled' else '0') + ' ' +
		'-DPROFILE=' + pgo
)

error = os.system(cmake_cmd)
if error:
	os.sys.exit(error)

make_cmd = 'make -j' + str(count_threads)
error = os.system(make_cmd)
if error:
	os.sys.exit(error)

print('Ok!')
