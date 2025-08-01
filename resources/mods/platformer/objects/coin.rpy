init -20 python:
	
	start_score = score = 0
	extra_text_vars.append(('score', 'Score: ', '#FB0'))
	
	class Pltf_Coin(Pltf_Empty):
		can_vertical = False
		can_horizontal = False
		
		is_dynamic = True
		
		image = im.composite((cell_size, cell_size),
		                     (cell_size / 4, cell_size / 4), im.circle('#FF0', cell_size / 2, cell_size / 2))
		coins = []
		
		@staticmethod
		def start():
			Pltf_Coin.coins = []
			
			global score, start_score
			if prev_level == cur_level: # restart current level, not next
				score = start_score
			else:
				start_score = score
		
		@staticmethod
		def init(x, y):
			Pltf_Coin.coins.append((x, y))
		
		@staticmethod
		def physics(cell, pixel, dx, dy):
			if cell in Pltf_Coin.coins:
				Pltf_Coin.coins.remove(cell)
				level_dynamic_objects.remove((Pltf_Coin, cell[0], cell[1]))
				
				global score
				score += 1
			
			x, y = pixel
			return x + dx, y + dy
