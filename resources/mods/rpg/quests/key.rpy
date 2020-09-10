label key__start:
	"Квест <Ключ> начат."
	
	"Я увидел какой-то ключ возле здания клубов."
	"Думаю, лучше его подобрать, пока кто-нибудь его не запнул в траву."
	"[Для этого нужно подойти к ключу и взять его в инвентарь, нажав E (англ.).]"
	"[Нажав I, можно открыть/закрыть инвентарь.]"

label key__end:
	"Квест <Ключ> завершён."

label key__on__taking__key:
	window show
	"Я подобрал ключ."
	"Интересно, что же мне теперь с ним делать?"
	window hide

label key__on__remove__key:
	window show
	"Быть может, не стоит возится с этим ключём?"
	"Мало ли каких скелетов в потайных шкафах он может наоткрывать..."
	window hide

label key__on__radio_club__clubs:
	if has_in_inventory("key", 1):
		$ set_rpg_control(False)
		$ location_cutscene_on()
		$ me.set_direction(to_forward)
		
		window show
		
		me "Я тут рядом проходил."
		$ sh.set_direction(to_back)
		me "Одна девочка напугала кузнечиком другую, постарше."
		me "Та сорвалась с места и убежала, но выронила какой-то ключ."
		
		sh "Это ключ от медпункта."
		sh "Лена только что за ним приходила."
		$ meet('un', 'Лена')
		sh "И, если я правильно тебя понял, она потеряла его из-за того, что её напугала Ульяна."
		$ meet('us', 'Ульяна')
		
		me "Ясно.{w} Так что с ключом-то делать?"
		sh "Да положи где-нибудь на столе, я отнесу потом."
		sh "Спасибо, что принёс его."
		me "Да пустяки."
		
		$ sh.set_direction(to_forward)
		
		python:
			last_keys = remove_from_inventory("key", 1)
			if last_keys == 0:
				quest_end('key')
		
		window hide
		$ location_cutscene_off()
		$ set_rpg_control(True)

