init python:
	hotkeys.disable_key_on_screens['ESCAPE'].append('save')

screen save:
	zorder 10000
	modal  True
	save   False
	
	use slots('save')
