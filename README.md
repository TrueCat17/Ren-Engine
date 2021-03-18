# Ren-Engine



## \[ru]

Ren-Engine - это движок для игр: Визуальных Новелл и RPG, но не только.  
Принципы работы аналогичны Ren'Py, формат файлов - его же `rpy`.  
Поддерживаются не все возможности, в то же время введены некоторые новые, для некоторой унификации и логичности
изменено поведение в некоторых местах (например, в языке описания интерфейсов ScreenLang или в запуске модов).

Впрочем, функции для Визуальных Новелл на базе Ren'Py реализованы почти полностью.

Самая главная особенность движка - гораздо бОльшая производительность (примерно в 130 раз быстрее).  
Также стоит отметить компактность (13 МБ против 100), скорость запуска, потребление памяти и отсутствие подвисаний.

[Отсюда](https://drive.google.com/open?id=1TUzhBevm2dRokaPw19rMbAauXFzuxjLh)
можно скачать **демку** движка.  
[Отсюда](https://drive.google.com/file/d/1f7fbKDHxvXlO6R2Gy__r4M44ZiTt1rZR/view)
- **лаунчер** для создания ваших проектов и управления ими.

Информацию про использование движка и создание своих игр вы можете найти в документации на
[wiki](https://github.com/TrueCat17/Ren-Engine/wiki)
проекта.

#### Сборка
Если вы по какой-то причине решили собрать движок самостоятельно, то см. `build/readme`.  
Но сначала нужно собрать 12 библиотек (см. `libs/readme`), от которых прямо или косвенно он зависит.  
Скачивание всех исходников и их сборка автоматизированы в десяток команд, выполняющиеся за 20 минут.



## \[en]

Ren-Engine is a engine for games: Visual Novells, RPG, etc...   
It is similar to Ren'Py, file format - Ren'Py's `rpy`.  
Compability is not full, but there are some new opportunities, and something changed for unification and logicality:
for example, language for interface description ScreenLang or starting of mods.

But implemented almost all functions for Visual Novells based on Ren'Py.

The most important feature of Ren-Engine - very high perfomance (x130 as to Ren'Py).  
Also compactness (13 MB vs 100), speed of start, intake of memory and miss of hangs.

[Here](https://drive.google.com/open?id=1TUzhBevm2dRokaPw19rMbAauXFzuxjLh)
you can download **demo**.  
[Here](https://drive.google.com/file/d/1f7fbKDHxvXlO6R2Gy__r4M44ZiTt1rZR/view)
- **launcher** for creating and managing your projects.

Info to using Ren-Engine and creating your own games you can find on
[wiki](https://github.com/TrueCat17/Ren-Engine/wiki).  
Unfortunately, parts of engine outside Visual Novells have unstable "api" now (before 1.0 release),
so the documentation has no English translation now, but you can use translator.

#### Building
If you for some reason decided to build engine by yourself, see `build/readme`.  
But before you must build 12 libs (see `libs/readme`), that are direct or indirect dependences.  
Downloading and building automated to ~10 commands and generally take 20 minutes.
