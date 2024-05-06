label key__start:
	"Квест <Ключ> начат."
	
	"Я увидел какой-то ключ возле здания клубов."
	"Думаю, лучше его подобрать, пока кто-нибудь его не запнул в траву."
	"[Для этого нужно подойти к ключу и взять его в инвентарь, нажав E (англ.).]"
	"[Нажав I, можно открыть/закрыть инвентарь.]"
	window hide

label key__end:
	"Квест <Ключ> завершён."
	window hide

label key__*:
	if rpg_event_object != 'key':
		return
	
	$ set_rpg_control(False)
	
	if rpg_event == 'take':
		"Я подобрал ключ."
		"Интересно, что же мне теперь с ним делать?"
	
	if rpg_event == 'use':
		"И что я могу открыть этим ключом?"
	
	if rpg_event == 'remove':
		"Быть может, не стоит возится с этим ключём?"
		"Мало ли каких скелетов в потайных шкафах он может наоткрывать..."
	
	window hide
	$ set_rpg_control(True)

label key__radio_club*:
	if not inventory.has('key', 1):
		return
	
	$ set_rpg_control(False)
	$ location_cutscene_on(align='down')
	$ me.set_direction(to_forward)
	
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
	window hide
	
	$ sh.set_direction(to_forward)
	
	python:
		last_keys = inventory.remove('key', 1)
		if last_keys == 0:
			quest_end('key')
	
	$ location_cutscene_off()
	$ set_rpg_control(True)

