translate english simple_examples_9b23260b:
	# rn "Добро пожаловать в длиннейший урок из начальной категории!"
	rn "Welcome to the longest lesson in the beginner category!"

translate english simple_examples_b81c6022:
	# rn "Это будет долго, но оно того стоит, поехали!"
	rn "It will be long, but it's worth it, let's go!"

translate english simple_examples_9ac23331:
	# rn "Для твоего удобства я сделала меню."
	rn "For your convenience, I've made a menu."

translate english simple_examples_57fbb050:
	# rn "Можешь сразу найти интересующую тебя часть, но в первый раз лучше просто иди по порядку."
	rn "You can immediately find the part you're interested in, but the first time, it's better to just follow the order."

translate english simple_examples_characters_dc6ea0f8:
	# rn "Итак, после предыдущих уроков тебя наверняка заинтересовал вопрос: как же создать своего персонажа?"
	rn "So, after the previous lessons, you surely have been interested in the question: how to create your own character?"

translate english simple_examples_characters_7d6237ae:
	# rn "И если ты всё ещё не заглянул в [sts]mods/common/characters.rpy[ste], то ответ - вот так."
	rn "And if you haven't looked into [sts]mods/common/characters.rpy[ste] yet, then the answer is right here."

translate english simple_examples_characters_b1aac456:
	# rn "Позже задать персонажу другое имя можно так: [text_code('set_name(\"np\", \"Прохожий\")')]."
	rn "You can later set the character a different name like this: [text_code('set_name(\"np\", \"Passerby\")')]."

translate english simple_examples_characters_5179b664:
	# extend " Обрати внимание, что [sts]np[ste] здесь - тоже в кавычках."
	extend " Note that [sts]np[ste] here is also in quotes."

translate english simple_examples_characters_c2053a7b:
	# rn "Вообще-то класс [sts]Character[ste] может принимать огромную кучу параметров, но в этом маленьком туториале для них нет места."
	rn "The class [sts]Character[ste] can actually accept a huge number of parameters, but there is no place for them in this small tutorial."

translate english simple_examples_characters_a0bd51f0:
	# rn "Так что в случае необходимости смотри подробности в документации, особенно обрати внимание на статью [text_code('Интерфейс (gui)')]."
	rn "So if you need details, check the documentation, especially the article [text_code('Interface (gui)')]."

translate english simple_examples_characters_05a34769:
	# rn "Теперь поговорим о стандартных персонажах."
	rn "Now let's talk about standard characters."

translate english simple_examples_characters_5f3b5c62:
	# rn "[sts]narrator[ste] - рассказчик, используется по умолчанию, когда ничего не указано."
	rn "[sts]narrator[ste] - narrator, used by default when nothing is specified."

translate english simple_examples_characters_9377eb3b:
	# rn "Пример: [sts]\"Светит яркое солнце.\"[ste]"
	rn "Example: [sts]\"Bright sun shines.\"[ste]"

translate english simple_examples_characters_9978c27e:
	# rn "А выглядит это так:"
	rn "It looks like this:"

translate english simple_examples_characters_e8af64be:
	# "Светит яркое солнце."
	"A bright sun shines."

translate english simple_examples_characters_b361903e:
	# rn "[sts]th[ste] - мысли главного героя твоего произведения."
	rn "[sts]th[ste] - thoughts of the main character of your work."

translate english simple_examples_characters_4060d36a:
	# rn "[sts]th \"Мне было жарко.\"[ste]"
	rn "[sts]th \"I was hot.\"[ste]"

translate english simple_examples_characters_bee31113:
	# th "Мне было жарко."
	th "I was hot."

translate english simple_examples_characters_dc7b8779:
	# rn "[sts]me[ste] - слова от лица главного героя (нужно регистрировать самостоятельно)."
	rn "[sts]me[ste] - words from the point of view of the main character (you need to register it yourself)."

translate english simple_examples_characters_6cebea55:
	# rn "[sts]me \"Здравствуйте.\"[ste]"
	rn "[sts]me \"Hello.\"[ste]"

translate english simple_examples_characters_dbea9e1c:
	# me "Здравствуйте."
	me "Hello."

translate english simple_examples_characters_ab46335e:
	# rn "Ну и [sts]extend[ste] - в отличие от всех остальных, не стирает текст, а продолжает его дальше от лица предыдущего говорящего."
	rn "And that's [sts]extend[ste] - unlike everyone else, it does not erase the text, but continues it further from the point of view of the previous speaker."

translate english simple_examples_characters_a7e444ba:
	# rn "[sts]extend \" Дайте 2 мороженных.\"[ste]"
	rn "[sts]extend \" Give me 2 ice creams, please.\"[ste]"

translate english simple_examples_characters_183584ce:
	# extend " Дайте 2 мороженных."
	extend " Give me 2 ice creams, please."

translate english simple_examples_interpolation_2845c58c:
	# rn "Текст внутри квадратных скобок интерполируется."
	rn "Text inside square brackets is interpolated."

translate english simple_examples_interpolation_f1c420e1:
	# rn "Звучит сложно, но посмотри сам, [sts]\"2 + 3 = [[2 + 3]\"[ste] выведет вот это:"
	rn "Sounds complicated, but take a look yourself, [sts]\"2 + 3 = [[2 + 3]\"[ste] will output this:"

translate english simple_examples_interpolation_1c8efaee:
	# extend " 2 + 3 = [2 + 3]."
	extend " 2 + 3 = [2 + 3]."

translate english simple_examples_interpolation_b7f5482e:
	# rn "Хотя обычно это используют для вывода переменной. Что-то типа этого."
	rn "Although usually it's used for variable output. Something like this."

translate english simple_examples_interpolation_80b8b264:
	# rn "Кроме выполнения содержимого в качестве python-кода, интерполяция позволяет компактным образом делать некоторые вещи."
	rn "Besides executing content as Python code, interpolation allows you to do certain things compactly."

translate english simple_examples_interpolation_6a86220e:
	# rn "Например, [sts][['qwerty'!u][ste] выведет ['qwerty'!u]. Другие \"модификаторы\" ты можешь найти в документации."
	rn "For example, [sts][['qwerty'!u][ste] will output ['qwerty'!u]. You can find other \"modifiers\" in the documentation."

translate english simple_examples_interpolation_e9307b05:
	# rn "Ну а если нужно вывести сам символ [sts][[[ste], чтобы движок не посчитал его началом кода для интерполяции, его всего лишь нужно написать 2 раза: [sts][[[[[ste]."
	rn "And if you need to output the literal character [sts][[[ste], so that the engine does not treat it as the start of an interpolation code, you simply need to write it twice: [sts][[[[[ste]."

translate english simple_examples_tags_a40e3d6f:
	# rn "Теперь пришло время поговорить о тэгах."
	rn "Now it's time to talk about tags."

translate english simple_examples_tags_2422daae:
	# rn "Есть простые:"
	rn "There are simple ones:"

translate english simple_examples_tags_c8687f43:
	# extend " [in_tags('i|курсив|/i')]"
	extend " [in_tags('i|italics|/i')]"

translate english simple_examples_tags_4c98ea6e:
	# extend ", [in_tags('b|\"полужирность\"|/b')]"
	extend ", [in_tags('b|\"boldness\"|/b')]"

translate english simple_examples_tags_ede8a8a4:
	# extend ", [in_tags('u|подчёркивание|/u')]"
	extend ", [in_tags('u|underlining|/u')]"

translate english simple_examples_tags_6695f1e6:
	# extend ", [in_tags('s|зачёркивание|/s')]"
	extend ", [in_tags('s|strikethrough|/s')]"

translate english simple_examples_tags_00ab694c:
	# extend " и [in_tags('plain|обычный|/plain')]."
	extend " and [in_tags('plain|~plain|/plain')]."

translate english simple_examples_tags_c9a5eae4:
	# rn "Как ты можешь заметить, для указания на начало тэга используется символ [sts]{{[ste]."
	rn "As you can see, the symbol [sts]{{[ste] is used to indicate the start of a tag."

translate english simple_examples_tags_f7ffb898:
	# extend " И как и в случае с [sts][[[ste], чтобы вывести его как символ, его тоже нужно продублировать: [sts]{{{{[ste]."
	extend " And as with [sts][[[ste], to output it as a symbol, it also needs to be doubled: [sts]{{{{[ste]."

translate english simple_examples_tags_a8ec53d5:
	# rn "Кстати, будь внимателен: тэги должны закрываться в обратном порядке (относительно их открытия)."
	rn "By the way, be careful: tags must be closed in reverse order (relative to their opening)."

translate english simple_examples_tags_547f45bf:
	# rn "Вот так писать нельзя: [in_tags('b|123|i|456|/b|789|/i', real_tags = False)]."
	rn "You cannot write like this: [in_tags('b|123|i|456|/b|789|/i', real_tags = False)]."

translate english simple_examples_tags_a24f7de7:
	# rn "Есть тэги чуть сложнее:"
	rn "There are tags that are a bit more complex:"

translate english simple_examples_tags_3d352f79:
	# extend " [in_tags('color=#08F|цвет|/color')]"
	extend " [in_tags('color=#08F|~color|/color')]"

translate english simple_examples_tags_5c4bec46:
	# extend ", [in_tags('outlinecolor=#7CF|обводка|/outlinecolor')]"
	extend ", [in_tags('outlinecolor=#7CF|border|/outlinecolor')]"

translate english simple_examples_tags_ffb243bd:
	# extend "\nи картинка [in_tags('image=images/anim/planet_light.png')]."
	extend "\nand image [in_tags('image=images/anim/planet_light.png')]."

translate english simple_examples_tags_ded2f7e6:
	# rn "Обрати внимание, что внутри [sts]{{[ste]тэга[sts]}[ste] не используются кавычки."
	rn "Note that inside [sts]{{[ste]tag[sts]}[ste] no quotes are used."

translate english simple_examples_tags_610413bb:
	# rn "Также есть возможность использовать другой [in_tags('font=FixEx3|шрифт|/font')]."
	rn "There is also the possibility to use another [in_tags('font=FixEx3|~font|/font')]."

translate english simple_examples_tags_e3db88df:
	# extend " Шрифты лежат в папке [sts]resources/fonts/[ste]."
	extend " Fonts are located in the folder [sts]resources/fonts/[ste]."

translate english simple_examples_tags_b9503a7e:
	# rn "Можно поменять и размер: [in_tags('size=50|текст|/size')]."
	rn "You can also change the size: [in_tags('size=50|text|/size')]."

translate english simple_examples_tags_1a500c08:
	# rn "Однако, изначальный размер текста зачастую зависит от размеров окна игры, поэтому имеет смысл задавать относительный размер."
	rn "However, the original text size often depends on the game window size, so it makes sense to specify a relative size."

translate english simple_examples_tags_f610a503:
	# rn "Сделать это можно с помощью символов [sts]+[ste], [sts]-[ste] и [sts]*[ste] после знака [sts]=[ste]:"
	rn "This can be done using symbols [sts]+[ste], [sts]-[ste] and [sts]*[ste] after the sign [sts]=[ste]:"

translate english simple_examples_tags_a3531b2f:
	# rn "Обычный, [in_tags('size=+7|увеличенный|/size| и |size=*0.8|уменьшенный|/size')] тексты."
	rn "Normal, [in_tags('size=+7|enlarged|/size| and |size=*0.8|reduced|/size')] texts."

translate english simple_examples_tags_e1bfa5a1:
	# rn "Таким же образом работает изменение прозрачности:\n[in_tags('alpha=0.5|полупрозрачный текст|/alpha')]."
	rn "Transparency changes work similarly:\n[in_tags('alpha=0.5|semi-transparent text|/alpha')]."

translate english simple_examples_tags_4a92c6b7:
	# rn "Ну и последний тэг - пауза [sts]{{w}[ste], работает только в этом окне диалога."
	rn "And finally, the last tag - pause [sts]{{w}[ste], which works only in this dialogue window."

translate english simple_examples_tags_cd51e8fd:
	# rn "Как и у тэга [sts]image[ste], у него отсутствует \"закрывающая\" часть."
	rn "Like with the [sts]image[ste] tag, it lacks a \"closing\" part."

translate english simple_examples_tags_a0f8c72a:
	# rn "Пауза без указания времени - \"бесконечна\", а указать время{w=0.5}, в секундах{w=0.5}, можно так: [sts]{{w=0.5}[ste]."
	rn "A pause without a specified time is \"infinite\", and to specify time{w=0.5}, in seconds{w=0.5}, you can do so like this: [sts]{{w=0.5}[ste]."

translate english simple_examples_tags_bb6d188c:
	# rn "Пауза любой длительности может быть пропущена игроком любым способом, в том числе нажатием на [sts]Пробел[ste] или [sts]Enter[ste]."
	rn "Any duration pause can be skipped by the player in any way, including pressing [sts]Space[ste] or [sts]Enter[ste]."

translate english simple_examples_python_610a8340:
	# rn "Перейдём к способам запуска python-кода в сценарии или в [sts]init[ste]-секциях."
	rn "Let's move on to methods for running Python code in a scenario or in [sts]init[ste] sections."

translate english simple_examples_python_4b283d6f:
	# rn "Вот так выглядит универсальный способ."
	rn "Here is how the universal method looks."

translate english simple_examples_python_98602bb7:
	# rn "Если же нужно запустить лишь одну строку кода, то есть более компактный вариант записи."
	rn "If you only need to run a single line of code, there is a more compact way to write it."

translate english simple_examples_python_4c35dcd1:
	# rn "Здесь [sts]$[ste] - команда, запускающая то, что идёт дальше в строке, и пробел после команды обязателен, как и всегда."
	rn "Here [sts]$[ste] is the command that runs whatever follows in the line, and the space after the command is mandatory, as always."

translate english simple_examples_python_e2d02f61:
	# rn "Кстати, если вдруг интересно, вот эти 2 строки делают одно и то же."
	rn "By the way, if you're curious, these two lines do exactly the same thing."

translate english simple_examples_python_d604958c:
	# rn "Просто на случай, если тебе понадобится отобразить реплику из python-кода."
	rn "Simply in case you need to display a reply from Python code."

translate english simple_examples_python_c9f9690e:
	# rn "Ну и заканчивая эту тему: функция [sts]print[ste] переопределена так, что её вывод можно увидеть в файле [sts]var/log.txt[ste]."
	rn "Well, finishing this topic: the function [sts]print[ste] is redefined so that its output can be seen in the file [sts]var/log.txt[ste]."

translate english simple_examples_python_397b9445:
	# rn "Иногда это бывает удобно, хотя для \"одноразовых\" проверок удобнее использовать консоль движка, открываемую по [sts]Shift+O[ste]."
	rn "Sometimes it's convenient, although for \"one-time\" checks it's more convenient to use the engine console, opened by [sts]Shift+O[ste]."

translate english simple_examples_conditions_9b19c32c:
	# rn "В играх бывают развилки, и реализуются они с помощью условий."
	rn "Games can have branches, and they are implemented using conditions."

translate english simple_examples_conditions_cbcaf702:
	# rn "Условия работают точно также, как и в питоне."
	rn "Conditions work exactly the same as in Python."

translate english simple_examples_conditions_45d1e4ff:
	# rn "Всё начинается с ключевого слова [sts]if[ste], после него идёт нужная проверка."
	rn "Everything starts with the keyword [sts]if[ste], followed by the necessary check."

translate english simple_examples_conditions_0474c310:
	# rn "Если она выполняется успешно, то идёт выполнение вложенных команд, а дальнейшие блоки [sts]elif[ste] и [sts]else[ste] игнорируются."
	rn "If it executes successfully, then nested commands are being executed, and further blocks [sts]elif[ste] and [sts]else[ste] are ignored."

translate english simple_examples_conditions_6c7e9914:
	# rn "Если же проверка не выполняется, то рассматривается следующее условие."
	rn "If the condition does not execute, then the next condition is considered."

translate english simple_examples_conditions_84784e9e:
	# rn "Ты можешь использовать любое количество веток [sts]elif[ste], в том числе и ноль."
	rn "You can use any number of branches [sts]elif[ste], including zero."

translate english simple_examples_conditions_e91accc5:
	# rn "Наконец, если ни одно из условий не было исполнено, то управление переходит к блоку [sts]else[ste], который тоже может отсутствовать."
	rn "Finally, if none of the conditions were executed, control passes to the [sts]else[ste] block, which may also be absent."

translate english simple_examples_conditions_1dd19199:
	# rn "А потом исполнение команд движется дальше.{w=0.5} Как видишь, ничего сложного."
	rn "And then command execution continues.{w=0.5} As you can see, nothing complicated."

translate english simple_examples_conditions_f7e654f0:
	# rn "Хотя некоторые новички путают присваивание ([sts]=[ste]) и сравнение ([sts]==[ste])."
	rn "Although some beginners confuse assignment ([sts]=[ste]) with comparison ([sts]==[ste])."

translate english simple_examples_conditions_e44a4921:
	# rn "Но ты же не будешь, правда?"
	rn "But you wouldn't, right?"

translate english simple_examples_conditions_944822e1:
	# rn "Кстати, команда [sts]pass[ste] здесь означает \"ничего не делать\". Пусть и редко, но она бывает полезна, когда в блоке нет осмысленных вложенных команд."
	rn "By the way, the [sts]pass[ste] command here means \"do nothing\". It is useful, though rarely, when there are no meaningful nested commands in the block."

translate english simple_examples_conditions_4665992e:
	# rn "А ещё здесь представлен комментарий, который игнорируется движком и питоном: от символа [sts]#[ste] и до конца строки."
	rn "And also, a comment is presented here, which is ignored by the engine and Python: from the symbol [sts]#[ste] to the end of the line."

translate english simple_examples_cycles_14f4e03b:
	# rn "Есть цикл [sts]while[ste] - такой же, как в питоне."
	rn "There is a [sts]while[ste] loop - just like in Python."

translate english simple_examples_cycles_c4ed5797:
	# rn "Так как он редко используется вне питона и вряд ли понадобится новичкам, то я просто покажу очевидный пример без особых объяснений."
	rn "Since it is rarely used outside of Python and is unlikely to be needed by beginners, I will simply show an obvious example without special explanations."

translate english simple_examples_cycles_7f8b1e2b:
	# rn "Как и в питоне, внутри цикла поддерживаются команды [sts]continue[ste] и [sts]break[ste], а после - блок [sts]else[ste]."
	rn "As in Python, inside a loop the commands [sts]continue[ste] and [sts]break[ste] are supported, and after them - a block [sts]else[ste]."

translate english simple_examples_cycles_bd319991:
	# rn "Цикл [sts]for[ste] не поддерживается из-за того, что он использует итераторы, не все из которых могут быть сохранены во время сохранений."
	rn "The [sts]for[ste] loop is not supported because it uses iterators, not all of which can be preserved during saves."

translate english simple_examples_cycles_e9d5405e:
	# rn "Ну а теперь снова вернёмся к часто используемым вещам."
	rn "Well, now let's return again to often used things."

translate english simple_examples_label_transitions_417a616e:
	# rn "Чтобы процесс создания игры был прост и удобен, мы ни в коем случае не должны писать весь сценарий в одном файле, и тем более - в одной метке."
	rn "To make the game creation process simple and convenient, we must under no circumstances write the entire script in one file, and even more so - in one label."

translate english simple_examples_label_transitions_5410bf18:
	# rn "А раз у нас много меток, то нужны и способы перехода между ними."
	rn "And since we have many labels, we also need ways to jump between them."

translate english simple_examples_label_transitions_0a8b00ad:
	# rn "Команда [sts]jump[ste] говорит движку \"прыгнуть\" на указанную метку и завершить игру после того, как все её команды будут выполнены."
	rn "The [sts]jump[ste] command tells the engine to \"jump\" to the specified label and finish the game after all its commands have been executed."

translate english simple_examples_label_transitions_1220b79c:
	# rn "Ещё есть команда [sts]call[ste], которая после выполнения метки возвращает управление назад и продолжает исполнять дальнейшие команды."
	rn "There is also the [sts]call[ste] command, which returns control back after executing a label and continues to execute next commands."

translate english simple_examples_label_transitions_39be5d94:
	# rn "Обе эти команды поддерживают \"вычисление\" метки python-кодом, если есть ключевое слово [sts]expression[ste]."
	rn "Both of these commands support \"evaluation\" of labels via Python code if the keyword [sts]expression[ste] is present."

translate english simple_examples_label_transitions_961254a8:
	# extend "\nПример: [sts]call expression \"my_label_name_\" + str(num)[ste]."
	extend "\nExample: [sts]call expression \"my_label_name_\" + str(num)[ste]."

translate english simple_examples_label_transitions_e8393298:
	# rn "Кроме того, эти команды можно и напрямую вызвать из кода:\n[sts]$ renpy.jump('some_label')[ste]."
	rn "Additionally, these commands can be called directly from code:\n[sts]$ renpy.jump('some_label')[ste]."

translate english simple_examples_label_transitions_43037e9d:
	# rn "И, конечно, есть команда [sts]return[ste] для досрочного завершения текущей метки и возврата назад, если есть куда возвращаться."
	rn "And, of course, there is the [sts]return[ste] command for early termination of the current label and returning back if there is somewhere to return to."

translate english simple_examples_label_transitions_b2fac0e8:
	# rn "Вернёмся к тексту."
	rn "Let's return to the text."

translate english simple_examples_text_displaying_baaad43c:
	# rn "Команды [sts]window show[ste] и [sts]window hide[ste] отвечают за показ и скрытие диалогового окна."
	rn "The [sts]window show[ste] and [sts]window hide[ste] commands handle showing and hiding the dialog window."

translate english simple_examples_text_displaying_0b106838:
	# rn "Как-то так. Кстати, при выводе текста диалоговое окно показывается автоматически."
	rn "Well, that's how it is. By the way, the dialog window is shown automatically when text is outputted."

translate english simple_examples_text_displaying_562cb78c:
	# rn "Есть два режима отображения текста."
	rn "There are two text display modes."

translate english simple_examples_text_displaying_5992f576:
	# rn "Первый, стандартный, используется сейчас и называется [sts]ADV[ste]."
	rn "The first, standard mode, is used currently and is called [sts]ADV[ste]."

translate english simple_examples_text_displaying_f0ce82b0:
	# rn "Также есть [sts]NVL[ste]-режим, который выглядит вот так."
	rn "There is also the [sts]NVL[ste] mode, which looks like this:"

translate english simple_examples_text_displaying_48f21050:
	# rn "Он включается вызовом [sts]$ set_mode_nvl()[ste]."
	rn "It is enabled by calling [sts]$ set_mode_nvl()[ste]."

translate english simple_examples_text_displaying_57e0a64d:
	# rn "Как видишь, здесь не происходит автоматического удаления предыдущих реплик."
	rn "As you can see, previous lines are not automatically deleted here."

translate english simple_examples_text_displaying_58b2a3ed:
	# rn "Зато это можно сделать вручную с помощью команды [text_code('nvl clear')]."
	rn "However, this can be done manually using the command [text_code('nvl clear')]."

translate english simple_examples_text_displaying_93f4da3d:
	# rn "Обрати внимание, что это не python-код, поэтому значок [sts]$[ste] здесь не нужен."
	rn "Note that this is not Python code, so the symbol [sts]$[ste] is not needed here."

translate english simple_examples_text_displaying_55f099f3:
	# rn "Собственно, вот так выглядит очистка экрана."
	rn "Essentially, this is how the screen clearing looks."

translate english simple_examples_text_displaying_f952ca23:
	# rn "Нужно не забывать это делать, чтобы текст не вылезал за данные ему границы."
	rn "You must remember to do this so the text does not overflow its assigned boundaries."

translate english simple_examples_text_displaying_8b28a953:
	# rn "Вернуться в стандартный [sts]ADV[ste]-режим можно с помощью [text_code('$ set_mode_adv()')]."
	rn "You can return to the standard [sts]ADV[ste]-mode using [text_code('$ set_mode_adv()')]."

translate english simple_examples_menu_a5cebee2:
	# rn "Важной частью многих игр является меню выбора из нескольких вариантов."
	rn "An important part of many games is a menu with multiple choices."

translate english simple_examples_menu_72985b42:
	# rn "В общем-то, тут всё просто: при выборе пункта меню выполняются его вложенные команды."
	rn "In general, it's quite simple here: when a menu item is selected, its nested commands are executed."

translate english simple_examples_menu_1398a2ce:
	# rn "Как можно заметить, некоторые пункты меню можно не отображать, если не соблюдается нужное условие."
	rn "As you can see, some menu items may not be displayed if the required condition is not met."

translate english simple_examples_menu_9d8f24f2:
	# rn "А ещё можно передать пустую строку - это создаст зазор между пунктами меню."
	rn "You can also pass an empty string - this will create a gap between menu items."

translate english simple_examples_menu_c0a2a9cf:
	# rn "Ну а как выглядит меню выбора, ты уже видел в начале этого урока."
	rn "And as for how the choice menu looks, you've already seen it at the beginning of this lesson."

translate english simple_examples_pause_67d1eeff:
	# rn "Последняя команда в этом уроке - пауза."
	rn "The last command in this lesson is pause."

translate english simple_examples_pause_66485769:
	# rn "Раньше я говорила про тэг паузы [sts]{{w}[ste], но он работает только посреди текста, а это не всегда то, что нужно."
	rn "I previously mentioned the pause tag [sts]{{w}[ste], but it only works in the middle of text, which isn't always what's needed."

translate english simple_examples_pause_b7a15974:
	# rn "Например, иногда нужно показать изображение, подождать две секунды и сменить его на другое изображение."
	rn "For example, sometimes you need to display an image, wait for two seconds, and then switch it to another image."

translate english simple_examples_pause_389b94d6:
	# rn "Как видишь, эту команду можно вызвать и из python-кода, если вдруг так будет удобнее."
	rn "As you can see, this command can also be called from Python code, if suddenly it will be more convenient."

translate english simple_examples_pause_6a667b5b:
	# rn "Кстати, здесь точно также любая пауза может быть пропущена."
	rn "By the way, here any pause can be skipped too."

translate english simple_examples_pause_a57994d7:
	# rn "Однако следует помнить, что пауза начинается после полного выполнения python-кода, а не прямо посреди него."
	rn "However, keep in mind that the pause begins after the full execution of Python code, not right in the middle of it."

translate english simple_examples_pause_bf242579:
	# rn "Таким образом, вот этот код сначала вызовет функцию [sts]print[ste], а уже потом поставит сценарий на секундную паузу."
	rn "Thus, this code will first call the function [sts]print[ste], and then set the scenario to a one-second pause."

translate english simple_examples_pause_44ad7463:
	# rn "Конечно, это не играет никакой роли, если код состоит из одной строки."
	rn "Of course, this doesn't matter if the code is one line."

translate english simple_examples_pause_19f2dbdf:
	# rn "Напоследок поговорим о модах."
	rn "Finally, let's talk about mods."

translate english simple_examples_mods_71205ed6:
	# rn "В мире Ren-Engine \"мод\" - это папка с [sts]rpy[ste]-файлами и нужными ресурсами."
	rn "In the Ren-Engine world, a \"mod\" is a folder containing [sts]rpy[ste] files and necessary resources."

translate english simple_examples_mods_88e7b58d:
	# rn "Понятия \"мод\" и \"непосредственно сама игра\" объединены в одно, чтобы не создавать новые сущности там, где в этом нет необходимости."
	rn "The concept of a \"mod\" and \"the game itself\" are combined into one to avoid creating new entities where it is not necessary."

translate english simple_examples_mods_228d659e:
	# rn "Ведь основная игра может делать ровно то же самое, что и моды."
	rn "After all, the main game can do exactly the same thing as mods."

translate english simple_examples_mods_60c17da8:
	# rn "Все моды лежат в [sts]resources/mods/[ste]."
	rn "All mods are located in [sts]resources/mods/[ste]."

translate english simple_examples_mods_2c1bf6ee:
	# rn "В начале движок запускает мод основного меню [sts]main_menu[ste], из него возможны переходы в другие моды, например - в саму игру."
	rn "At startup, the engine launches the main menu mod [sts]main_menu[ste]; from there, transitions to other mods are possible, for example - into the game itself."

translate english simple_examples_mods_6d64a5b8:
	# rn "У каждого мода есть отдельная папка. Это нужно для поддержания порядка, а также для отсутствия конфликтов и замещения файлов."
	rn "Each mod has its own separate folder. This is necessary for maintaining order, avoiding conflicts, and preventing file replacement."

translate english simple_examples_mods_497d6aa6:
	# rn "Если мод должен отображаться в списке модов, то он также должен содержать файл [sts]name[ste] (без расширений вроде [sts].txt[ste]) с названием мода."
	rn "If a mod must appear in the list of mods, it must also contain a file [sts]name[ste] (without extensions like [sts].txt[ste]) with the mod's name."

translate english simple_examples_mods_83971997:
	# rn "Конечно, это не нужно для модов главного меню или основной игры."
	rn "Of course, this is not required for the main menu or the main game mods."

translate english simple_examples_mods_20057f6e:
	# rn "Для запуска мода нужно вызвать функцию [sts]start_mod('mod_dir')[ste], в которую передаётся имя папки мода."
	rn "To launch a mod, you need to call the function [sts]start_mod('mod_dir')[ste], passing the mod folder's name as an argument."

translate english simple_examples_mods_9c005878:
	# rn "Ты можешь вызвать её даже из [sts]init[ste]-блока главного меню, что бывает удобно во время разработки."
	rn "You can even call it from the [sts]init[ste] block of the main menu, which is convenient during development."

translate english simple_examples_mods_95527cc0:
	# rn "Наконец, что происходит при запуске какого-либо мода?"
	rn "Finally, what happens when a mod is launched?"

translate english simple_examples_mods_6bab61f7:
	# rn "Идёт разбор всех [sts]rpy[ste]-файлов движка в [sts]Ren-Engine/rpy/[ste], общих файлов игры в [sts]mods/common/[ste] и файлов мода в [sts]mods/mod_dir/[ste]."
	rn "The engine parses all [sts]rpy[ste]-files from [sts]Ren-Engine/rpy/[ste], game common files in [sts]mods/common/[ste], and mod files in [sts]mods/mod_dir/[ste]."

translate english simple_examples_mods_af86aec3:
	# rn "Выполняются все [sts]init[ste]-блоки в нужном порядке."
	rn "All [sts]init[ste] blocks are executed in the correct order."

translate english simple_examples_mods_6ca54000:
	# rn "Отображаются все скрины из списка [sts]start_screens[ste]."
	rn "All screens from the list [sts]start_screens[ste] are displayed."

translate english simple_examples_mods_ffe980bc:
	# rn "И происходит переход на метку [sts]start[ste], если она есть."
	rn "Then, if it exists, a jump to the label [sts]start[ste] occurs."

translate english simple_examples_mods_9e6d32c9:
	# extend " После выполнения последней команды метки идёт возврат в главное меню."
	extend " After the last command of the label is executed, control returns to the main menu."

translate english simple_examples_mods_1955b08d:
	# rn "Кстати, [sts]init[ste]-блоки имеют 2 необязательных параметра."
	rn "By the way, [sts]init[ste] blocks have 2 optional parameters."

translate english simple_examples_mods_a85585b5:
	# rn "Приоритет - просто число, чем оно меньше, тем раньше будет выполнен этот блок."
	rn "Priority is simply a number; the smaller it is, the sooner this block will be executed."

translate english simple_examples_mods_badff5c5:
	# rn "А [sts]python[ste] указывает, что всё содержимое блока является python-кодом."
	rn "And [sts]python[ste] indicates that all content of the block is Python code."

translate english simple_examples_mods_e4f8b3b1:
	# rn "Вот и всё!"
	rn "That's all!"

translate english simple_examples_mods_2d519861:
	# rn "На этом этот огромный урок подошёл к концу."
	rn "This huge lesson comes to an end here."

translate english simple_examples_mods_b72b1394:
	# rn "Ты молодец, если смог его осилить и всё понять!"
	rn "You are great if you were able to master it and understand everything!"

translate english simple_examples_mods_89a0b800:
	# rn "А если нет - тоже не беда, ведь его всегда можно просмотреть ещё раз."
	rn "What if not? That's no problem either, as it can always be reviewed again."

translate english simple_examples_mods_6dc929f6:
	# rn "До встречи в следующем уроке!"
	rn "See you in the next lesson!"


translate english strings:
	
	old 'Начало'
	new 'Start'
	
	old 'Персонажи'
	new 'Characters'
	
	old 'Интерполяция'
	new 'Interpolation'
	
	old 'Тэги'
	new 'Tags'
	
	old 'Условия'
	new 'Conditions'
	
	old 'Циклы'
	new 'Loops'
	
	old 'Переходы по меткам'
	new 'Jumps to labels'
	
	old 'Отображение текста'
	new 'Displaying text'
	
	old 'Меню'
	new 'Menu'
	
	old 'Пауза'
	new 'Pause'
	
	old 'Моды'
	new 'Mods'
	
	
	old 'Новая Персона'
	new 'New Person'
	
	old 'Имя'
	new 'Name'
	
	old 'Привет'
	new 'Hello'
	
	old 'Текст.'
	new 'Text.'
	
	old 'Как дела?'
	new 'How are you?'
	
	old 'Нормально.'
	new 'Fine.'
	
	old 'Хорошо. А у тебя?'
	new 'Good. And you?'
	
	old 'Я думал прогуляться.'
	new "I thought I'd take a walk."
	
	old 'Составишь компанию?'
	new 'Would you like to join me?'
	
	old 'ошибочное использование'
	new 'incorrect usage'
	
	old 'Напечатает:'
	new 'Will print:'
	
	old 'Начальный текст.'
	new 'Initial text.'
	
	old 'Текст из some_label.'
	new 'Text from some_label.'
	
	old 'Продолжение текста.'
	new 'Continuation of the text'
	
	old 'Куда бы мне пойти?'
	new 'Where should I go?'
	
	old 'Никуда, останусь сидеть дома.'
	new "Nowhere, I'll stay at home."
	
	old 'Пора на работу.'
	new "It's time to go to work."
	
	old 'В парк!'
	new 'To the park!'
