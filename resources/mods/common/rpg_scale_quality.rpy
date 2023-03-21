init -1000000 python:
	# default (params.conf) scale_quality - 1, but pixel rpg need 0
	if 'rpg' in get_current_mod():
		set_scale_quality('0')
