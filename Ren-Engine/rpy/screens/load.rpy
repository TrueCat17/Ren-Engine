init python:
	hotkeys.disable_key_on_screens['ESCAPE'].append('load')

screen load:
	zorder 10001
	modal  True
	
	use slots('load')

