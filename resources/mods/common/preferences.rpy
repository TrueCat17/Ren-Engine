init python:
	preferences.tabs.append('RPG')
	
	preferences.content['RPG'] = [
		[['bool', '["Usual moving - run"!t]', GetSetAttr('config.default_moving_is_run'), ToggleVariable('config.default_moving_is_run')]],
	]
