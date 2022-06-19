init python:
	settings.tabs.append('RPG')
	
	settings.content['RPG'] = [
		[['bool', '["Usual moving - run"!t]', Function(getattr, config, 'shift_is_run'), ToggleDict(config, 'shift_is_run')]],
	]
