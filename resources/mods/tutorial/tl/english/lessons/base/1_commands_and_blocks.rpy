translate english commands_and_blocks_593b0a50:
	# rn "С возвращением! Готов ко второму раунду нашего обучения?"
	rn "Welcome back! Ready for the second round of our training?"

translate english commands_and_blocks_61610a47:
	# rn "Надеюсь, ты успел немного поэкспериментировать и заставить меня сказать что-нибудь забавное в прошлом уроке."
	rn "I hope you had a chance to experiment a bit and make me say something funny in the previous lesson."

translate english commands_and_blocks_5b313a28:
	# rn "Сегодня мы разберёмся, из каких \"кирпичиков\" строится весь сценарий нашей игры. Это очень просто, вот увидишь!"
	rn "Today we'll figure out what \"bricks\" make up the entire scenario of our game. It's very simple, you'll see!"

translate english commands_and_blocks_93249a38:
	# rn "Представь, что мы пишем рецепт. Каждая строчка - это одно простое действие. В нашем мире это называется команда."
	rn "Imagine we're writing a recipe. Each line is one simple action. In our world, this is called a command."

translate english commands_and_blocks_dd7a0591:
	# rn "Всё очень логично. Сначала идёт само действие (имя команды), а потом - уточнения (мы называем их параметрами)."
	rn "Everything is very logical. First comes the action itself (the command name), then details (we call them parameters)."

translate english commands_and_blocks_89c22163:
	# rn "Например, команда [sts]pause[ste] может быть без параметров - тогда это будет пауза до клика мыши или чего-то подобного."
	rn "For example, the [sts]pause[ste] command can have no parameters - in that case, it's a pause until a mouse click or something similar."

translate english commands_and_blocks_4fd250f4:
	# rn "А если мы добавим параметр [sts]1[ste], то игра замрёт ровно на одну секунду."
	rn "But if we add a parameter like [sts]1[ste], the game will freeze for exactly one second."

translate english commands_and_blocks_321afeed:
	# rn "Помнишь, как я появлялась? Команда [sts]show[ste] имеет просто кучу параметров, которые уточняют: кто, как и где должен быть показан."
	rn "Do you remember how I appeared? The [sts]show[ste] command has just tons of parameters that specify who, how, and where I should be shown."

translate english commands_and_blocks_285c4dd2:
	# rn "А что, если нам нужно выполнить целую серию действий, но только при определённом условии?"
	rn "And what if we need to execute a whole series of actions, but only under a certain condition?"

translate english commands_and_blocks_ee000d8b:
	# rn "Например, \"если у игрока есть ключ, то открыть дверь, показать сокровище и проиграть победную музыку\"."
	rn "For example, \"if the player has a key, then open the door, show the treasure, and play the victory music\"."

translate english commands_and_blocks_f50df03d:
	# rn "Для таких случаев у нас есть блоки."
	rn "For such cases, we have blocks."

translate english commands_and_blocks_d9a01116:
	# rn "Блок - это такой удобный контейнер для других команд."
	rn "A block is just a convenient container for other commands."

translate english commands_and_blocks_344c11a0:
	# rn "Посмотри внимательно: строка, которая его объявляет, всегда заканчивается двоеточием."
	rn "Look closely: the line that declares it always ends with a colon."

translate english commands_and_blocks_f7a10b4c:
	# rn "Эта строчка как бы говорит: \"Внимание! Всё, что будет дальше с отступом - выполняется вместе, как одна большая задача!\"."
	rn "This line essentially says: \"Attention! Everything below it indented will execute together as one big task!\""

translate english commands_and_blocks_8cd02c89:
	# rn "Вот отличный пример из жизни! Это блок условия [sts]if[ste]."
	rn "Here's an excellent real-life example! This is the [sts]if[ste] condition block."

translate english commands_and_blocks_5b333ca2:
	# rn "Здесь мы проверяем, больше ли моих \"очков\", [sts]rn_points[ste], чем [sts]5[ste]."
	rn "Here we check if my \"points\", [sts]rn_points[ste], are greater than [sts]5[ste]."

translate english commands_and_blocks_d4c9374e:
	# rn "И только если это правда, выполняются две команды, которые стоят с отступом под ним: я появлюсь на экране и скажу свою реплику."
	rn "And only if this is true, the two commands indented below it execute: I appear on the screen and say my line."

translate english commands_and_blocks_d9f30f88:
	# rn "Если очков [sts]5[ste] или меньше - игра просто проигнорирует эти две строчки и пойдёт дальше."
	rn "If the points are [sts]5[ste] or less, the game simply ignores those two lines and continues."

translate english commands_and_blocks_c1414edd:
	# rn "Видишь эту связь? Двоеточие в конце - это обещание, что дальше будет блок с отступом. А отступ - это выполнение этого обещания."
	rn "Do you see that connection? The colon at the end is a promise that an indented block will follow. And the indentation is the fulfillment of that promise."

translate english commands_and_blocks_6cfbde55:
	# rn "Это золотое правило, запомни его, оно очень важное!"
	rn "This is a golden rule - remember it, it's very important!"

translate english commands_and_blocks_11bb027a:
	# rn "Кстати, {b}самыми{/b} главными могут быть лишь 3 блока."
	rn "By the way, {b}the most{/b} important blocks can be only 3."

translate english commands_and_blocks_b32f9a55:
	# rn "В [sts]init[ste]-блоках мы готовим всё к началу игры: регистрируем изображения, создаём персонажей, инициализируем переменные и всё в этом духе."
	rn "In [sts]init[ste] blocks we prepare everything for the start of the game: registering images, creating characters, initializing variables, and everything in that spirit."

translate english commands_and_blocks_35fac432:
	# rn "В метках [sts]label[ste] живёт сценарий: появление персонажей, музыка, диалоги и смена декораций."
	rn "In labels [sts]label[ste] the scenario lives: character appearances, music, dialogues, and decoration changes."

translate english commands_and_blocks_68be9673:
	# rn "В [sts]screen[ste] описывается интерфейс на языке ScreenLang. Это очень мощный и удобный инструмент, но о нём мы поговорим гора-а-аздо позже."
	rn "In [sts]screen[ste] the interface is described in ScreenLang. It's a very powerful and convenient tool, but we'll talk about it much later."

translate english commands_and_blocks_6a529dec:
	# rn "Фух! На сегодня теории достаточно."
	rn "Phew! That's enough theory for today."

translate english commands_and_blocks_c1a707b6:
	# rn "Главное, запомнить: команды - это одиночные действия, а блоки - это контейнеры для них, которые начинаются с двоеточия и требуют отступа у вложенных команд."
	rn "The main thing to remember: commands are single actions, while blocks are containers for them that start with a colon and require indentation for nested commands."

translate english commands_and_blocks_22dc93ca:
	# rn "Не переживай, если что-то кажется сложным, на практике всё станет кристально ясно!"
	rn "Don't worry if something seems complicated; in practice, everything will become crystal clear!"

translate english commands_and_blocks_91f682a7:
	# rn "В следующий раз мы с головой нырнём в [sts]init[ste]-блоки и научимся создавать собственных персонажей! Будет весело! До встречи!"
	rn "Next time we'll dive headfirst into [sts]init[ste] blocks and learn how to create our own characters! It'll be fun! See you!"


translate english strings:
	
	old 'Привет, ты не занят?'
	new 'Hello, are you busy?'
