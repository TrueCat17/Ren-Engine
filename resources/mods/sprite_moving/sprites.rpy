init python:
	sprite_btn_bg = im.rect('#000')

# copy-paste from Ren-Engine/rpy/screens/sprites.rpy
# changed zoom (panel props), add mouse processing
screen sprites:
	zorder -3
	
	$ sprites.images = sprites.get_draw_data()
	null:
		xzoom (get_stage_width()  if panels.hide else panels.sprites_xsize) / (config.width  or get_stage_width())
		yzoom (get_stage_height() if panels.hide else panels.sprites_ysize) / (config.height or get_stage_height())
		
		size   (sprites.xsize,   sprites.ysize)
		pos    (sprites.xpos,    sprites.ypos)
		anchor (sprites.xanchor, sprites.yanchor)
		
		for spr in sprites.images:
			image (spr.res_image or spr.image):
				size   (spr.real_xsize, spr.real_ysize)
				anchor (spr.real_xanchor, spr.real_yanchor)
				pos    (spr.real_xpos + spr.real_xanchor, spr.real_ypos + spr.real_yanchor) # real_pos already taked anchor, cancel it
				crop   (spr.xcrop, spr.ycrop, spr.xsizecrop, spr.ysizecrop)
				alpha   spr.real_alpha
				rotate  spr.real_rotate
	
	button:
		ground sprite_btn_bg
		hover  sprite_btn_bg
		size  1.0
		alpha 0.01
		mouse False
		action props.on_sprite_click
	
	$ props.update_mouse()
