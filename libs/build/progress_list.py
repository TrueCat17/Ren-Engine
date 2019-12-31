#!/bin/python2

from config import *

file_name = build + '/progress_list.txt'

def read_progress():
	res = []
	
	f = open(file_name, 'rb')
	for s in f:
		lib, progress = s.strip().split(' ')
		res.append([lib, progress])
	
	return res

def write_progress(progress_list):
	f = open(file_name, 'wb')
	for lib, progress in progress_list:
		f.write(lib + ' ' + progress + '\n')


if __name__ == '__main__':
	progress_list = []
	
	from config import libs
	for lib in libs:
		progress_list.append([lib, 'start'])
	
	write_progress(progress_list)
