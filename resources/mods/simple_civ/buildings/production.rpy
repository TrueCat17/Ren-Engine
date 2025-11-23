init python:
	
	sc_buildings.simple = ['farm', 'sawmill', 'career']
	
	sc_buildings.takes = {
		'cement factory': (
			('stone', 30),
			('coal',  40),
		),
		
		'metal factory': (
			('coal',  45),
			('metal', 30),
		),
		
		'barracks': (
			('wood',  8),
			('stone', 8),
		),
	}
	
	sc_buildings.makes = {
		'cement factory': ('cement', 25),
		'metal factory': ('steel', 15),
		'college': ('science', 4),
	}
