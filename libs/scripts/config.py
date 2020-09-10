import os

libs = ['Python', 'jemalloc', 'ffmpeg', 'zlib', 'brotli', 'freetype', 'jpeg', 'libpng', 'libwebp', 'SDL2', 'SDL2_image', 'SDL2_ttf']
if os.sys.platform in ('win32', 'msys', 'msys2'):
	libs.remove('Python')
	libs.remove('jemalloc')

scripts_path = os.path.dirname(os.path.abspath(__file__)) + '/'
scripts_path = scripts_path.replace('\\', '/')

for c in scripts_path:
	if c >= 'a' and c <= 'z': continue
	if c >= 'A' and c <= 'Z': continue
	if c >= '0' and c <= '9': continue
	if c not in '-_/:':
		print('Path contains bad symbol: ' + c)
		os.sys.exit(1)

download_path = scripts_path + '../download/'
sources_path  = scripts_path + '../sources/'



params_path = scripts_path + 'params.txt'

def read_params():
	res = {}
	
	f = open(params_path, 'rb')
	for line in f:
		name, value = str(line, 'utf8').strip().split(' ')
		res[name] = value
	
	return res

def write_params(params):
	f = open(params_path, 'wb')
	for name in params:
		value = params[name]
		f.write(bytes(name + ' ' + value + '\n', 'utf8'))
