init python:
	
	register_location("clubs", "images/locations/clubs/", False, 1152, 1664)
	register_place(   "clubs", "admin", 1132, 1190, 20, 130)
	register_place(   "clubs", "before_clubs", 340, 1190, 50, 130)
	register_place(   "clubs", "before_porch", 447, 1280, 2, 2)
	register_place(   "clubs", "cluster", 200, 1300, 2, 2)
	register_place(   "clubs", "mus_club_column_pos-1", 773, 400, 2, 2)
	register_place(   "clubs", "mus_club_column_pos-2", 836, 400, 2, 2)
	register_place(   "clubs", "mus_club_column_pos-3", 899, 400, 2, 2)
	register_place(   "clubs", "mus_club_column_pos-4", 962, 400, 2, 2)
	register_place(   "clubs", "mus_club_column_pos-5", 1025, 400, 2, 2)
	register_place(   "clubs", "mus_club_rails_pos-1", 804, 400, 2, 2)
	register_place(   "clubs", "mus_club_rails_pos-2", 867, 400, 2, 2)
	register_place(   "clubs", "mus_club_rails_pos-3", 930, 400, 2, 2)
	register_place(   "clubs", "mus_club_rails_pos-4", 1056, 400, 2, 2)
	register_place(   "clubs", "porch", 447, 1210, 2, 2)
	register_place(   "clubs", "porch_cleft", 425, 1210, 2, 2)
	register_place(   "clubs", "porch_left", 410, 1210, 2, 2)
	register_place(   "clubs", "enter", 0, 1190, 40, 130, to=["left", "enter", "clubs", to_back])
	register_place(   "clubs", "radio_club", 390, 1120, 120, 30, to=["up", "radio_club", "clubs"])
	
	register_location("enter", "images/locations/enter/", False, 960, 992)
	register_place(   "enter", "before_gates", 410, 290, 156, 50)
	register_place(   "enter", "behind_gates", 410, 250, 156, 10)
	register_place(   "enter", "gate_left_pos", 449, 279, 2, 2)
	register_place(   "enter", "gate_right_pos", 509, 279, 2, 2)
	register_place(   "enter", "ikarus_pos", 293, 590, 2, 2)
	register_place(   "enter", "clubs", 410, 0, 156, 40, to=["up", "clubs", "enter", to_right])
	register_place(   "enter", "ikarus", 400, 590, 40, 40, to=["up", "ikarus", "enter"])
	register_place(   "enter", "left", 0, 570, 40, 135, to=["left", "enter", "right"])
	register_place(   "enter", "right", 920, 565, 40, 145, to=["right", "enter", "left"])
	
	register_location("ikarus", "images/locations/ikarus/", True, 478, 154)
	register_place(   "ikarus", "before_sit_place", 409, 76, 2, 2)
	register_place(   "ikarus", "before_sit_place-2", 410, 100, 10, 10)
	register_place(   "ikarus", "sit_place", 397, 71, 2, 2)
	register_place(   "ikarus", "enter", 407, 124, 30, 30, to=["down", "enter", "ikarus"])
	
	register_location("radio_club", "images/locations/radio_club/", True, 310, 247)
	register_place(   "radio_club", "before_computer", 185, 130, 2, 2)
	register_place(   "radio_club", "clubs", 215, 205, 60, 40, to=["down", "clubs", "radio_club"])
	
	
	
	rpg_locations["clubs"].x, rpg_locations["clubs"].y = 433, 161
	rpg_locations["enter"].x, rpg_locations["enter"].y = 312, 276
	rpg_locations["ikarus"].x, rpg_locations["ikarus"].y = 173, 301
	rpg_locations["radio_club"].x, rpg_locations["radio_club"].y = 390, 54

