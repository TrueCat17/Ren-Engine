#!/usr/bin/env -S python3 -B

import os

ok = True
for prog in ('g++', 'make', 'cmake'):
	if os.system('command -v %s > /dev/null' % prog):
		ok = False
		print('You must install <%s>' % prog)

if not ok:
	os.sys.exit(1)


build = os.path.dirname(os.path.abspath(__file__)) + '/'


def set_platform():
	global platform
	print('1/4: Platform')
	
	if 'linux' in os.sys.platform:
		print('Choose platform:')
		print('1. i686   (x32)')
		print('2. x86_64 (x64)')
		
		action = input('Input number of platform (empty = auto): ')
		if action not in ('1', '2'):
			action = '2' if os.sys.maxsize > 2**32 else '1'
		
		platform = 'linux-' + ('x86_64' if action == '2' else 'i686')
	else:
		platform = 'win32'
	
	print('  platform = %s' % platform)
	print()
	
	if not os.path.isdir(build + '../libs/' + platform):
		if platform == 'win32':
			libs_for_platform = 'libs'
		else:
			libs_for_platform = 'libs for platform <%s>' % platform
		print('You must build %s before (see libs/readme)' % libs_for_platform)
		os.sys.exit(1)


def set_lto():
	global lto
	lto = False
	
	print('2/4: LTO')
	if platform == 'win32':
		print('Disabled for win32')
	else:
		print('Link Time Optimization make building very slow.')
		if input('Input 1 to enable LTO: ') == '1':
			lto = True
	
	print('  LTO: %s' % ('enabled' if lto else 'disabled'))
	print()
	
	mode = 'lto' if lto else 'no_lto'
	path = build + '../libs/' + platform + '/' + mode
	if not os.path.isdir(path):
		with_out = 'with' if lto else 'without'
		print('You must build libs %s LTO before (see libs/readme)' % with_out)
		os.sys.exit(1)


def set_pgo():
	print('3/4: PGO')
	
	if platform == 'win32':
		print('Disabled for win32')
		mode = '0'
	else:
		print('Profile-Guided Optimization requires 2 builds. How to use it:')
		print(' * build with <profile-generate>')
		print('   * run compiled Ren-Engine, start some hard mods and close app')
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
			if mode in '012':
				break
			print('Expected 0, 1 or 2')
	
	global pgo
	pgo = ['', 'generate', 'use'][int(mode)]
	print('  PGO: %s' % (pgo or 'disabled'))
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
	print('  Using %i threads' % count_threads)
	print()



set_platform()
set_lto()
set_pgo()
set_multithreading()

cmake_cmd = (
	'cmake ../src ' +
		'-G "Unix Makefiles"' + ' ' +
		'-DCMAKE_CXX_FLAGS="-Wno-address" ' + # disable useless warnings of old gcc (about constexpr cmp of template param with NULL)
		'-DCMAKE_BUILD_TYPE=Release ' +
		'-DPLATFORM=' + platform + ' ' +
		'-DLTO=' + ('1' if lto else '0') + ' ' +
		'-DPROFILE=' + pgo
)

if os.system(cmake_cmd):
	os.sys.exit(1)

make_cmd = 'make -j%i' % count_threads
if os.system(make_cmd):
	os.sys.exit(1)

print('Ok!')
