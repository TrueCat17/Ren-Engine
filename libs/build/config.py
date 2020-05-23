import os

libs = ['Python', 'jemalloc', 'ffmpeg', 'zlib', 'freetype', 'jpeg', 'libpng', 'libwebp', 'SDL2', 'SDL2_image', 'SDL2_ttf']
if os.sys.platform == 'win32':
	libs.remove('Python')
	libs.remove('jemalloc')

cc = 'gcc' # usual
#cc = 'i686-linux-gnu-gcc' # linux-i686


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
