import sys
bits = '64' if sys.maxsize > 2**32 else '32'

def load(ver):
	path = '../Ren-Engine/py_libs/linux.so/v' + ver + '_x' + bits + '/_ssl.so'
	import imp
	imp.load_module('_ssl', None, path, ('', 'rb', imp.C_EXTENSION))


try:
	load('3') # linked with new lib: libssl.so.3
except:
	load('1') # linked with old lib: libssl.so.1.1
finally:
	del sys, bits, load
