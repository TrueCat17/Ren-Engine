# Ren-Engine

## \[links]

* [Discord](https://discord.gg/DBagjrCWVp),
* [Telegram](https://t.me/ren_engine_tg),
* [VK](https://vk.com/ren_engine).

## \[OS]

* Windows: 7+, 32-bit;
* Linux: 32-bit and 64-bit, glibc-2.31+ (2020.02).



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
\- **лаунчер** для создания ваших проектов и управления ими.

Информацию про использование движка и создание своих игр вы можете найти в документации на
[wiki](https://github.com/TrueCat17/Ren-Engine/wiki)
проекта.

#### Сборка
Если вы по какой-то причине решили собрать движок самостоятельно, то см. `build/readme`.  
Но сначала нужно собрать 12 библиотек (см. `libs/readme`), от которых прямо или косвенно он зависит.  
Скачивание всех исходников и их сборка автоматизированы в десяток команд, выполняющиеся за 20 минут.

Скачивание этого репозитория:
```
git clone --depth=1 https://github.com/TrueCat17/Ren-Engine
```
Переход в проект:  
`cd ./Ren-Engine/`  
Скачивание зависимостей (`libs`):
```
git submodule update --init --depth=1
```

#### Версия на Python2.7

На данный момент движок перешёл на Python3.11.  
Старая версия движка, оставленная без обновлений, доступна по
[этой ссылке](https://drive.google.com/file/d/15Ryxox5hGL6_bEt7WgXAVHPKENhuNV78/view).  
Архив содержит демку, лаунчер и документацию (в markdown и html), актуальную на 15.04.23.



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
\- **launcher** for creating and managing your projects.

Info to using Ren-Engine and creating your own games you can find on
[wiki](https://github.com/TrueCat17/Ren-Engine/wiki).  
Unfortunately, parts of engine outside Visual Novells have unstable "api" now (before 1.0 release),
so the documentation has no English translation now, but you can use translator.

#### Building
If you for some reason decided to build engine by yourself, see `build/readme`.  
But before you must build 12 libs (see `libs/readme`), that are direct or indirect dependencies.  
Downloading and building automated to ~10 commands and generally take 20 minutes.

Download this repository:
```
git clone --depth=1 https://github.com/TrueCat17/Ren-Engine
```
Enter to project directory:  
`cd ./Ren-Engine/`  
Download dependencies (`libs`):
```
git submodule update --init --depth=1
```

#### Version in Python2.7

At the moment, the engine has switched to Python3.11.  
The old version of the engine, left without updates, is available at
[this link](https://drive.google.com/file/d/15Ryxox5hGL6_bEt7WgXAVHPKENhuNV78/view).  
The archive contains a demo, launcher and documentation (in markdown and html) that is current as of April 15, 2023.
