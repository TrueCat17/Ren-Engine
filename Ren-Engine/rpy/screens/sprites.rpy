init -1000 python:
	
	def sprites__effects_ended():
		for spr in sprites.list:
			if spr.effect is not None:
				return False
		return sprites.screen.effect is None
	can_exec_next_check_funcs.append(sprites__effects_ended)
	
	def sprites__effects_to_end():
		for spr in sprites.list:
			spr.remove_effect()
		sprites.screen.remove_effect()
		sprites.remove_hiding()
	can_exec_next_skip_funcs.append(sprites__effects_to_end)
	
	def sprites__remove_hiding():
		for spr in list(sprites.list): # copy
			if spr.hiding:
				sprites.list.remove(spr)
			else:
				if spr.effect is not None:
					spr.effect.for_not_hiding()
	
	
	def sprites__get_tag_of_image_name(image_name):
		i = image_name.find(' ')
		if i != -1:
			return image_name[:i]
		return image_name
	
	
	
	def sprites__set_scene(params, show_at):
		if len(params):
			sprites.show(params, show_at, True)
			
			if sprites.screen.effect or sprites.scene.effect:
				for spr in sprites.list:
					if spr is not sprites.scene:
						spr.old_data, spr.new_data = spr.new_data, None
						spr.hiding = True
			else:
				sprites.list = [sprites.scene]
		else:
			sprites.list = []
			sprites.scene = None
	
	
	def sprites__show(params, show_at, is_scene = False):
		if len(params) == 0:
			out_msg('sprites.show', 'List of params is empty')
			return
		
		params_str = ' '.join(params)
		
		pnames = ('at', 'with', 'behind', 'as')
		
		d = dict()
		while len(params) >= 2 and (params[-2] in pnames):
			pname, pvalue = params[-2], params[-1]
			params = params[:-2]
			if pname in d:
				out_msg('sprites.show', 'Param <%s> specified several times' % (pname, ))
			else:
				d[pname] = pvalue
		if len(params) == 0 and not is_scene:
			out_msg('sprites.show', 'List of params does not contain name of sprite\n' + params_str)
			return
		
		for pname in pnames:
			if pname not in d:
				d[pname] = None
		if d['as'] is None:
			d['as'] = params[0] if params else '<empty>'
		
		def eval_param(d, name):
			code = d[name]
			try:
				if code:
					return eval(code)
			except:
				out_msg('sprites.show', 'Failed on eval param <%s>: %s' % (name, code))
			return None
		
		if params:
			image_name = ' '.join(params)
			decl_at = get_image(image_name)
		else:
			image_name = None
			decl_at = ()
			params_str = '<empty> ' + params_str
		
		at     = eval_param(d, 'at')
		effect = eval_param(d, 'with')
		
		kwargs = dict(decl_at = decl_at, show_at = show_at, call_str = params_str)
		sprites.show_impl(image_name, d['as'], d['behind'], at, effect, is_scene, **kwargs)
	
	
	def sprites__show_impl(image_name, tag = None, behind = None, at = None, effect = None, is_scene = False, **kwargs):
		if not has_screen('sprites'):
			show_screen('sprites')
		
		if 'decl_at' in kwargs:
			decl_at = kwargs.pop('decl_at')
		else:
			decl_at = get_image(image_name)
		show_at = kwargs.pop('show_at', ())
		call_str = kwargs.pop('call_str', image_name or '<empty>')
		if kwargs:
			out_msg('sprites.show_impl', 'Unexpected params: %s' % (list(kwargs.keys()), ))
		
		if not tag:
			tag = sprites.get_tag_of_image_name(image_name)
		
		old_sprite = None
		if is_scene:
			index = len(sprites.list)
		else:
			index = 0
			while index < len(sprites.list):
				spr = sprites.list[index]
				if spr.tag == tag:
					old_sprite = spr
					sprites.list.pop(index)
					break
				index += 1
		
		if at is None:
			data = old_sprite and (old_sprite.new_data or old_sprite.old_data)
			if data:
				at = data.at
				if not at.actions and not show_at:
					at = center
			else:
				at = () if show_at else center
		
		if type(at) is SpriteAnimation:
			at = at.actions
		
		spr = Sprite(decl_at, at, show_at, old_sprite if effect else None)
		spr.sprite_name = image_name or '<empty>'
		spr.tag = tag
		spr.call_str = call_str
		if is_scene or old_sprite is sprites.scene:
			sprites.scene = spr
		spr.set_effect(effect)
		
		
		if behind is not None:
			index = 0
			while index < len(sprites.list):
				if sprites.list[index].tag == behind:
					break
				index += 1
			else:
				out_msg('sprites.show', 'Sprite <%s> not found' % (behind, ))
		
		if sprites.scene in sprites.list:
			index = max(index, sprites.list.index(sprites.scene) + 1)
		
		sprites.list.insert(index, spr)
		persistent._seen_images[image_name] = True
	
	
	def sprites__hide(params):
		if len(params) == 0:
			out_msg('sprites.hide', 'List of params is empty')
			return
		
		tag = params[0]
		
		effect = None
		if len(params) == 3:
			if params[1] != 'with':
				out_msg('sprites.hide', '2 param must be <with>, got <%s>' % (params[1], ))
				return
			
			try:
				effect = eval(params[2])
			except:
				out_msg('sprites.hide', 'Failed on eval param <with>: %s' % (params[2], ))
		
		elif len(params) != 1:
			out_msg('sprites.hide', 'Expected 1 or 3 params: name ["with" effect]\n' + 'Got: <%s>' % (params, ))
			return
		
		sprites.hide_impl(tag, effect)
	
	
	def sprites__hide_impl(tag, effect):
		for i in range(len(sprites.list)):
			spr = sprites.list[i]
			if spr.tag == tag:
				if effect is not None:
					spr.old_data, spr.new_data = spr.new_data, None
					
					spr.set_effect(effect)
					spr.hiding = True
				else:
					sprites.list.pop(i)
				break
		else:
			out_msg('sprites.hide', 'Sprite <%s> not found' % (tag, ))
	
	
	def sprites__get_datas():
		res = []
		for spr in sprites.list + [sprites.screen]: # new list, because in update something can be removed from sprites.list
			spr.update()
			
			for data in spr.get_all_data():
				tmp_image = data.res_image or data.image
				if not tmp_image or data.real_alpha <= 0: continue
				
				tmp = SimpleObject()
				tmp.image  =  tmp_image
				tmp.size   = (data.real_xsize, data.real_ysize)
				tmp.anchor = (data.real_xanchor, data.real_yanchor)
				tmp.pos    = (data.real_xpos + data.real_xanchor, data.real_ypos + data.real_yanchor) # real_pos already taked anchor, cancel it
				tmp.crop   = (data.xcrop, data.ycrop, data.xsizecrop, data.ysizecrop)
				tmp.alpha  =  data.real_alpha
				tmp.rotate =  data.real_rotate
				res.append(tmp)
		return res
	
	
	build_object('sprites')
	
	sprites.list = []
	
	sprites.screen = Sprite([], [], [], None)
	sprites.screen.new_data.xsize, sprites.screen.new_data.ysize = 1.0, 1.0
	sprites.screen.new_data.real_xsize, sprites.screen.new_data.real_ysize = 1.0, 1.0
	sprites.screen.call_str = 'screen'
	
	sprites.scene = None


screen sprites:
	zorder -3
	
	$ sprites.images = sprites.get_datas()
	
	xzoom get_stage_width()  / (config.width  or get_stage_width())
	yzoom get_stage_height() / (config.height or get_stage_height())
	
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
