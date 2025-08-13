init 1 python:
	
	register_location_object('ikarus', 'images/locations/enter/objects/', 'ikarus_main', 'ikarus_free')
	
	register_location_object('gate_left', 'images/locations/enter/objects/', 'gate_left_main', 'gate_free')
	register_location_object('gate_right', 'images/locations/enter/objects/', 'gate_right_main', 'gate_free')
	
	register_location_object_animation('gate_right', 'open',
		'images/locations/enter/objects/anim/', 'gate_right_main', 'gate_right_free',
		0, 34,
		3, 2, 2,
		-1
	)
	register_location_object_animation('gate_right', 'opening',
		'images/locations/enter/objects/anim/', 'gate_right_main', 'gate_right_free',
		0, 34,
		3, 0, 2
	)
	register_location_object_animation('gate_right', 'closing',
		'images/locations/enter/objects/anim/', 'gate_right_main', 'gate_right_free',
		0, 34,
		3, 2, 0
	)
	
	
	register_location_object('key', 'images/locations/objects/', 'key', None, 10)
	
	
	register_location_object('bench_right',    'images/locations/clubs/objects/', 'bench_right', None)
	register_location_object('mus_club_bench', 'images/locations/clubs/objects/', 'mus_club_bench', None)
	register_location_object('mus_club_rails',  'images/locations/clubs/objects/', 'mus_club_rails', None)
	register_location_object('mus_club_column', 'images/locations/clubs/objects/', 'mus_club_column', None)
	
	register_location_object('radio_club_cupboards', 'images/locations/radio_club/objects/', 'cupboards', None)
	register_location_object('radio_club_table',     'images/locations/radio_club/objects/', 'table', None)
	register_location_object('radio_club_tabouret',  'images/locations/radio_club/objects/', 'tabouret', 'tabouret_free')
	
	add_location_object("clubs", "bench_right_pos", "bench_right")
	add_location_object("clubs", "mus_club_bench_pos", "mus_club_bench")
	add_location_object("clubs", "mus_club_column_pos-1", "mus_club_column")
	add_location_object("clubs", "mus_club_column_pos-2", "mus_club_column")
	add_location_object("clubs", "mus_club_column_pos-3", "mus_club_column")
	add_location_object("clubs", "mus_club_column_pos-4", "mus_club_column")
	add_location_object("clubs", "mus_club_column_pos-5", "mus_club_column")
	add_location_object("clubs", "mus_club_rails_pos-1", "mus_club_rails")
	add_location_object("clubs", "mus_club_rails_pos-2", "mus_club_rails")
	add_location_object("clubs", "mus_club_rails_pos-3", "mus_club_rails")
	add_location_object("clubs", "mus_club_rails_pos-4", "mus_club_rails")
	add_location_object("enter", "gate_left_pos", "gate_left")
	add_location_object("enter", "gate_right_pos", "gate_right")
	add_location_object("enter", "ikarus_pos", "ikarus")
	add_location_object("radio_club", "radio_club_cupboards_pos", "radio_club_cupboards")
	add_location_object("radio_club", "radio_club_table_pos", "radio_club_table")
	add_location_object("radio_club", "radio_club_tabouret_pos-1", "radio_club_tabouret")
	add_location_object("radio_club", "radio_club_tabouret_pos-2", "radio_club_tabouret")
	add_location_object("radio_club", "radio_club_tabouret_pos-3", "radio_club_tabouret")
