init 100 python:
	set_fps(30)
	quick_menu = False
	config.has_autosave = False
	
	show_screen('doom')
	show_screen('desc')
	
	
	class DoomEngine:
		def __init__(self):
			self.wad_path = mod_dir + '/wad/doom1.wad.zlib'
			self.set_level('1')
		
		def set_level(self, num):
			global player
			
			self.wad_data = WADData(self, map_name='E1M' + str(num))
			player = self.player = Player(self)
			self.bsp = BSP(self)
			self.seg_handler = SegHandler(self)
			self.view_renderer = ViewRenderer(self)
		
		def update(self):
			self.player.update()
			self.seg_handler.update()
			self.bsp.update()
	
	doom = DoomEngine()
