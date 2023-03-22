init -1000000 python:
	# default (params.conf) scale_quality - 0, but some mods need 1
	if get_current_mod() in ('vn', 'sprite_moving'):
		set_scale_quality('1')
