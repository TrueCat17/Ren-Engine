import os

libs = ['Python', 'jemalloc', 'ffmpeg', 'zlib', 'brotli', 'freetype', 'jpeg', 'libpng', 'libwebp', 'SDL2', 'SDL2_image', 'SDL2_ttf']
if os.sys.platform in ('win32', 'msys', 'msys2'):
	libs.remove('Python')
	libs.remove('jemalloc')

bits = 64
if bits == 64:
	cc = 'gcc'
else:
	cc = 'i686-linux-gnu-gcc'


build = os.path.dirname(os.path.abspath(__file__))
build = build.replace('\\', '/')

for c in build:
	if c >= 'a' and c <= 'z': continue
	if c >= 'A' and c <= 'Z': continue
	if c >= '0' and c <= '9': continue
	if c not in '-_/:':
		print 'Path contains bad symbol: ' + c
		os.sys.exit(1)

main = os.path.dirname(build)
