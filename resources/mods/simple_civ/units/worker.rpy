init -1 python:
	
	class Worker(Object):
		image = sc_map.image_dir + 'worker.png'
		symbol_offset = 0.75
		
		def __init__(self, player, x, y):
			Object.__init__(self)
			
			self.player = player
			self.x, self.y = x, y
			
			self.max_work_dist = 4
			self.work_cell = None
		
		def get_symbol(self):
			return 'w' if self.work_cell else ''
		
		def get_menu(self):
			res = []
			
			if self.work_cell:
				res.append(MenuItem('Cancel work', self.cancel_work, 'n'))
			res.append(MenuItem('Select work', control.pick, 'm'))
			
			return res
		
		def cancel_work(self):
			self.work_cell.workers.remove(self)
			self.work_cell = None
			
			control.select_cell(self.x, self.y) # update menu
			self.player.calc_changing_resources()
		
		def selected_cell(self, cell_x, cell_y):
			dist = abs(self.x - cell_x) + abs(self.y - cell_y)
			if dist > self.max_work_dist:
				info.set_msg('Too far')
				return
			
			cell = sc_map.map[cell_y][cell_x]
			if cell.building in (None, 'district', 'storage'):
				info.set_msg('No work')
				return
			if len(cell.workers) == cell.building_level:
				info.set_msg('No free places')
				return
			
			if self.work_cell:
				self.work_cell.workers.remove(self)
			
			self.work_cell = cell
			cell.workers.append(self)
			
			control.select_cell(self.x, self.y) # update menu
			
			self.player.calc_changing_resources()
