init 10 python:
	show_extra_help = True
	
	was = []
	
	show_character(sh, 'before_computer', 'radio_club')
	sh.set_direction(to_forward)


label rpg_start:
	python:
		set_rpg_control(False)
		make_names_unknown()
		
		set_location("ikarus", "sit_place")
		me.set_direction(to_right)
		me.set_pose("sit")
	
	window show
	if config.language != 'russian':
		"There is only russian text now."
	"Я проснулся в икарусе."
	
	$ me.set_pose("stay")
	pause 1
	$ me.move_to_place("before_sit_place")
	$ me.move_to_place("before_sit_place-2")
	
	"Управление WASD/стрелки + Shift (бег/шаг)."
	"Для выхода из локации-помещения подойдите к выходу и нажмите кнопку <Действие> - E."
	"То же самое относится и к входу."
	
	window hide
	$ set_rpg_control(True)

label enter__ikarus:
	if not show_extra_help:
		return
	
	$ set_rpg_control(False)
	
	$ show_extra_help = False
	window show
	"Быстрые сохранение/загрузка - Q/L."
	"Скрытие/Показ интерфейса - H, скриншот - P."
	window hide
	
	$ set_rpg_control(True)


label enter__before_gates:
	if "gates" in was:
		return
	$ was.append("gates")
	
	$ set_rpg_control(False)
	$ location_cutscene_on(align='down')
	$ me.move_to_place("before_gates", False)
	$ me.set_direction(to_forward)
	
	window show
	"Я подошёл к воротам"
	$ gate_right = get_location_objects(cur_location_name, me, "gate_right", 1)[0]
	$ gate_right.start_animation("opening")
	extend " и открыл их."
	"После я хотел уже было продолжить идти, как вдруг показалась девочка."
	window hide
	
	$ show_character(sl, "clubs")
	$ cam_to(sl, align='up')
	$ sl.start_animation('hello', -1)
	$ sl.move_to_place("behind_gates", False, 2)
	$ sl.remove_animation()
	
	window show
	sl "Привет, ты, наверное, только что приехал?"
	sl "Меня Славя зовут!"
    $ meet('sl', 'Славя')
    sl "Вообще, полное имя Славяна, но все меня Славей зовут.{w} И ты тоже зови!"
    
	sl "Лагерь ещё не доделан, а это - лишь демка, поэтому здесь представлено 4 локации."
	sl "В первой ты уже был - это Икарус."
	sl "Во второй ты находишься сейчас - это вход в лагерь."
	sl "Из третьей я пришла и туда же вернусь после этого разговора - в локацию клубов."
	sl "Четвёртая - само помещение клубов."
	
	sl "Понял?"
	menu:
		"Конечно":
			pass
		"Промолчать":
			sl "Видимо, тебя такие подробности не слишком интересуют."
	
	sl "Ну ладно, я пошла тогда."
	window hide
	
	$ sl.move_to_place("clubs")
	$ hide_character(sl)
	$ cam_to(me)
	
	$ location_cutscene_off()
	$ set_rpg_control(True)

label clubs__before_clubs:
	if "clubs" in was:
		return
	$ was.append("clubs")
	
	$ set_rpg_control(False)
	$ location_cutscene_on(align='down')
	$ me.move_to_place("before_porch")
	$ me.set_direction(to_forward)
	
	window show
	"Проходя мимо здания клубов, я заметил, что тут вышла {color=" + hex(un.name_text_color) + "}какая-то девушка{/color}."
	
	$ show_character(un, "radio_club")
	$ un.move_to_place("porch")
	
	$ show_character(us, "cluster")
	$ us.move_to_place("porch_left", True, 1)
	$ un.set_direction(to_left)
	
	"Пока я раздумывал, стоит ли подходить к ней, появилась {color=" + hex(us.name_text_color) + "}ещё одна{/color}."
	$ us.set_direction(to_right)
	"Она подошла к первой."		
	"Начала что-то рассказывать и..."
	
	$ us.start_animation("cricket", 0, 0.7)
	$ us.move_to_place("porch_cleft")
	$ un.x += 10
	extend "напугала её!"
	un "Иииии-ииииииииИИИИИ!!!"
	
	$ add_location_object('clubs', 'porch', 'key')
	$ un.move_to_place("admin", True, 0.5)
	"Первая убежала."
	
	$ us.remove_animation()
	$ us.set_direction(to_back)
	pause 0.2
	$ me.set_direction(to_right)
	$ us.move_to_place("admin", True, 0.5)
	"Вторая посмотрела на меня, а потом бросилась вслед за ней."
	$ hide_character(un)
	$ hide_character(us)
	
	$ me.set_direction(to_forward)
	"Когда они убежали, я заметил, что {color=" + hex(un.name_text_color) + "}первая{/color} девушка что-то обронила."
	$ quest_start('key')
	
	window hide
	$ location_cutscene_off()
	$ set_rpg_control(True)

label on__radio_club:
	if 'radio_club' in was:
		return
	$ was.append('radio_club')
	
	python:
		set_rpg_control(False)
		me.set_direction(to_forward)
		sh.set_direction(to_back)
	$ location_cutscene_on()
	
	window show
	
	sh "Хм... новенький?{w} Уже пришёл к нам записываться?"
	sh "А ты довольно быстр."
	sh "Я - Шурик."
    $ meet('sh', 'Шурик')
	
	me "Я - " + str(me) + ", приятно познакомиться, конечно, но я просто зашёл посмотреть..."
	
	window hide
	$ sh.set_direction(to_forward)
	$ location_cutscene_off()
	$ set_rpg_control(True)

