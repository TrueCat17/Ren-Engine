init python:
	player = persistent.player
	
	def get_random_player_name():
		word = random.choice((
			'Игрок', 'Кот', 'Лис', 'ЛунныйМодуль', 'ДамПамяти',
			'=(^-^)=', 'Диванодав', 'НалогИкамни', 'Велосипедист', 'Впродакшон!',
			'АдамВамЛам', 'X4K3P #1337', 'Мастдайство', 'Кнопка Сделать Зашибись', 'Инженер',
		))
		
		if random.random() < 0.5 or not word[-1].isalpha():
			return word
		
		num = random.randint(1, 33)
		return '%s-%s' % (word, num)

label first_meet:
	if config.language != 'russian':
		$ notification.out('Only Russian language is available now')
	
	show rn 2 smile with dspr
	rn "Ты наконец-то прибыл!"
	
	rn "Эм... Точно, привет!"
	rn "Меня зовут Рен. {w=0.5}Да-да, именно так."
	rn "А тебя мне звать как и прежде, по нику, или ты всё же скажешь мне своё настоящее имя?"
	
	$ notification.out('Доступны Ctrl+C и Ctrl+V')
	
	python:
		player_orig = get_random_player_name()
		renpy.input('player', prompt = '', default = player_orig)
	
	python:
		player = player.strip() or player_orig
		if player == player.lower():
			player = player.title()
		persistent.player = player
	
	rn "Хорошо, [player]."
	if player == player_orig:
		rn "Как хочешь. Мне так даже привычнее будет."
	else:
		rn "Я обязательно запомню твоё \"новое\" имя."
	
	show rn 3 smile with dspr
	rn "Кстати, ты можешь заглядывать сюда в любое время!"
	show rn 2 smile with dspr
	rn "Я всегда буду рада твоей компании."
	
	rn "В свободное от основной работы время я здесь делаю игры."
	rn "Иногда, конечно, и играю в них. Больше всего мне нравится \"Простая Цивилизация\" - очень советую, если ты любишь подобный жанр."
	rn "Кроме игр есть и более простенькие демки. {w=0.5}Но они тоже интересны!"
	rn "Возможно, они смогут вдохновить тебя на создание чего-то подобного, или даже ещё более увлекательного!"
	
	show rn 1 smile with dissolve
	rn "Кстати говоря... Я тут совсем недавно начала делать туториал по движку {color=#0C0}Ren-Engine{/color}, на котором и держится весь этот мир."
	show rn 3 smile with dissolve
	rn "Разумеется, у него и раньше была документация, но я решила сделать что-то более простое и доступное."
	rn "Так что если у тебя, [player], появится вдохновение, и ты найдёшь силы для изучения чего-то нового, то добро пожаловать в этот туториал!"
	show rn 5 sulk with dissolve
	rn "Было бы обидно, если после всех моих усилий по его созданию, он бы так тебе и не пригодился, правда?"
	
	show rn 2 smile with dissolve
	rn "Надеюсь, это было не слишком затянутое вступление."
	rn "Что же. Теперь я оставлю тебя перед открывшимися возможностями."
	rn "Если захочешь поболтать о чём-нибудь, то просто дай мне знать, хорошо?"
	
	hide rn with dissolve
	window hide
	
	$ persistent.first_meet_ended = True
