init -1000 python:
	
	def sprites__effects_ended():
		for spr in sprites.list:
			if spr.effect is not None:
				return False
		return sprites.overlay.effect is None
	can_exec_next_check_funcs.append(sprites__effects_ended)
	
	def sprites__effects_to_end():
		for spr in sprites.list.copy():
			spr.remove_effect()
		sprites.overlay.remove_effect()
	can_exec_next_skip_funcs.append(sprites__effects_to_end)
	
	def sprites__remove_effect_sprites(effect):
		for spr in effect.removing_sprites:
			if spr in sprites.list:
				sprites.list.remove(spr)
		effect.removing_sprites.clear()
	
	
	def sprites__get_tag_of_image_name(image_name):
		i = image_name.find(' ')
		if i != -1:
			return image_name[:i]
		return image_name
	
	
	
	def sprites__just_effect(effect):
		sprites.overlay.set_effect(effect, hiding = False)
	
	def sprites__set_scene(params, show_at):
		if len(params) == 0:
			sprites.list = []
			sprites.scene = None
			return
		
		old_sprites_hided = sprites.show(params, show_at, True)
		
		if sprites.overlay.effect or (sprites.scene and sprites.scene.effect):
			if not old_sprites_hided:
				effect = sprites.overlay.effect or sprites.scene.effect
				for spr in sprites.list:
					if spr is not sprites.scene:
						effect.removing_sprites.append(spr)
		else:
			sprites.list = [sprites.scene] if sprites.scene else []
	
	
	def sprites__show(params, show_at, is_scene = False):
		if len(params) == 0:
			out_msg('sprites.show', 'List of params is empty')
			return False
		
		params_str = ' '.join(params)
		
		pnames = ('at', 'with', 'behind', 'as')
		
		d = dict()
		while len(params) >= 2 and (params[-2] in pnames):
			pname, pvalue = params[-2:]
			params = params[:-2]
			if pname in d:
				out_msg('sprites.show', 'Param <%s> specified several times', pname)
			else:
				d[pname] = pvalue
		
		for pname in pnames:
			if pname not in d and pname != 'at':
				d[pname] = None
		if d['as'] is None and params:
			d['as'] = params[0]
		
		def eval_param(d, name):
			code = d[name]
			try:
				if code:
					return eval(code)
			except:
				out_msg('sprites.show', 'Failed to eval param <%s>: %s', name, code)
			return None
		
		effect = eval_param(d, 'with')
		
		if len(params) == 0:
			if is_scene:
				if effect is not None:
					for spr in sprites.list:
						spr.set_effect(effect, hiding = True)
				else:
					sprites.list = []
				sprites.scene = None
				return True
			
			out_msg('sprites.show', 'List of params does not contain name of sprite\n%s', params_str)
			return False
		
		image_name = ' '.join(params)
		decl_at = get_image(image_name)
		
		if 'at' in d:
			at = eval_param(d, 'at')
			if at is None:
				at = empty_transform
		else:
			at = None
		
		kwargs = dict(decl_at = decl_at, show_at = show_at, call_str = params_str)
		sprites.show_impl(image_name, d['as'], d['behind'], at, effect, is_scene, **kwargs)
		return False
	
	
	def sprites__show_impl(image_name, tag = None, behind = None, at = None, effect = None, is_scene = False, **kwargs):
		if not has_screen('sprites'):
			show_screen('sprites')
		
		if 'decl_at' in kwargs:
			decl_at = kwargs.pop('decl_at')
		else:
			decl_at = get_image(image_name)
		show_at  = kwargs.pop('show_at', ())
		call_str = kwargs.pop('call_str', image_name)
		if kwargs:
			out_msg('sprites.show_impl', 'Unexpected params: %s', list(kwargs.keys()))
		
		if not tag:
			tag = sprites.get_tag_of_image_name(image_name)
		
		old_sprite = None
		if is_scene:
			index = len(sprites.list)
		else:
			for spr in reversed(sprites.list.copy()):
				if spr.tag != tag:
					continue
				
				index = sprites.list.index(spr)
				
				old_sprite = spr
				old_sprite.remove_effect() # case: 2+ cmd (scene/show/hide) on 1 tag in 1 <with> expression
				
				if not effect:
					if old_sprite in sprites.list: # exactly not removed by .remove_effect?
						sprites.list.remove(old_sprite)
				else:
					index += 1
				break
			else:
				index = len(sprites.list)
		
		if at is None:
			at = () if show_at or old_sprite else center
		
		if type(at) is SpriteAnimation:
			at = at.actions
		
		spr = Sprite(tag, call_str, decl_at, at, show_at, old_sprite)
		spr.sprite_name = image_name
		if is_scene or (old_sprite and old_sprite is sprites.scene):
			sprites.scene = spr
		
		
		if behind is not None:
			index = 0
			for index, tmp_spr in enumerate(sprites.list):
				if tmp_spr.tag == behind:
					break
			else:
				out_msg('sprites.show_impl', 'Sprite <%s> not found', behind)
		
		if sprites.scene in sprites.list:
			index = max(index, sprites.list.index(sprites.scene) + 1)
		
		sprites.list.insert(index, spr)
		spr.set_effect(effect, hiding = False)
		
		persistent._seen_images[image_name] = True
	
	
	def sprites__hide(params):
		if len(params) == 0:
			out_msg('sprites.hide', 'List of params is empty')
			return
		
		tag = params[0]
		
		effect = None
		if len(params) == 3:
			if params[1] != 'with':
				out_msg('sprites.hide', '2 param must be <with>, got <%s>', params[1])
				return
			
			try:
				effect = eval(params[2])
			except:
				out_msg('sprites.hide', 'Failed to eval param <with>: %s', params[2])
		
		elif len(params) != 1:
			out_msg('sprites.hide', 'Expected 1 or 3 params: name ["with" effect]\nGot: <%s>', params)
			return
		
		sprites.hide_impl(tag, effect)
	
	
	def sprites__hide_impl(tag, effect):
		for i, spr in enumerate(sprites.list):
			if spr.tag == tag:
				if effect is not None:
					spr.set_effect(effect, hiding = True)
				else:
					sprites.list.pop(i)
				break
		else:
			out_msg('sprites.hide_impl', 'Sprite <%s> not found', tag)
	
	
	def sprites__get_draw_data():
		res = []
		def add_sprite(res, sprite):
			for spr in sprite.get_all_sprites():
				if (spr.res_image or spr.image) and spr.real_alpha > 0:
					res.append(spr)
		
		sprites.overlay.update()
		for sprite in sprites.list.copy():
			sprite.update()
		
		for sprite in sprites.list:
			add_sprite(res, sprite)
		add_sprite(res, sprites.overlay)
		
		return res
	
	
	build_object('sprites')
	
	sprites.list = []
	
	sprites.xsize   = sprites.ysize   = 1.0
	sprites.xanchor = sprites.yanchor = 0.0
	sprites.xpos    = sprites.ypos    = 0.0
	
	sprites.overlay = Sprite('_overlay', '_overlay', (), (), (), None)
	sprites.overlay.xsize,      sprites.overlay.ysize      = 1.0, 1.0
	sprites.overlay.real_xsize, sprites.overlay.real_ysize = 1.0, 1.0
	
	sprites.scene = None


screen sprites:
	zorder -3
	
	$ sprites.images = sprites.get_draw_data()
	
	xzoom get_stage_width()  / (config.width  or get_stage_width())
	yzoom get_stage_height() / (config.height or get_stage_height())
	
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
