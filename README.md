# Ren-Engine
Ren-Engine - это движок для игр.
Принципы работы аналогичны Ren'Py.
Формат файлов такой же.
Поддерживаются изображения (image), музыка (play/stop), интерфейсы (screen), условия, циклы, python-вставки и т. д.
Разумеется, поддерживаются ещё далеко не все возможности.
Но главная особенность - гораздо бОльшая производительность (в 23 раза быстрее).
Именно ради этого он и существует.

# Исходники
Код написан на С++ и лежит в src.
Он кроссплатформенный, но для сборки требуются

# Библиотеки:
SDL2, sdl-image, sdl-ttf, boost::python (2.7), boost::filesystem, ffmpeg (lavformat, lavcodec, lavutil, lswresample).
Они тоже кроссплатформенные, но их нужно уметь установить и подключить.
Т. к. на каждой платформе это делается отдельно - у нас 100% будут сборки только для Linux и Windows.
Остальные - не 100%.

# Компиляция
Компилируется всё это дело в Qt (в смысле, через qmake).
Выбор компилятора вроде как не важен...

# Ресурсы
Ресурсы располагаются в resources.
Эта директория должна быть на уровень выше исполняемого (запускаемого) файла.
Здесь находятся
* Стандартные изображения
* Стандартные музыка и звуки
* Шрифты

# Моды
Директория с модами (mods) должна лежать внутри resources.
Главное меню тоже является модом.
Каждый мод должен лежать отдельно.
При загрузке мода рекурсивно загружаются все rpy-файлы из его директории и из engine и common.
**engine - хранилище всего стандартного и предустановленного (в том числе функций и скринов диалога, настроек...) для всех игр на данном движке,  common - хранилище для всего общего всех модов текущей игры**

# Пример использования движка
https://github.com/TrueCat17/es2d
