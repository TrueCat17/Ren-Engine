init -1000 python:
	for f in 'sin cos tan radians degrees atan dist isclose ceil atan2'.split(' '):
		globals()['math_' + f] = getattr(math, f)

init 100 python:
	set_fps(30)
	quick_menu = False
	config.has_autosave = False
	
	start_screens.remove('dialogue_box')
	show_screen('doom')
	show_screen('desc')
	
	
	class DoomEngine:
		def __init__(self):
			self.wad_path = mod_dir + '/wad/doom1.wad.zlib'
			self.set_level('1')
		
		def set_level(self, num):
			global player
			
			self.wad_data = WADData(self, map_name = 'E1M' + str(num))
			player = Player(self)
			self.bsp = BSP(self)
			self.seg_handler = SegHandler(self)
			self.view_renderer = ViewRenderer(self)
		
		def update(self):
			player.update()
			self.seg_handler.update()
			self.bsp.update()
	
	doom = DoomEngine()
