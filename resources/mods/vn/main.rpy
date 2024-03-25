label start:
	play music music_list['quiet'] fadein 3
	scene bg room
	
	if config.language != 'russian':
		"There is only russian text now."
	
	show rn happy with dspr
	rn "Привет!"
	show rn normal with dissolve
	rn "Меня зовут Рен."
	rn "В честь этого движка - Ren-Engine."
	
	rn "Я могу рассказать о некоторых его возможностях для игр в жанре <Визуальные Новеллы>."
	rn "Хочешь?"
	me "Давай."
	
	show rn serious with dissolve
	rn "С чего бы начать?"
	me "Может быть, с самого простого?"
	me "Это текст."
	
	rn "Да, наверное."
	rn "В нём могут использоваться теги: {u}подчёркивание{/u}, {i}курсив{/i}, {b}полужирный текст{/b}, {s}зачёркнутый{/s}, {color=00FF00}раскрашенный{/color} и {w=0.75}пауза."
	rn "Пауза может держаться {w=1}временно."
	rn "А может {w}до тех пор, пока пользователь не продолжит сам."
	
	show rn normal with dissolve
	extend "\nЕщё текст можно продолжить, то есть не стирать то, что уже было раньше."
	
	$ set_mode_nvl()
	rn "Кстати, если нужно сразу показывать много текста, то можно использовать этот режим."
	rn "Тогда можно будет постоянно видеть предыдущий текст."
	rn "Иногда это довольно удобно."
	nvl clear
	rn "Но его нужно убирать вручную."
	$ set_mode_adv()
	
	rn "Вроде бы это всё, что касается текста."
	rn "Далее. {w=0.5}Мало какая игра обходится без эффектов."
	rn "Поэтому сейчас самое время их показать!"
	rn "Вот это: "
	show rn serious-2 with dissolve
	extend "переход dissolve."
	rn "Кстати, он использовался с самого начала."
	rn "Пора переходить к новому."
	stop music fadeout 2
	rn "Пошли на балкон?"
	me "Пошли."
	
	play music music_list['dreams'] fadein 3
	scene bg night
	show rn normal
	with fade
	rn "Это был переход fade."
	rn "Разумеется, времена затенения, ожидания и появления, а также цвет можно менять."
	rn "Видел в фильмах, как герои и злодеи бросают себе под ноги всякие штуки, которые тут же начинают дыметь, а сами они в это время убегают?"
	rn "Я вот не понимаю, почему никто не додумался использовать вместо этого световую гранату?"
	scene bg night with Fade(0.75, 1, color="#FFF")
	me "Ай!"
	show rn normal with dissolve
	rn "Или ещё можно использовать другой цвет. Например, красный."
	rn "Красный - это же ведь красиво, правда?"
	scene bg night with Fade(0.5, 2, color="#E11")
	me "Да хватит уже!"
	show rn surprise with dissolve
	me "От такого и ослепнуть можно, вот почему этим никто не пользуется при побегах!"
	rn "Да? Ну, извини!"
	show rn normal with dissolve
	rn "Просто я ослепнуть не могу."
	rn "Но так как я человек чуткий, заботливый и понимающий, то я не буду больше использовать такое рядом с тобой."
	me "Очень на это надеюсь."
	
	show rn happy with vpunch
	rn "Да ладно тебе!"
	me "И бить меня по плечу тоже не нужно."
	rn "Кстати, это был vpunch (встряска по-вертикали), ещё есть hpunch (по-горизонтали)."
	
	rn "Я ещё не расказывала тебе про телепорт?"
	rn "Классная штука!"
	show rn normal at left with ImageDissolve("images/masks/teleport_1.png")
	rn "Но он работает с помощью масок, а они на данный момент реализованы только программным способом, а это не слишком-то быстро и экономно."
	show rn normal at center with ImageDissolve("images/masks/teleport_2.png")
	rn "А переместить персонажа в другое место можно с помощью любого эффекта."
	rn "Или даже вообще без эффектов, но это будет выглядеть несколько дёргано."
	
	rn "На этом демонстрацию эффектов можно остановить."
	rn "Теперь поговорим об im-функциях."
	rn "Вообще-то их довольно много: изменение размера, вырезка части изображения, накладывание картинок друг на друга, поворот и так далее..."
	rn "О только что перечисленных функциях и говорить особо нечего - их действия понятны из их названий."
	
	rn "Так, о масках я тебе только что уже рассказывала."
	rn "Остаются ReColor и MatrixColor."
	rn "Обе функции работают с изображением попиксельно."
	
	rn "ReColor позволяет умножить значение канала (RGBA) каждого пикселя на определённое значение."
	rn "Например, 255 - оставляет как есть, а 51 - оставляет только 1/5 часть от оригинального значения."
	
	"Рен подошла к выключателю и щёлкнула им."
	show rn normal night with dissolve
	"Свет погас."
	rn "Видишь?"
	me "Да."
	rn "Ну и чего ты стоишь тогда?"
	rn "Пошли домой, становится холодно же."
	
	scene bg room
	show rn normal
	with fade
	
	rn "MatrixColor позволяет не только умножать значение канала на какое-то число, но и указывает взаимосвязь каналов между собой."
	rn "Благодаря этому каналы можно поменять местами, инвертировать их значения, изменить яркость, контраст и много чего ещё."
	rn "И слово Matrix присутствует здесь не просто так, разумеется."
	rn "Допустим, нам к изображению нужно применить несколько матриц (например, матрицы инвертирования цветов и повышения яркости)."
	rn "Можно было бы подумать, что мы должны для этого сделать 2 преобразования, так как матриц у нас 2."
	rn "Но на самом деле матрицы можно перемножать друг с другом перед применением к изображению."
	rn "В конечном итоге у нас получится только 1, окончательная матрица."
	rn "Следовательно, MatrixColor нужно вызвать только один раз."
	rn "Таким образом можно сделать несколько преобразований за время одного."
	rn "Это довольно полезная возможность MatrixColor."
	
	show rn normal:
		align (0.5, 1.0)
		linear 0.5 xalign 0.85
	show photo ren at left_center with dissolve
	rn "Например, вот моя чёрно-белая фотография."
	show photo room with dspr
	rn "А здесь - эта комната, но перекрашенная."
	show rn normal:
		align (0.85, 1.0)
		linear 0.5 xalign 0.5
	hide photo with dspr
	
	stop music fadeout 2
	rn "Ну и напоследок, наверно, можно показать внутриигровое меню."
	rn "Оно обычно используется, когда Герой стоит на Развилке Судьбы и делает Сложный Выбор."
	
	play music music_list['silent'] fadein 3
	show rn happy at center with dissolve
	rn "Полагаю, вопрос <Сколько будет 123 * 24?> достаточно сложен, чтобы показать меню именно на его примере."
	$ right = False
	$ start_answer = get_game_time()
	menu:
		"12324":
			pass
		"-295":
			pass
		"2956":
			pass
		"2952":
			$ right = True
		"29512":
			pass
	
	if right:
		$ spent_answer = get_game_time() - start_answer
		
		show rn serious with dissolve
		rn "Ты справился за " + str(round(spent_answer)) + " секунд."
		
		if spent_answer < 5:
			rn "Ты просто щёлкнул на первый попавшийся ответ и угадал, не так ли?"
		elif spent_answer < 15:
			rn "Видимо, это было совсем уж легко."
		else:
			rn "Кажется, это было не так уж и сложно."
		show rn normal with dissolve
	else:
		show rn normal with dissolve
		rn "Тест провален."
	
	
	rn "Наверно, на сегодня хватит."
	rn "Пока?"
	me "Пока."
	stop music fadeout 1
	hide rn with ImageDissolve("images/masks/teleport_1.png")
	
	scene bg black with dissolve

