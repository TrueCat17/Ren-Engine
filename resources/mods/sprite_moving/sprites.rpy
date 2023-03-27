# copy-paste from Ren-Engine/rpy/screens/sprites.rpy
# changed zoom (panel props), add mouse processing
screen sprites:
	zorder -3
	
	$ props.update_mouse()
	$ sprites.images = sprites.get_datas()
	
	xzoom (get_stage_width()  if panel.hide else panel.sprites_xsize) / float(config.width  or get_stage_width())
	yzoom (get_stage_height() if panel.hide else panel.sprites_ysize) / float(config.height or get_stage_height())
	
	pos    (sprites.screen.new_data.xpos,       sprites.screen.new_data.ypos)
	anchor (sprites.screen.new_data.xanchor,    sprites.screen.new_data.yanchor)
	size   (sprites.screen.new_data.real_ysize, sprites.screen.new_data.real_xsize)
	
	for tmp in sprites.images:
		image tmp.image:
			pos    tmp.pos
			anchor tmp.anchor
			size   tmp.size
			crop   tmp.crop
			alpha  tmp.alpha
			rotate tmp.rotate
	
	button:
		ground im.rect('#000')
		hover  im.rect('#000')
		size 1.0
		alpha 0.01
		mouse False
		action props.on_sprite_click
