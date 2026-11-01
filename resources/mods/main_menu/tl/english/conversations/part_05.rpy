translate english conv_part_05_5b49b5fa:
	# rn "Ты когда-нибудь задумывался, что нелинейность - это просто сломанная линейность?"
	rn "Have you ever thought that nonlinearity is just broken linearity?"

translate english conv_part_05_b559f38f:
	# rn "Наверно, тебе любопытно, [player], как и зачем я к этому пришла?"
	rn "Probably, [player], you're curious about how and why I arrived at this conclusion?"

translate english conv_part_05_6e219d5f:
	# rn "Ну, меня долгое время интересовало, как устроены высокоуровневые языки программирования, в чём их отличие от низкоуровневых?"
	rn "Well, for a long time I was interested in how high-level programming languages are structured and what distinguishes them from low-level ones?"

translate english conv_part_05_197067a5:
	# rn "Например, в Питоне ты можешь создать массив и добавить в него строку и число, а потом вывести их."
	rn "For example, in Python you can create an array, add a string and a number to it, and then print them."

translate english conv_part_05_28982c64:
	# rn "А в Си так нельзя. {w=0.5}Почему?"
	rn "In C, you can't do that. {w=0.5}Why?"

translate english conv_part_05_401c651c:
	# rn "Ведь интерпретатор Питона написан на Си! Какая магия в нём применяется, что невозможное становится возможным?"
	rn "After all, the Python interpreter is written in C! What magic does it apply to make the impossible possible?"

translate english conv_part_05_b21a234a:
	# rn "Со временем я во многом разобралась."
	rn "Over time, I figured out much of it."

translate english conv_part_05_053aabff:
	# rn "Например, что все эти \"классы\" в Питоне и C++ - это, упрощённо, таблицы виртуальных функций. Или ещё проще: массивы с указателями на функции."
	rn "For instance, what all these \"classes\" in Python and C++ are - simply put, tables of virtual functions. Or even simpler: arrays of function pointers."

translate english conv_part_05_7f7db965:
	# rn "Шаблонные функции и переопределение операторов в C++ - синтаксический сахар, и не более."
	rn "Template functions and operator overloading in C++ are syntactic sugar, nothing more."

translate english conv_part_05_ccee2fc9:
	# rn "То есть, код на C++ можно без особых проблем преобразовать в код на Си."
	rn "That means code in C++ can be transformed into C code without much trouble."

translate english conv_part_05_ab03040c:
	# rn "Сначала, кстати, так и делали, если хочешь знать."
	rn "First of all, that's exactly what was done if you want to know."

translate english conv_part_05_b186ccd6:
	# rn "Вызов функции в Си - это всего лишь размещение параметров для неё и установка адреса этой функции в регистре счётчика команд."
	rn "A function call in C is just placing parameters for it and setting the address of this function in the instruction pointer register."

translate english conv_part_05_bec4fb2c:
	# rn "Всё очень просто."
	rn "Everything is very simple."

translate english conv_part_05_93606259:
	# rn "Остаются только условия и циклы, всё остальное мы уже разобрали до уровня Ассемблера или машинных кодов, что почти одно и то же."
	rn "Only conditions and loops remain; everything else we've already covered down to the level of Assembly or machine code, which are almost the same thing."

translate english conv_part_05_6054d0ec:
	# rn "Цикл for - это тот же самый цикл while с некоторыми мелкими дополнениями."
	rn "A 'for' loop is just a 'while' loop with some small additions."

translate english conv_part_05_738c9e00:
	# rn "Цикл while - это тот же самый if, у которого в конце есть переход goto к началу."
	rn "A 'while' loop is just an 'if', which has a 'goto' jump at the end back to the beginning."

translate english conv_part_05_f28b0fe9:
	# rn "Операторы continue и break - это, очевидно, просто переходы к началу и концу соответственно."
	rn "The 'continue' and 'break' operators are, obviously, simply jumps to the beginning and end respectively."

translate english conv_part_05_40952cc6:
	# rn "Ахах, надеюсь, я не слишком сумбурно рассказываю, и ты всё ещё поспеваешь за моей мыслью."
	rn "Heh, I hope I'm not explaining too clumsily, and you're still keeping up with my thoughts."

translate english conv_part_05_5008ddf9:
	# rn "Так вот, остался последний вопрос: как же реализован if?"
	rn "So here's the last question: how is 'if' implemented?"

translate english conv_part_05_5ff7de77:
	# rn "Ну... если говорить о разложении на составляющие, то никак."
	rn "Well... if we talk about breaking it down into components, then no way."

translate english conv_part_05_92d2141d:
	# rn "Ведь if есть именно что \"в самом железе\"."
	rn "Because 'if' is literally \"in the hardware itself\"."

translate english conv_part_05_f65ea9e0:
	# rn "Поэтому мне всегда и было интересно, как эта команда там работает?"
	rn "That's why I've always been curious how that command works there?"

translate english conv_part_05_7874723d:
	# rn "Для начала, конечно, стоит отметить, что if там всё-таки несколько отличается от того, что есть в Си. Но иначе ведь и быть не могло."
	rn "First of all, of course, it's worth noting that 'if' there differs a bit from what exists in C. But otherwise, it couldn't be anything else."

translate english conv_part_05_e12eb55d:
	# rn "В процессоре есть инструкция условного перехода - именно её я и называла \"if на уровне железа\"."
	rn "In the processor there is a conditional jump instruction - precisely which one I called \"'if' at the hardware level\"."

translate english conv_part_05_0c580e8e:
	# rn "Если инструкция {b}безусловного{/b} перехода просто устанавливает нужное число в счётчике команд..."
	rn "If an {b}unconditional{/b} jump instruction simply sets the needed number in the instruction pointer register..."

translate english conv_part_05_d1581f7d:
	# rn "...то инструкция {b}условного{/b} перехода делает это только при выполнении какого-то условия."
	rn "...then a {b}conditional{/b} jump instruction does this only when some condition is met."

translate english conv_part_05_87f6ea3f:
	# rn "Например, если в определённом регистре лежит число 0."
	rn "For example, if a certain register holds the number 0."

translate english conv_part_05_2f4bad32:
	# rn "Вот в этом месте мы и возвращаемся к началу нашего разговора: нелинейность - это сломанная линейность."
	rn "Here we return to the beginning of our conversation: nonlinearity is broken linearity."

translate english conv_part_05_6c0b107d:
	# rn "Этим я хотела сказать, что \"устройство\" для выполнения инструкции {b}условного{/b} перехода - это то же самое устройство {b}безусловного{/b} перехода, которое..."
	rn "By that I meant that the \"device\" for executing a {b}conditional{/b} jump instruction is the same device as for an {b}unconditional{/b} jump, which..."

translate english conv_part_05_32d196bf:
	# rn "...непринуждённо физически отключается при невыполнении условия - и просто перестаёт работать."
	rn "...reluctantly physically turns off when the condition is not met and simply stops working."

translate english conv_part_05_2ce95a90:
	# rn "Как какая-нибудь лампочка."
	rn "Like some kind of light bulb."

translate english conv_part_05_e8266440:
	# rn "Конечно, это \"отключение\" тоже ещё надо как-то реализовать, но это делается совсем просто, так что на этом тему, я думаю, можно и закрыть."
	rn "Of course, this \"turning off\" also needs to be implemented somehow, but it's done very simply, so on this topic, I think we can close it."

translate english conv_part_05_993f5e36:
	# rn "Если заинтересовался этим, то просто поищи в интернете что-то вроде \"Как работает компьютер\", можно даже в форме видео. {w=0.5}Да, так просто."
	rn "If you're interested in this, just search online for something like \"How a computer works\", even in video form. {w=0.5}Yes, that simple."

translate english conv_part_05_9ec18965:
	# rn "Надеюсь, сегодня ты узнал от меня что-то новое."
	rn "I hope you learned something new from me today."
