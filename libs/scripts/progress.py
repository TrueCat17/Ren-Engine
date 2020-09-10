#!/usr/bin/env -S python3 -B

from config import *

file_name = scripts_path + '/progress_list.txt'


def read_progress():
	res = []
	
	f = open(file_name, 'rb')
	for s in f:
		lib, progress = str(s, 'utf8').strip().split(' ')
		res.append((lib, progress))
	
	return res

def write_progress(progress_list):
	f = open(file_name, 'wb')
	for lib, progress in progress_list:
		f.write(bytes(lib + ' ' + progress + '\n', 'utf8'))


if __name__ == '__main__':
	print('Choose need actions for all libs: ')
	print('1. Download and build')
	print('2. Build')
	
	while True:
		action = input('Input number of action: ')
		if action in ('1', '2'):
			break
		print('Expected 1 or 2')
	
	progress = 'start' if action == '1' else 'downloaded'
	
	progress_list = []
	for lib in libs:
		progress_list.append([lib, progress])
	write_progress(progress_list)
