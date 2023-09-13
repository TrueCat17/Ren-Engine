init python:
	class Player:
		def __init__(self, engine):
			self.engine = engine
			self.thing = engine.wad_data.things[0]
			self.pos = self.thing.pos
			self.angle = self.thing.angle
			self.height = PLAYER_HEIGHT
			self.floor_height = 0
			self.z_vel = 0
		
		def update(self):
			self.get_height()
		
		def get_height(self):
			# self.height = self.engine.bsp.get_sub_sector_height() + PLAYER_HEIGHT
			self.floor_height = self.engine.bsp.get_sub_sector_height()
			
			if self.height < self.floor_height + PLAYER_HEIGHT:
				self.height += 0.4 * (self.floor_height + PLAYER_HEIGHT - self.height)
				self.z_vel = 0
			else:
				self.z_vel -= 0.9
				self.height += max(-15.0, self.z_vel)
