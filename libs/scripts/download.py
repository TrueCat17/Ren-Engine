#!/usr/bin/env -S python3 -B

from config import *
from progress import *

import os
import shutil
from urllib import request

from zipfile import ZipFile
import tarfile


# claudflare on jpeg-site set ban for python/urllib user-agent
user_agent = 'Mozilla/5.0 (X11; Linux x86_64; rv:80.0) Gecko/20100101 Firefox/80.0'


urls = {
	'Python':     'https://www.python.org/ftp/python/2.7.18/Python-2.7.18.tgz',
	'jemalloc':   'https://github.com/jemalloc/jemalloc.git',
	'ffmpeg':     'https://ffmpeg.org/releases/ffmpeg-4.4.tar.gz',
	'zlib':       'https://www.zlib.net/zlib-1.2.11.tar.gz',
	'brotli':     'https://github.com/google/brotli.git',
	'freetype':   'https://download.savannah.gnu.org/releases/freetype/freetype-2.10.4.tar.gz',
	'jpeg':       'https://www.ijg.org/files/jpegsrc.v9d.tar.gz',
	'libpng':     'https://github.com/glennrp/libpng.git',
	'libwebp':    'https://github.com/webmproject/libwebp.git',
	'SDL2':       'https://www.libsdl.org/release/SDL2-2.0.16.zip',
	'SDL2_image': 'https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.5.zip',
	'SDL2_ttf':   'https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.15.zip',
}


if not os.path.exists(download_path):
	os.makedirs(download_path)
if not os.path.exists(sources_path):
	os.makedirs(sources_path)

def extract(f):
	if f.endswith('zip'):
		archive = ZipFile(f, 'r')
	elif f.endswith('gz'):
		archive = tarfile.open(f, 'r:gz')
	else:
		was_error = True
		print('  Archive <' + f + '> does not ends with zip or gz')
		return
	
	archive.extractall(sources_path)


was_error = False
downloaded = []

progress_list = read_progress()
for i in range(len(progress_list)):
	lib, progress = progress_list[i]
	if lib not in urls:
		print('URL for <' + lib + '> is unknown') 
		continue
	
	if progress != 'start':
		continue
	
	
	for d in os.listdir(sources_path):
		if os.path.isdir(sources_path + d) and d.startswith(lib) and not d.startswith(lib + '_'):
			shutil.rmtree(sources_path + d)
			break
	
	url = urls[lib]
	
	try:
		print('Download <' + lib + '>')
		
		if 'git' in url:
			dirname, _ = os.path.splitext(os.path.basename(url))
			if not os.path.exists(download_path + dirname):
				os.chdir(download_path)
				cmd = 'git clone --depth=1 ' + url
				if os.system(cmd):
					print('Comand <' + cmd + '> failed')
			else:
				print('  use cached')
			
			shutil.copytree(download_path + dirname, sources_path + dirname)
			
		else:
			file_path = download_path + os.path.basename(url)
			
			if not os.path.exists(file_path):
				req = request.Request(url)
				req.add_header('User-Agent', user_agent)
				
				sock = request.urlopen(req)
				content = sock.read()
				sock.close()
				
				f = open(file_path, 'wb')
				f.write(content)
				f.close()
			else:
				print('  use cached')
			
			extract(file_path)
		
		downloaded.append(lib)
		
		progress_list[i] = lib, 'downloaded'
		write_progress(progress_list)
		
	except BaseException as e:
		if type(e) is KeyboardInterrupt:
			os.sys.exit(1)
		
		was_error = True
		print('Error on download <' + lib + '>')
		print('You can repeat later or find and download by yourself')
		print(e)


if downloaded:
	print('Downloaded libs: ' + ', '.join(downloaded))
elif not was_error:
	print('Nothing to do')
