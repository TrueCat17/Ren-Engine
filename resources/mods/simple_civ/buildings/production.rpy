init python:
	
	simple_production = ['farm', 'sawmill', 'career']
	
	building_production = {
		'college': {
			'from': {},
			'to': {
				'science': 4,
			}
		},
		
		'cement factory': {
			'from': {
				'coal': 40,
				'stone': 30,
			},
			'to': {
				'cement': 25,
			}
		},
		
		'metal factory': {
			'from': {
				'coal': 45,
				'metal': 30,
			},
			'to': {
				'steel': 15,
			}
		},
	}
