init python:
	lessons.add_current('image, ATL, show, hide, with')

label images:
	show rn 2 smile
	rn "Перед тем, как отображать сцены и персонажей, нам нужно их всех зарегистрировать."
	
	python:
		code.show('''
			image rn sml = "images/spr/rn/sml.png"
			image rn hpp = "images/spr/rn/hpp.png"
		''')
	show rn 4 smile with dissolve:
		rn_moving 0.5 xpos 0.28
	
	rn "Это делается в [sts]init[ste]-блоке."
	rn "Путь к изображениям задаётся относительно папки [sts]resources/[ste]."
	
	rn "На этом экране мало места, так что используются сокращения для слов [sts]sprites[ste], [sts]smile[ste] и [sts]happy[ste]."
	
	show rn 3 bore with dissolve
	rn "В своей игре ты, конечно, не должен их использовать!"
	rn "Ведь это - просто пример."
	
	python:
		code.show('''
			label start:
				show rn smile
				pause 1
				show rn happy
				pause 1
				hide rn
		''')
	show rn 1 smile with dissolve
	
	rn "Теперь мы можем использовать зарегистрированные спрайты!"
	rn "Как ты уже знаешь из самого первого урока, команды [sts]scene[ste] и [sts]show[ste] отвечают за показ картинки."
	rn "Название нужной картинки в этом случае должно указываться полностью."
	
	show rn 2 smile with dissolve
	rn "Второй вызов команды [sts]show[ste] уберёт первый спрайт, потому что они оба имеют \"псевдоним\" [sts]rn[ste]."
	rn "Псевдоним берётся из первого слова в названии, либо из параметра [sts]as[ste] как в этом примере: [text_code('show rn happy as rn2')]."
	rn "Такое поведение нужно редко, но всё же."
	
	rn "Команда [sts]hide[ste], если ты вдруг ещё не догадался, скрывает указанный спрайт."
	rn "И она принимает как раз псевдоним, то есть для удаления персонажа со сцены не нужно указывать точное название его изображения."
	rn "Это очень удобно."
	
	
	python:
		code.show('''
			show rn smile with dissolve
			pause 1
			show rn happy with dspr
			pause 1
			hide rn with dissolve
		''')
	
	show rn 3 wink with dissolve
	rn "Разумеется, обычно эти команды используют эффекты, но про них будет отдельная глава, а здесь они, полагаю, будут лишь отвлекать тебя."
	
	
	python:
		code.show('''
			show rn smile at left
			pause 1
			show rn happy
			pause 1
			hide rn
		''')
	
	show rn 2 smile with dissolve
	rn "Кроме эффектов есть и куча других параметров."
	rn "Например, параметр [sts]at[ste] позволяет указать место, в котором нужно отобразить спрайт."
	rn "Стандартные места слева направо: [sts]fleft[ste], [sts]left[ste], [sts]cleft[ste], [sts]center[ste], [sts]cright[ste], [sts]right[ste] и [sts]fright[ste]."
	rn "Здесь [sts]f[ste] - сокращение от [sts]far[ste], то есть дальше от центра, а [sts]c[ste] - от [sts]close[ste] - близко."
	rn "Думаю, эта информация поможет тебе лучше их запомнить."
	
	rn "Если место не указано, то оно остаётся прежним."
	rn "А если до этой команды спрайт ещё не отображался, то используется центр."
	
	
	python:
		code.show('''
			show rn smile at left
			
			show rn smile:
				xpos 0.28
				xanchor 0.5
				yalign 1.0
			
			show rn smile:
				linear 1.0 xpos 0.5
		''')
	
	show rn 5 smile with dissolve
	rn "На самом деле эти \"места\" - специальные объекты, анимации-трансформации, которые могут гораздо больше, чем просто определить позицию спрайта."
	rn "Позже о них будет отдельная статья, сейчас же..."
	rn "Первый и второй примеры делают одно и то же."
	rn "В третьем используется функция [sts]linear[ste] для линейного движения за секунду к центру."
	
	
	python:
		code.show('''
			show rn smile
			show some person behind rn
		''')
	
	show rn 1 smile with dissolve
	rn "В некоторых случаях нужно показать нового персонажа, но поставить его позади уже существующего на сцене."
	rn "Для этого используется ключевое слово [sts]behind[ste]."
	
	
	python:
		code.show('''
			scene bg entry
			show rn smile
			with fade
		''')
	
	show rn 3 smile with dissolve
	rn "Кстати! {w=0.33}Если нужно показать несколько изображений сразу, с применением эффекта, но без пауз между показами, [sts]with[ste] можно использовать вот так."
	rn "В этом случае он как бы \"собирает\" все команды [sts]scene[ste] и [sts]show[ste], идущие до него подряд, а затем одновременно применяет к ним указанный эффект."
	
	
	$ code.show('$ config.width, config.height = ...')
	show rn 2 smile with dissolve
	
	rn "Ну и наконец - автомасштабирование спрайтов."
	rn "В файле [sts]mods/common/sprites.rpy[ste] есть строка вроде этой. В ней указаны размеры экрана, для которого нарисованы спрайты."
	rn "Тебе нужно поменять эти числа на свои. И если окно игры будет меньше их, то спрайты автоматически уменьшатся, а если больше - увеличатся."
	
	$ code.hide()
	show rn 2 smile:
		rn_moving 0.5 xpos 0.5
	
	rn "До встречи в последнем уроке по основам!"
