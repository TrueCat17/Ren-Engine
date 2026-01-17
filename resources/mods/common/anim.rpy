init -10 python:
	config.setdefault('animations', True)
	
	preferences.content['Other'].append(
		[['bool', '["Animations"!t]', GetSetAttr('config.animations'), ToggleVariable('config.animations')]]
	)
	
	
	planet_fps = 30
	planet_pause_time = 1 / planet_fps
	
	monitor_fps = 14
	monitor_pause_time = 1 / monitor_fps
	
	anim_params = {
		'planet': dict(
			xcount = 10,
			ycount = 6,
			pos = (1077, 624),
		),
		'monitor1': dict(
			xcount = 8,
			ycount = 5,
			pos = (1445, 580),
		),
		'monitor2': dict(
			xcount = 5,
			ycount = 8,
			pos = (1772, 540),
		),
		'monitor3': dict(
			xcount = 10,
			ycount = 4,
			pos = (1422, 824),
		),
		'monitor4': dict(
			xcount = 10,
			ycount = 4,
			pos = (1466, 776),
		),
		'monitor5': dict(
			xcount = 5,
			ycount = 8,
			pos = (1737, 839),
		),
		'monitor6': dict(
			xcount = 8,
			ycount = 5,
			pos = (2019, 792),
		),
	}
	
	def init_anim(name):
		params = anim_params[name]
		
		params['image'] = 'images/anim/' + name + ('.webp' if name != 'planet' else '.png')
		params['first_frame'] = params['image'].replace('anim', 'anim/first_frames')
		
		if config.animations:
			w, h = get_image_size(params['image'])
			params['size'] = (w // params['xcount'], h // params['ycount'])
		else:
			params['size'] = get_image_size(params['first_frame'])
		
		params['frame'] = random.randint(0, 100)
	
	init_anim('planet')
	for i in '123456':
		init_anim('monitor' + i)
	
	
	def get_anim_image(name):
		params = anim_params[name]
		animations = params['animations'] = config.animations
		return params['image' if animations else 'first_frame']
	
	def update_anim_and_get_crop(name):
		params = anim_params[name]
		
		if not params['animations']:
			return (0, 0, 1.0, 1.0)
		
		frames = params['xcount'] * params['ycount']
		frame = params['frame'] = (params['frame'] + 1) % frames
		xcount = params['xcount']
		
		xframe = frame % xcount
		yframe = frame // xcount
		w, h = params['size']
		
		return (xframe * w, yframe * h, w, h)

init -10:
	image anim planet_light = "images/anim/planet_light.png":
		pos (1133, 937)
	
	image anim planet:
		pos  anim_params['planet']['pos']
		size anim_params['planet']['size']
		block:
			get_anim_image('planet')
			crop update_anim_and_get_crop('planet')
			planet_pause_time
			repeat
	
	
	image anim monitor1:
		pos  anim_params['monitor1']['pos']
		size anim_params['monitor1']['size']
		block:
			get_anim_image('monitor1')
			crop update_anim_and_get_crop('monitor1')
			monitor_pause_time
			repeat
	
	image anim monitor2:
		pos  anim_params['monitor2']['pos']
		size anim_params['monitor2']['size']
		block:
			get_anim_image('monitor2')
			crop update_anim_and_get_crop('monitor2')
			monitor_pause_time
			repeat
	
	image anim monitor3:
		pos  anim_params['monitor3']['pos']
		size anim_params['monitor3']['size']
		block:
			get_anim_image('monitor3')
			crop update_anim_and_get_crop('monitor3')
			monitor_pause_time
			repeat
	
	image anim monitor4:
		pos  anim_params['monitor4']['pos']
		size anim_params['monitor4']['size']
		block:
			get_anim_image('monitor4')
			crop update_anim_and_get_crop('monitor4')
			monitor_pause_time
			repeat
	
	image anim monitor5:
		pos  anim_params['monitor5']['pos']
		size anim_params['monitor5']['size']
		block:
			get_anim_image('monitor5')
			crop update_anim_and_get_crop('monitor5')
			monitor_pause_time
			repeat
	
	image anim monitor6:
		pos  anim_params['monitor6']['pos']
		size anim_params['monitor6']['size']
		block:
			get_anim_image('monitor6')
			crop update_anim_and_get_crop('monitor6')
			monitor_pause_time
			repeat
