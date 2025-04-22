## \[ru]

Папка `guitar_hero` (которая содержит этот `readme.md`) должна находиться в `mods/common/`
(или в `mods/common/games/`, например), чтобы эта мини-игра была доступна не только в "основной" вашей игре,
но и в модах к ней.

Для модов же сделаны списки с путями к нотам и музыке:
* `guitar_hero.note_dirs` - список "нотных" папок (по умолчанию содержит только `songs/` из текущей папки);
* `guitar_hero.fallback_sound_dirs` - список папок, в которых (рекурсивно) ищутся аудио-файлы.

Указанные в `note_dirs` "общие" папки должны содержать другие папки, хранящие информацию о конкретных мелодиях.  
Поиск папки нот для мелодии прекращается после того, как найдена первая подходящая кандидатура,
поэтому в модах свою папку с нотами стоит добавлять через
`.insert(0, "my_path")` (вставить в начало), а не `.append` (добавить в конец),
чтобы можно было поставить свои ноты для уже существующей мелодии (если такое нужно).

Название такой папки является названием мелодии по умолчанию.  
Содержащиеся в ней файлы (необязательны):
* `name` - как и `name`-файлы модов, содержат название ("красивое") и переводы (формат тот же);
* `notes3.txt` - файл нот для лёгкого режима (3 струны);
* `notes4.txt` - для среднего режима;
* `notes6.txt` - для сложного режима;
* `notes.txt` - "общий" файл нот, выбирается, если отсутствует специализированный вариант
(обязателен, если отсутствует любой из специализированных).

Если файл нот отсутствует, то его следует "записать" из "режима записи" (кнопка `R`).  
В этом режиме можно использовать кнопку `0` для псевдо-случайной ноты.  
Указать зерно (при необходимости) можно только вручную.

Формат файла с нотами такой:
* Зерно для генератора случайных чисел (опционально, но всегда в первой строке): `seed 123`;
* Время и струна через пробел: `2.6 3`;
* Струна всегда указывается одна, для "аккорда" нужно у двух подряд идущих строк использовать одинаковое время;
* Если нужная струна уже содержит ноту для указанного времени, то берётся следующая струна;
* Если номер струны превышает кол-во струн, то берётся остаток от деления (`5 -> 1` на среднем уровне);
* Струна `0` - случайная (последовательность остаётся одной и той же для каждого зерна).

Если папка с нотами содержит аудио-файл (с таким же именем, что и у папки, и любым расширением),
то он используется в качестве музыки.  
Иначе этот файл ищется (рекурсивно) в папках, указанных в списке `guitar_hero.fallback_sound_dirs`
(рекомендуется этот вариант).


Список свойств, доступных для настройки:

* `guitar_hero.allowed = []` - список мелодий (названий папок), которые разрешены к проигрыванию
(если пуст, то разрешено всё, что не в `guitar_hero.disallowed`);
* `guitar_hero.disallowed = []` - список мелодий, которые запрещены к проигрыванию
(только если `guitar_hero.allowed` пуст);

* `guitar_hero.screen_size_without_panel = True` - стоит ли учитывать панель справа при центрировании основного экрана;

* `guitar_hero.close_btn = False` - нужно ли показывать кнопку закрытия мини-игры;
* `guitar_hero.close_btn_text = 'Close'` - текст на этой кнопке (будет применена система переводов);

* `guitar_hero.block_difficulty = False` - нужно ли блокировать изменение сложности игроком;
* `guitar_hero.block_playing = False` - нужно ли блокировать начало проигрывания мелодии игроком;

* `guitar_hero.errors_in_row_for_stopping = 5` - сколько ошибок подряд остановят игру (`0` - нисколько);
* `guitar_hero.ok_in_row_for_x2 = 3` - сколько нот нужно "поймать" для включения "комбо-режима";

* `guitar_hero.record_btn = True` - нужно ли показывать кнопку "записи" (обычно требуется блокировать для игроков);
* `guitar_hero.pause_btn = True` - нужно ли показывать кнопку "пауза" (скрин паузы ВСЕГДА ставит мини-игру на паузу);
* `guitar_hero.stop_btn = True` - нужно ли показывать кнопку остановки;

* `guitar_hero.appearance_time = 0.5` - время плавного появления экрана мини-игры (в секундах);
* `guitar_hero.disappearance_time = 0.5` - время исчезновения;

* `guitar_hero.waiting_before_start = 0.5` - время ожидания перед началом (для подготовки);

* `guitar_hero.difficulty = 0` - режим сложности игры (`0, 1, 2`);
* `guitar_hero.difficulty_names = ['easy', 'medium', 'hard']` - названия этих режимов (к ним будет применяться система переводов);

* `guitar_hero.shadow_bg = im.rect('0005')` - затенённое изображение, поверх которого будут ноты и струны;
* `guitar_hero.shadow_xpadding = 70 / 1200` - отступ от "края тени" до крайней струны по горизонтали;
* `guitar_hero.shadow_ypadding = 40 / 675` - аналогично для вертикали;

* `guitar_hero.bg = im.rect('#888')` - фон на весь экран, иногда бывает нужно использовать прозрачный (`#0000`);

* `guitar_hero.panel_bg = im.rect('#DDD')` - фон панели (справа);
* `guitar_hero.panel_size = 250` - ширина панели;

* `guitar_hero.spacing = 12 / 675` - "обычный" зазор между блоками панели;
* `guitar_hero.small_spacing = 5 / 675` - "маленький" зазор между кнопками внутри блока;

* `guitar_hero.separator_bg = im.rect('#000')` - изображение для "разделителя" блоков панели;

* `guitar_hero.string_images` - изображения для струн (под индексом `0` используйте `None`);
* `guitar_hero.note_images` - изображения нот (`0` - `None`);
* `guitar_hero.note_target_images` - изображения "целей" нот снизу (`0` - `None`);

* `guitar_hero.string_counts = [3, 4, 6]` - число струн в разных режимах сложности;
* `guitar_hero.error_times = [0.3, 0.25, 0.2]` - допустимое время ошибки для разных режимов сложности;

* `guitar_hero.note_ignore_time = 1.0` - игнорировать нажатия клавиш (без штрафа),
если нет нот, которые упадут в течение указанного времени;

* `guitar_hero.note_moving_time = 2.0` - время падения ноты от верха до низа;

* `guitar_hero.score_color_usual = '#FF0'` - цвет "обычного" текста, показывающего очки;
* `guitar_hero.score_color_error = '#F00'` - цвет после ошибки;
* `guitar_hero.score_color_good = '#0F0'` - цвет после "пойманной" ноты;
* `guitar_hero.score_color_changed_time = 0.5` - время до смены цвета текста на обычный;

* `guitar_hero.last_game_quality` - качество игры (от `0` до `1`),
устанавливается после остановки проигрывания (`stop()`).


Функции:

* `guitar_hero.show()` показать экран мини-игры (перед этим сделать инициализацию);
* `guitar_hero.hide()` - скрыть;

* `guitar_hero.play()` - начать проигрывание выбранной мелодии;
* `guitar_hero.stop()` - остановиться;
* `guitar_hero.playing()` - идёт ли проигрывание (вернёт `True`/`False`);

* `guitar_hero.pause()` - поставить на паузу;
* `guitar_hero.unpause()` - снять с паузы;

* `guitar_hero.record()` - начать запись нот для выбранной мелодии;

* `guitar_hero.add_difficulty(v)` - добавить сложность (ожидаются `-1` или `1`, внутри функции делаются проверки);
* `guitar_hero.set_difficulty(v)` - установить сложность (ожидаются `0`, `1` или `2`, внутри функции делаются проверки);

* `guitar_hero.get_name_for_song(song)` - получить "красивое" название песни с переводом (из файла `name`) из обычного;

* `guitar_hero.set_song(song)` - выделить песню `song`.


Также для изменения доступны стили, указанные в файле `styles.rpy`.



## \[en]

The `guitar_hero` folder (which contains this `readme.md`) must be in `mods/common/`
(or `mods/common/games/`, for example) so that this minigame is available not only in your "main" game,
but also in mods for it.

For mods, lists with paths to notes and music have been made:
* `guitar_hero.note_dirs` - list of "note" folders (by default contains only `songs/` from the current folder);
* `guitar_hero.fallback_sound_dirs` - list of folders in which audio files are (recursively) searched.

The "common" folders specified in `note_dirs` should contain other folders that store information about specific notes.  
The search for a folder of notes for a melody stops after the first suitable candidate is found,
so in mods, your own folder with notes should be added via
`.insert(0, "my_path")` (insert at the beginning), and not `.append` (add at the end),
so that you can put your own notes for an existing melody (if necessary).

The name of such a folder is the default name of the melody.  
Files contained in it (optional):
* `name` - like `name`-files of mods, they contain the name ("pretty") and translations (the format is the same);
* `notes3.txt` - note file for easy mode (3 strings);
* `notes4.txt` - for medium mode;
* `notes6.txt` - for hard mode;
* `notes.txt` - "common" note file, selected if there is no specialized version
(required if any of the specialized versions are missing).

If the note file is missing, it should be "recorded" from the "record mode" (key `R`).  
In this mode you can use the `0` button for a pseudo-random note.    
You can specify the seed only manually (if necessary).

The format of the file with notes is:
* Seed for random number generator (optional, but always on first line): `seed 123`;
* Time and string separated by space: `2.6 3`;
* One string is always indicated; for a "chord" you need to use the same time for two consecutive strings;
* If the desired string already contains a note for the specified time, then the next string is taken;
* If the string number exceeds the count of strings, then the remainder of the division is taken
(`5 -> 1` at the middle mode);
* String `0` is random (the sequence remains the same for each seed).

If the note folder contains an audio file (with the same name as the folder and any extension), it is used as music.  
Otherwise, this file is searched (recursively) in the folders specified in the `guitar_hero.fallback_sound_dirs` list
(this option is recommended).

The properties and functions are listed above (if their purposes are not obvious to you, use a translator).

The styles specified in the `styles.rpy` file are also available for modification.
