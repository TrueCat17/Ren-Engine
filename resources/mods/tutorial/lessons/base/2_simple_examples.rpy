init python:
	lessons.add_current('character, text, if, while, jump, return')


label simple_examples:
	show rn 1 smile
	rn "Добро пожаловать в длиннейший урок из начальной категории!"
	show rn 1 laugh with dissolve
	rn "Это будет долго, но оно того стоит, поехали!"
	
	show rn 2 smile with dissolve
	
	rn "Для твоего удобства я сделала меню."
	rn "Можешь сразу найти интересующую тебя часть, но в первый раз лучше просто иди по порядку."
	window hide
	
	show rn 2 smile:
		rn_moving 0.5 xpos 0.16
	pause 0.33
	
	$ current_part.show('Начало')
	
	menu:
		"Персонажи":
			call simple_examples_move_to_left
			call simple_examples_characters
		"Интерполяция":
			call simple_examples_move_to_left
			call simple_examples_interpolation
		"Тэги":
			call simple_examples_move_to_center
			call simple_examples_tags
		"Python":
			call simple_examples_move_to_left
			call simple_examples_python
		"Условия":
			call simple_examples_move_to_left
			call simple_examples_conditions
		"Циклы":
			call simple_examples_move_to_left
			call simple_examples_cycles
		"Переходы по меткам":
			call simple_examples_move_to_left
			call simple_examples_label_transitions
		"Отображение текста":
			call simple_examples_move_to_center
			call simple_examples_text_displaying
		"Меню":
			call simple_examples_move_to_left
			call simple_examples_menu
		"Пауза":
			call simple_examples_move_to_left
			call simple_examples_pause
		"Моды":
			call simple_examples_move_to_center
			call simple_examples_mods


label simple_examples_move_to_left:
	show rn 2 smile:
		rn_moving 0.4 xpos 0.28
	pause 0.4

label simple_examples_move_to_center:
	show rn 2 smile:
		rn_moving 0.75 xpos 0.5
	pause 0.75


label simple_examples_characters:
	$ current_part.show('Персонажи')
	
	rn "Итак, после предыдущих уроков тебя наверняка заинтересовал вопрос: как же создать своего персонажа?"
	
	python:
		code.show('''
			python:
				np = Character("Новая Персона",
				               color = "#F80")
		''')
	
	rn "И если ты всё ещё не заглянул в [sts]mods/common/characters.rpy[ste], то ответ - вот так."
	
	rn "Позже задать персонажу другое имя можно так: [text_code('set_name(\"np\", \"Прохожий\")')]."
	extend " Обрати внимание, что [sts]np[ste] здесь - тоже в кавычках."
	
	rn "Вообще-то класс [sts]Character[ste] может принимать огромную кучу параметров, но в этом маленьком туториале для них нет места. "
	rn "Так что в случае необходимости смотри подробности в документации, особенно обрати внимание на статью [text_code('Интерфейс (gui)')]."
	
	show rn 2 smile:
		rn_moving 0.5 xpos 0.5
	$ code.hide()
	
	
	rn "Теперь поговорим о стандартных персонажах."
	
	rn "[sts]narrator[ste] - рассказчик, используется по умолчанию, когда ничего не указано."
	rn "Пример: [sts]\"Светит яркое солнце.\"[ste]"
	rn "А выглядит это так:"
	"Светит яркое солнце."
	
	rn "[sts]th[ste] - мысли главного героя твоего произведения."
	rn "[sts]th \"Мне было жарко.\"[ste]"
	th "Мне было жарко."
	
	rn "[sts]me[ste] - слова от лица главного героя (нужно регистрировать самостоятельно)."
	rn "[sts]me \"Здравствуйте.\"[ste]"
	me "Здравствуйте."
	
	rn "Ну и [sts]extend[ste] - в отличие от всех остальных, не стирает текст, а продолжает его дальше от лица предыдущего говорящего."
	rn "[sts]extend \" Дайте 2 мороженных.\"[ste]"
	extend " Дайте 2 мороженных."
	
	call simple_examples_interpolation
label simple_examples_interpolation:
	$ current_part.show('Интерполяция')
	
	show rn 1 smile with dissolve
	
	rn "Текст внутри квадратных скобок интерполируется."
	rn "Звучит сложно, но посмотри сам, [sts]\"2 + 3 = [[2 + 3]\"[ste] выведет вот это:"
	extend " 2 + 3 = [2 + 3]."
	
	python:
		code.show('''
			$ player = "Имя"
			# ...
			rn "Привет, [player]!"
		''')
	show rn 1 smile:
		rn_moving 0.5 xpos 0.28
	
	rn "Хотя обычно это используют для вывода переменной. Что-то типа этого."
	
	show rn 1 smile:
		rn_moving 0.5 xpos 0.5
	$ code.hide()
	
	show rn 2 smile with dissolve
	rn "Кроме выполнения содержимого в качестве python-кода, интерполяция позволяет компактным образом делать некоторые вещи."
	rn "Например, [sts][['qwerty'!u][ste] выведет ['qwerty'!u]. Другие \"модификаторы\" ты можешь найти в документации."
	
	rn "Ну а если нужно вывести сам символ [sts][[[ste], чтобы движок не посчитал его началом кода для интерполяции, его всего лишь нужно написать 2 раза: [sts][[[[[ste]."
	
	call simple_examples_tags
label simple_examples_tags:
	$ current_part.show('Тэги')
	
	show rn 3 smile with dissolve
	
	rn "Теперь пришло время поговорить о тэгах."
	
	rn "Есть простые:"
	extend  " [in_tags('i|курсив|/i')]"
	extend ", [in_tags('b|\"полужирность\"|/b')]"
	extend ", [in_tags('u|подчёркивание|/u')]"
	extend ", [in_tags('s|зачёркивание|/s')]"
	extend " и [in_tags('plain|обычный|/plain')]."
	
	rn "Как ты можешь заметить, для указания на начало тэга используется символ [sts]{{[ste]."
	extend " И как и в случае с [sts][[[ste], чтобы вывести его как символ, его тоже нужно продублировать: [sts]{{{{[ste]."
	
	rn "Кстати, будь внимателен: тэги должны закрываться в обратном порядке (относительно их открытия)."
	rn "Вот так писать нельзя: [in_tags('b|123|i|456|/b|789|/i', real_tags = False)]."
	
	show rn 2 smile with dissolve
	
	rn "Есть тэги чуть сложнее:"
	extend  " [in_tags('color=#08F|цвет|/color')]"
	extend ", [in_tags('outlinecolor=#7CF|обводка|/outlinecolor')]"
	extend "\nи картинка [in_tags('image=images/anim/planet_light.png')]."
	rn "Обрати внимание, что внутри [sts]{{[ste]тэга[sts]}[ste] не используются кавычки."
	
	rn "Также есть возможность использовать другой [in_tags('font=FixEx3|шрифт|/font')]."
	extend " Шрифты лежат в папке [sts]resources/fonts/[ste]."
	
	rn "Можно поменять и размер: [in_tags('size=50|текст|/size')]."
	
	rn "Однако, изначальный размер текста зачастую зависит от размеров окна игры, поэтому имеет смысл задавать относительный размер."
	rn "Сделать это можно с помощью символов [sts]+[ste], [sts]-[ste] и [sts]*[ste] после знака [sts]=[ste]:"
	rn "Обычный, [in_tags('size=+7|увеличенный|/size| и |size=*0.8|уменьшенный|/size')] тексты."
	
	rn "Таким же образом работает изменение прозрачности:\n[in_tags('alpha=0.5|полупрозрачный текст|/alpha')]."
	
	show rn 3 smile with dissolve
	rn "Ну и последний тэг - пауза [sts]{{w}[ste], работает только в этом окне диалога."
	show rn 2 smile with dissolve
	
	rn "Как и у тэга [sts]image[ste], у него отсутствует \"закрывающая\" часть."
	rn "Пауза без указания времени - \"бесконечна\", а указать время{w=0.5}, в секундах{w=0.5}, можно так: [sts]{{w=0.5}[ste]."
	rn "Пауза любой длительности может быть пропущена игроком любым способом, в том числе нажатием на [sts]Пробел[ste] или [sts]Enter[ste]."
	
	call simple_examples_python
label simple_examples_python:
	$ current_part.show('Python')
	
	rn "Перейдём к способам запуска python-кода в сценарии или в [sts]init[ste]-секциях."
	
	show rn 4 smile with dissolve:
		rn_moving 0.5 xpos 0.28
	
	python:
		code.show('''
			label example:
				python:
					a = 3
					b = a + 2
		''')
	
	rn "Вот так выглядит универсальный способ."
	
	python:
		code.show('''
			label example:
				$ a = 3
		''')
	
	rn "Если же нужно запустить лишь одну строку кода, то есть более компактный вариант записи."
	rn "Здесь [sts]$[ste] - команда, запускающая то, что идёт дальше в строке, и пробел после команды обязателен, как и всегда."
	
	python:
		code.show('''
			rn "Текст."
			$ rn("Текст.")
		''')
	
	show rn 3 smile with dissolve
	
	rn "Кстати, если вдруг интересно, вот эти 2 строки делают одно и то же."
	rn "Просто на случай, если тебе понадобится отобразить реплику из python-кода."
	
	$ code.show('print(a)')
	rn "Ну и заканчивая эту тему: функция [sts]print[ste] переопределена так, что её вывод можно увидеть в файле [sts]var/log.txt[ste]."
	rn "Иногда это бывает удобно, хотя для \"одноразовых\" проверок удобнее использовать консоль движка, открываемую по [sts]Shift+O[ste]."
	
	call simple_examples_conditions
label simple_examples_conditions:
	$ current_part.show('Условия')
	
	show rn 2 smile with dissolve
	
	rn "В играх бывают развилки, и реализуются они с помощью условий."
	
	python:
		code.show('''
			label example:
				me "Как дела?"
				
				if rn_points < 5:
					rn "..."
				elif rn_points < 10:
					rn "Нормально."
				else:
					rn "Хорошо. А у тебя?"
				
				me "Я думал прогуляться."
				extend " Составишь компанию?"
		''')
	
	rn "Условия работают точно также, как и в питоне."
	rn "Всё начинается с ключевого слова [sts]if[ste], после него идёт нужная проверка."
	
	show rn 3 smile with dissolve
	rn "Если она выполняется успешно, то идёт выполнение вложенных команд, а дальнейшие блоки [sts]elif[ste] и [sts]else[ste] игнорируются."
	show rn 2 smile with dissolve
	
	rn "Если же проверка не выполняется, то рассматривается следующее условие."
	rn "Ты можешь использовать любое количество веток [sts]elif[ste], в том числе и ноль."
	rn "Наконец, если ни одно из условий не было исполнено, то управление переходит к блоку [sts]else[ste], который тоже может отсутствовать."
	rn "А потом исполнение команд движется дальше.{w=0.5} Как видишь, ничего сложного."
	
	python:
		code.show('''
			label ok:
				$ a = 3
				if a == 3:
					pass
			
			# ошибочное использование
			label error:
				$ a == 3
				if a = 3:
					pass
		''')
	
	show rn 3 smile with dissolve
	
	rn "Хотя некоторые новички путают присваивание ([sts]=[ste]) и сравнение ([sts]==[ste])."
	rn "Но ты же не будешь, правда?"
	rn "Кстати, команда [sts]pass[ste] здесь означает \"ничего не делать\". Пусть и редко, но она бывает полезна, когда в блоке нет осмысленных вложенных команд."
	rn "А ещё здесь представлен комментарий, который игнорируется движком и питоном: от символа [sts]#[ste] и до конца строки."
	
	call simple_examples_cycles
label simple_examples_cycles:
	$ current_part.show('Циклы')
	
	python:
		code.show('''
			np "Я считаю до 5."
			np "Не могу до 10."
			np ""
			$ num = 0
			while num < 5:
				extend "[num + 1] "
				$ num += 1
			extend "- с рифмой с детства я дружу!"
			# Последняя строка будет такой:
			# 1 2 3 4 5 - с рифмой с детства я дружу!
		''')
	
	show rn 4 smile with dissolve
	
	rn "Есть цикл [sts]while[ste] - такой же, как в питоне."
	rn "Так как он редко используется вне питона и вряд ли понадобится новичкам, то я просто покажу очевидный пример без особых объяснений."
	rn "Как и в питоне, внутри цикла поддерживаются команды [sts]continue[ste] и [sts]break[ste], а после - блок [sts]else[ste]."
	rn "Цикл [sts]for[ste] не поддерживается из-за того, что он использует итераторы, не все из которых могут быть сохранены во время сохранений."
	
	rn "Ну а теперь снова вернёмся к часто используемым вещам."
	
	call simple_examples_label_transitions
label simple_examples_label_transitions:
	$ current_part.show('Переходы по меткам')
	
	python:
		code.show('''
			label example:
				rn "Начальный текст."
				jump some_label
			
			label some_label:
				rn "Текст из some_label."
		''')
	
	show rn 3 smile with dissolve
	rn "Чтобы процесс создания игры был прост и удобен, мы ни в коем случае не должны писать весь сценарий в одном файле, и тем более - в одной метке."
	rn "А раз у нас много меток, то нужны и способы перехода между ними."
	show rn 2 smile with dissolve
	
	rn "Команда [sts]jump[ste] говорит движку \"прыгнуть\" на указанную метку и завершить игру после того, как все её команды будут выполнены."
	
	python:
		code.show('''
			label example:
				rn "Начальный текст."
				call some_label
				rn "Продолжение текста."
			
			label some_label:
				rn "Текст из some_label."
		''')
	
	rn "Ещё есть команда [sts]call[ste], которая после выполнения метки возвращает управление назад и продолжает исполнять дальнейшие команды."
	
	rn "Обе эти команды поддерживают \"вычисление\" метки python-кодом, если есть ключевое слово [sts]expression[ste]."
	extend "\nПример: [sts]call expression \"my_label_name_\" + str(num)[ste]."
	rn "Кроме того, эти команды можно и напрямую вызвать из кода:\n[sts]$ renpy.jump('some_label')[ste]."
	
	rn "И, конечно, есть команда [sts]return[ste] для досрочного завершения текущей метки и возврата назад, если есть куда возвращаться."
	
	show rn 2 smile:
		rn_moving 0.5 xpos 0.5
	$ code.hide()
	
	rn "Вернёмся к тексту."
	
	call simple_examples_text_displaying
label simple_examples_text_displaying:
	$ current_part.show('Отображение текста')
	
	rn "Команды [sts]window show[ste] и [sts]window hide[ste] отвечают за показ и скрытие диалогового окна."
	window hide
	pause 1.5
	rn "Как-то так. Кстати, при выводе текста диалоговое окно показывается автоматически."
	
	rn "Есть два режима отображения текста."
	rn "Первый, стандартный, используется сейчас и называется [sts]ADV[ste]."
	
	$ current_part.hide()
	$ set_mode_nvl()
	rn "Также есть [sts]NVL[ste]-режим, который выглядит вот так."
	rn "Он включается вызовом [sts]$ set_mode_nvl()[ste]."
	rn "Как видишь, здесь не происходит автоматического удаления предыдущих реплик."
	rn "Зато это можно сделать вручную с помощью команды [text_code('nvl clear')]."
	rn "Обрати внимание, что это не python-код, поэтому значок [sts]$[ste] здесь не нужен."
	nvl clear
	rn "Собственно, вот так выглядит очистка экрана."
	rn "Нужно не забывать это делать, чтобы текст не вылезал за данные ему границы."
	rn "Вернуться в стандартный [sts]ADV[ste]-режим можно с помощью [text_code('$ set_mode_adv()')]."
	$ set_mode_adv()
	
	call simple_examples_menu
label simple_examples_menu:
	$ current_part.show('Меню')
	
	python:
		code.show('''
			$ energy = 10
			$ skills = 4
			th "Куда бы мне пойти?"
			menu:
				"Никуда, останусь сидеть дома.":
					pass
				"Пора на работу.":
					$ energy -= 5
					$ skills += 1
				"В парк!" if energy > 2 else None:
					$ energy -= 2
					call day2_park_meet
		''')
	
	show rn 2 smile:
		rn_moving 0.5 xpos 0.28
	
	rn "Важной частью многих игр является меню выбора из нескольких вариантов."
	rn "В общем-то, тут всё просто: при выборе пункта меню выполняются его вложенные команды."
	rn "Как можно заметить, некоторые пункты меню можно не отображать, если не соблюдается нужное условие."
	rn "А ещё можно передать пустую строку - это создаст зазор между пунктами меню."
	rn "Ну а как выглядит меню выбора, ты уже видел в начале этого урока."
	
	call simple_examples_pause
label simple_examples_pause:
	$ current_part.show('Пауза')
	
	python:
		code.show('''
			pause
			pause 1.5
			$ pause(1.5)
			$ renpy.pause(1.5)
		''')
	
	show rn 1 smile with dissolve
	
	rn "Последняя команда в этом уроке - пауза."
	rn "Раньше я говорила про тэг паузы [sts]{{w}[ste], но он работает только посреди текста, а это не всегда то, что нужно."
	rn "Например, иногда нужно показать изображение, подождать две секунды и сменить его на другое изображение."
	rn "Как видишь, эту команду можно вызвать и из python-кода, если вдруг так будет удобнее."
	rn "Кстати, здесь точно также любая пауза может быть пропущена."
	
	python:
		code.show('''
			python:
				pause(1)
				print(123)
		''')
	
	show rn 3 smile with dissolve
	rn "Однако следует помнить, что пауза начинается после полного выполнения python-кода, а не прямо посреди него."
	rn "Таким образом, вот этот код сначала вызовет функцию [sts]print[ste], а уже потом поставит сценарий на секундную паузу."
	
	show rn 2 smile with dissolve
	rn "Конечно, это не играет никакой роли, если код состоит из одной строки."
	
	show rn 2 smile:
		rn_moving 0.5 xpos 0.5
	$ code.hide()
	
	rn "Напоследок поговорим о модах."
	
	call simple_examples_mods
label simple_examples_mods:
	$ current_part.show('Моды')
	
	rn "В мире Ren-Engine \"мод\" - это папка с [sts]rpy[ste]-файлами и нужными ресурсами."
	rn "Понятия \"мод\" и \"непосредственно сама игра\" объединены в одно, чтобы не создавать новые сущности там, где в этом нет необходимости."
	rn "Ведь основная игра может делать ровно то же самое, что и моды."
	
	show rn 3 smile with dissolve
	
	rn "Все моды лежат в [sts]resources/mods/[ste]."
	rn "В начале движок запускает мод основного меню [sts]main_menu[ste], из него возможны переходы в другие моды, например - в саму игру."
	rn "У каждого мода есть отдельная папка. Это нужно для поддержания порядка, а также для отсутствия конфликтов и замещения файлов."
	
	rn "Если мод должен отображаться в списке модов, то он также должен содержать файл [sts]name[ste] (без расширений вроде [sts].txt[ste]) с названием мода."
	rn "Конечно, это не нужно для модов главного меню или основной игры."
	
	rn "Для запуска мода нужно вызвать функцию [sts]start_mod('mod_dir')[ste], в которую передаётся имя папки мода."
	rn "Ты можешь вызвать её даже из [sts]init[ste]-блока главного меню, что бывает удобно во время разработки."
	
	show rn 2 smile with dissolve
	
	rn "Наконец, что происходит при запуске какого-либо мода?"
	rn "Идёт разбор всех [sts]rpy[ste]-файлов движка в [sts]Ren-Engine/rpy/[ste], общих файлов игры в [sts]mods/common/[ste] и файлов мода в [sts]mods/mod_dir/[ste]."
	rn "Выполняются все [sts]init[ste]-блоки в нужном порядке."
	rn "Отображаются все скрины из списка [sts]start_screens[ste]."
	rn "И происходит переход на метку [sts]start[ste], если она есть."
	extend " После выполнения последней команды метки идёт возврат в главное меню."
	
	python:
		code.show('''
			init:
			init 10:
			init python:
			init -1 python:
		''')
	show rn 4 smile with dissolve:
		rn_moving 0.5 xpos 0.28
	
	rn "Кстати, [sts]init[ste]-блоки имеют 2 необязательных параметра."
	rn "Приоритет - просто число, чем оно меньше, тем раньше будет выполнен этот блок."
	rn "А [sts]python[ste] указывает, что всё содержимое блока является python-кодом."
	
	$ code.hide()
	$ current_part.hide()
	pause 0.33
	show rn 2 smile with dissolve:
		rn_moving 0.5 xpos 0.5
	
	rn "Вот и всё!"
	
	rn "На этом этот огромный урок подошёл к концу."
	rn "Ты молодец, если смог его осилить и всё понять!"
	rn "А если нет - тоже не беда, ведь его всегда можно просмотреть ещё раз."
	rn "До встречи в следующем уроке!"
