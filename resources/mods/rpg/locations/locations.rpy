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
	register_place(   "clubs", "enter"         , 20, 1190, 20, 130, to_right)
	register_exit(    "clubs", "enter", "clubs", 0, 1190, 20, 130)
	register_place(   "clubs", "radio_club"         , 390, 1135, 120, 15)
	register_exit(    "clubs", "radio_club", "clubs", 390, 1120, 120, 15)
	
	register_location("enter", "images/locations/enter/", False, 960, 992)
	register_place(   "enter", "before_gates", 415, 290, 130, 50)
	register_place(   "enter", "behind_gates", 415, 250, 130, 10)
	register_place(   "enter", "gate_left_pos", 449, 279, 2, 2)
	register_place(   "enter", "gate_right_pos", 509, 279, 2, 2)
	register_place(   "enter", "ikarus_pos", 293, 590, 2, 2)
	register_place(   "enter", "left_exit", 20, 570, 20, 135)
	register_place(   "enter", "right_exit", 920, 565, 20, 145)
	register_place(   "enter", "clubs"         , 410, 20, 140, 20, to_back)
	register_exit(    "enter", "clubs", "enter", 410, 0, 140, 20)
	register_place(   "enter", "ikarus"         , 400, 610, 40, 20)
	register_exit(    "enter", "ikarus", "enter", 400, 590, 40, 20)
	register_exit("enter", "enter", "right_exit", 0, 570, 20, 135)
	register_exit("enter", "enter", "left_exit", 940, 565, 20, 145)
	
	register_location("ikarus", "images/locations/ikarus/", True, 478, 154)
	register_place(   "ikarus", "before_sit_place", 409, 76, 2, 2)
	register_place(   "ikarus", "before_sit_place-2", 410, 100, 10, 10)
	register_place(   "ikarus", "sit_place", 397, 71, 2, 2)
	register_place(   "ikarus", "enter"          , 407, 124, 30, 15)
	register_exit(    "ikarus", "enter", "ikarus", 407, 139, 30, 15)
	
	register_location("radio_club", "images/locations/radio_club/", True, 310, 247)
	register_place(   "radio_club", "before_computer", 185, 130, 2, 2)
	register_place(   "radio_club", "clubs"              , 215, 207, 60, 20)
	register_exit(    "radio_club", "clubs", "radio_club", 215, 227, 60, 20)
	
	
	
	locations["clubs"].x, locations["clubs"].y = 433, 161
	locations["enter"].x, locations["enter"].y = 312, 276
	locations["ikarus"].x, locations["ikarus"].y = 173, 301
	locations["radio_club"].x, locations["radio_club"].y = 390, 54

