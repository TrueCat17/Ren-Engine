init -1000000 python:
	# default (params.conf) scale_quality - 0, but some mods need 1
	if get_current_mod() in ('main_menu', 'tutorial', 'sprite_moving'):
		set_scale_quality('1')
