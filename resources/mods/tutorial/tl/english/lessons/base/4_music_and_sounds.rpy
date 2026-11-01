translate english music_and_sounds_9689e37d:
	# rn "Настало время разобраться с музыкой!"
	rn "It's time to get down to music!"

translate english music_and_sounds_0b9bb3ef:
	# rn "Самый простой пример воспроизведения музыки выглядит вот так."
	rn "The simplest example of playing music looks like this."

translate english music_and_sounds_a0811747:
	# rn "В начале идёт команда [sts]play[ste], потом указывается канал воспроизведения, а затем - путь к файлу, который нужно запустить."
	rn "First comes the command [sts]play[ste], then the playback channel is specified, and next - the path to the file that needs to be played."

translate english music_and_sounds_761eaa15:
	# rn "Прежде всего стоит сказать, что это - именно максимально упрощённый пример, и писать прямо так - плохая идея."
	rn "First and foremost, it should be noted that this is exactly the most simplified example, and writing it directly like this is a bad idea."

translate english music_and_sounds_856ebd94:
	# rn "Проблема в том, что при указании пути к файлу напрямую, его придётся менять по всей игре, если позже ты решишь заменить его другим файлом."
	rn "The problem is that when you specify the file path directly, you'll have to change it throughout the entire game, if you later decide to replace it with another file."

translate english music_and_sounds_b1d8b0e9:
	# rn "А это мало того, что неудобно, так о некоторых местах можно и вообще забыть!"
	rn "And that's not only inconvenient; you can also simply forget some places!"

translate english music_and_sounds_11dbf65e:
	# rn "Чтобы такого не было, этот путь нужно куда-то записать в [sts]init[ste]-блоке, а при воспроизведении - доставать его оттуда."
	rn "To avoid it, this path needs to be stored somewhere in an [sts]init[ste] block, and then retrieved from there during playback."

translate english music_and_sounds_61ba0492:
	# rn "В случае изменения пути к файлу, теперь его нужно будет исправить лишь в одном месте, и всё гарантированно продолжит работать."
	rn "In case the file path changes, you now only need to fix it in one place, and everything will continue to work reliably."

translate english music_and_sounds_00a6e765:
	# rn "Здорово, правда? {w=0.5}Но для этого нужно самую малость усложнить код."
	rn "Great, right? {w=0.5}But for this, you'll need to complicate the code just a little bit."

translate english music_and_sounds_d4ec9a25:
	# rn "Кроме того, уметь проигрывать музыку только так - не очень-то полезно."
	rn "Additionally, being able to play music only like this isn't very useful."

translate english music_and_sounds_c661e16f:
	# rn "Как есть параметры у команды [sts]show[ste] из прошлого урока, так они есть и у команды [sts]play[ste]."
	rn "Just as there are parameters for the [sts]show[ste] command from the previous lesson, there are also parameters for the [sts]play[ste] command."

translate english music_and_sounds_67e9ca6d:
	# rn "Я расскажу о двух самых важных."
	rn "I'll tell you about two of the most important ones."

translate english music_and_sounds_f4125ec4:
	# rn "Добавление [sts]fadeout 1[ste] в конце укажет, что сначала нужно плавно, за секунду, снизить громкость текущей мелодии до нуля, и лишь потом начать новую."
	rn "Adding [sts]fadeout 1[ste] at the end will indicate that first you need to smoothly reduce the volume of the current melody to zero over one second, and only then start the new one."

translate english music_and_sounds_5694b909:
	# rn "Точно также есть параметр [sts]fadein 1.5[ste], который указывает время плавного повышения громкости."
	rn "There's also a parameter [sts]fadein 1.5[ste], which specifies the duration of smooth volume increase."

translate english music_and_sounds_90f1ac17:
	# rn "Конечно, их можно использовать и одновременно."
	rn "Of course, you can use them simultaneously."

translate english music_and_sounds_de36facf:
	# rn "Теперь поговорим о каналах воспроизведения."
	rn "Now let's talk about playback channels."

translate english music_and_sounds_da00d13b:
	# rn "Изначально есть 3 категории каналов."
	rn "Initially, there are three categories of channels."

translate english music_and_sounds_218e1769:
	# rn "Музыка: с одним каналом [sts]music[ste], в котором указанные файлы после завершения начинаются с начала."
	rn "Music: with one channel [sts]music[ste], where specified files start from the beginning after completion."

translate english music_and_sounds_0739fdd2:
	# rn "Звуки: [sts]sound[ste], [sts]sound2[ste] и [sts]sound3[ste] - эти каналы отвечают за однократное воспроизведение. Полезно для звука открытия двери, например."
	rn "Sounds: [sts]sound[ste], [sts]sound2[ste], and [sts]sound3[ste] - these channels handle single playback. Useful for sounds like opening a door, for example."

translate english music_and_sounds_5570203e:
	# rn "На одном канале может проигрываться только 1 файл, поэтому таких каналов сделано сразу 3 штуки."
	rn "Only one file can be played on a single channel, which is why three such channels are provided."

translate english music_and_sounds_641b1884:
	# rn "Обычно звуки принято хранить в [sts]sound/sfx/[ste]."
	rn "Usually, sounds are stored in [sts]sound/sfx/[ste]."

translate english music_and_sounds_9fa359fa:
	# rn "Ну и каналы для окружения: [sts]ambience[ste], [sts]sound_loop[ste], [sts]sound_loop2[ste] и [sts]sound_loop3[ste]."
	rn "And there are environment channels: [sts]ambience[ste], [sts]sound_loop[ste], [sts]sound_loop2[ste], and [sts]sound_loop3[ste]."

translate english music_and_sounds_e717289b:
	# rn "Эти каналы повторяют воспроизведение своих файлов, как и [sts]music[ste]."
	rn "These channels repeat playback of their files, just like [sts]music[ste]."

translate english music_and_sounds_8ca887d9:
	# rn "Отличие в том, что у окружения есть отдельный параметр громкости на экране настроек."
	rn "The difference is that the environment has a separate volume parameter in the screen settings."

translate english music_and_sounds_19a327ce:
	# rn "К примеру, пользователю может быть удобно снизить громкость сверчков, оставив музыку на прежнем уровне."
	rn "For example, it might be convenient for the user to lower the volume of crickets while keeping the music at its original level."

translate english music_and_sounds_cd64d279:
	# rn "Звуки окружения обычно складируются в папке [sts]sound/ambience/[ste]."
	rn "Environment sounds are usually stored in the [sts]sound/ambience/[ste] folder."

translate english music_and_sounds_47986a94:
	# rn "После того, как мы научились запускать музыку, было бы неплохо научиться и останавливать её, не так ли?"
	rn "After we've learned how to start playing music, wouldn't it be nice to also learn how to stop it?"

translate english music_and_sounds_a661a7e4:
	# rn "Как видишь, это делается очень просто: команда [sts]stop[ste] и название канала."
	rn "As you can see, this is done very simply: the command [sts]stop[ste] and the channel name."

translate english music_and_sounds_cccc75ce:
	# rn "А для плавной остановки здесь тоже есть параметр [sts]fadeout[ste]."
	rn "And for smooth stopping, there's also a parameter [sts]fadeout[ste]."

translate english music_and_sounds_22da30c5:
	# rn "Думаю, для начала тебе этого хватит."
	rn "I think that should be enough to get started."

translate english music_and_sounds_4d2b6f15:
	# rn "Конечно, в движке для работы с музыкой есть множество инструментов: громкость, пауза, перемотка, очереди, создание своих каналов и много чего ещё..."
	rn "Of course, the engine has many tools for working with music: volume, pause, seeking, queues, creating your own channels, and much more..."

translate english music_and_sounds_3e271fca:
	# rn "Однако, вряд ли тебе всё это пригодится в самом начале."
	rn "However, it's unlikely that all of this will be useful right at the beginning."

translate english music_and_sounds_3922302c:
	# rn "Тем более, что все эти детали ты всегда сможешь найти в документации - я всё ещё призываю тебя не стесняться туда заглядывать за подробностями!"
	rn "Besides, you can always find all these details in the documentation - I still urge you not to hesitate to look there for specifics!"

translate english music_and_sounds_00bc733d:
	# rn "На этом базовая часть туториала подошла к концу."
	rn "That concludes the basic part of this tutorial."

translate english music_and_sounds_e26dd7b9:
	# rn "До встречи в других разделах!"
	rn "See you in other sections!"
