init -1000 python:
	
	def sprites__effects_ended():
		for spr in sprites.list:
			if spr.effect is not None:
				return False
		return sprites.screen.effect is None
	can_exec_next_check_funcs.append(sprites__effects_ended)
	
	def sprites__effects_to_end():
		sprites.remove_hiding()
		for spr in sprites.list.copy():
			spr.remove_effect()
		sprites.screen.remove_effect()
	can_exec_next_skip_funcs.append(sprites__effects_to_end)
	
	def sprites__remove_hiding():
		for spr in sprites.list.copy():
			if spr.hiding:
				sprites.list.remove(spr)
	
	
	def sprites__get_tag_of_image_name(image_name):
		i = image_name.find(' ')
		if i != -1:
			return image_name[:i]
		return image_name
	
	
	
	def sprites__set_scene(params, show_at):
		if len(params) == 0:
			sprites.list = []
			sprites.scene = None
			return
		
		old_sprites_hided = sprites.show(params, show_at, True)
		
		if sprites.screen.effect or (sprites.scene and sprites.scene.effect):
			if not old_sprites_hided:
				for spr in sprites.list:
					if spr is not sprites.scene:
						spr.hiding = True
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
						spr.hiding = True
						spr.set_effect(effect)
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
		show_at = kwargs.pop('show_at', ())
		call_str = kwargs.pop('call_str', image_name)
		if kwargs:
			out_msg('sprites.show_impl', 'Unexpected params: %s', list(kwargs.keys()))
		
		if not tag:
			tag = sprites.get_tag_of_image_name(image_name)
		
		old_sprite = None
		if is_scene:
			index = len(sprites.list)
		else:
			index = -1
			for index, spr in enumerate(sprites.list):
				if spr.tag == tag:
					if effect:
						old_sprite = spr
						spr.hiding = True
					else:
						sprites.list.pop(index)
						index -= 1
					break
			index += 1
		
		if at is None:
			if old_sprite:
				at = old_sprite.at
				if not at.actions and not show_at:
					at = center
			else:
				at = () if show_at else center
		
		if type(at) is SpriteAnimation:
			at = at.actions
		
		spr = Sprite(tag, call_str, decl_at, at, show_at, old_sprite if effect else None)
		spr.sprite_name = image_name
		if is_scene or old_sprite is sprites.scene:
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
		spr.set_effect(effect)
		
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
					spr.hiding = True
					spr.set_effect(effect)
				else:
					sprites.list.pop(i)
				break
		else:
			out_msg('sprites.hide_impl', 'Sprite <%s> not found', tag)
	
	
	def sprites__get_draw_data():
		res = []
		for sprite in sprites.list + [sprites.screen]: # new list, because in update something can be removed from sprites.list
			sprite.update()
			
			for spr in sprite.get_all_sprites():
				tmp_image = spr.res_image or spr.image
				if not tmp_image or spr.real_alpha <= 0: continue
				
				tmp = SimpleObject()
				tmp.image  =  tmp_image
				tmp.size   = (spr.real_xsize, spr.real_ysize)
				tmp.anchor = (spr.real_xanchor, spr.real_yanchor)
				tmp.pos    = (spr.real_xpos + spr.real_xanchor, spr.real_ypos + spr.real_yanchor) # real_pos already taked anchor, cancel it
				tmp.crop   = (spr.xcrop, spr.ycrop, spr.xsizecrop, spr.ysizecrop)
				tmp.alpha  =  spr.real_alpha
				tmp.rotate =  spr.real_rotate
				res.append(tmp)
		return res
	
	
	build_object('sprites')
	
	sprites.list = []
	
	sprites.screen = Sprite('screen', 'screen', (), (), (), None)
	sprites.screen.xsize,      sprites.screen.ysize      = 1.0, 1.0
	sprites.screen.real_xsize, sprites.screen.real_ysize = 1.0, 1.0
	
	sprites.scene = None


screen sprites:
	zorder -3
	
	$ sprites.images = sprites.get_draw_data()
	
	xzoom get_stage_width()  / (config.width  or get_stage_width())
	yzoom get_stage_height() / (config.height or get_stage_height())
	
	pos    (sprites.screen.xpos,       sprites.screen.ypos)
	anchor (sprites.screen.xanchor,    sprites.screen.yanchor)
	size   (sprites.screen.real_ysize, sprites.screen.real_xsize)
	
	for tmp in sprites.images:
		image tmp.image:
			pos    tmp.pos
			anchor tmp.anchor
			size   tmp.size
			crop   tmp.crop
			alpha  tmp.alpha
			rotate tmp.rotate
