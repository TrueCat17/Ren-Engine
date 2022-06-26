init python:
	preferences.tabs.append('RPG')
	
	preferences.content['RPG'] = [
		[['bool', '["Usual moving - run"!t]', Function(getattr, config, 'default_moving_is_run'), ToggleDict(config, 'default_moving_is_run')]],
	]
