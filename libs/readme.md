## Preparing

For start, **if your OS is Windows**, you must download and install
[MSYS2](https://repo.msys2.org/distrib/msys2-i686-latest.exe).  
After it, you must update package database in MSYS2's PACkage MANager:  
`pacman -Syu`  
and (if needed, close MSYS2 and run it again) update rest:  
`pacman -Su`  
If you have some errors with PGP keys, see [this page](https://www.msys2.org/news/#2020-06-29-new-packagers), fix problem and
run prev 2 commands again.  
Also you must open `%msys2_dir%/home/user/.bashrc`, add `PATH="/mingw32/bin:$PATH"` and restart MSYS2.

You must have installed **packages**: `python3`, `git`, `nasm`, `autoconf`, `automake`, `libtool`, `make`, `cmake`, `gcc`, `g++`.  
Examples for:
* MSYS2: `pacman -S python3 git nasm autoconf automake libtool make mingw-w64-i686-cmake mingw-w64-i686-gcc`  
(not just cmake and gcc!),
* Debian-based OS: `apt-get install python3 git nasm autoconf automake libtool make cmake gcc g++`.

**If your OS is Linux**, you must also install:
* for audio subsystem: `libasound-dev libpulse-dev`
* for window subsystem: `libdbus-1-dev libudev-dev libxrandr-dev libxcursor-dev libxxf86vm-dev`
(or just `xorg-dev`, but it is not only needed packages).

***

## Building

1. Open console (MSYS2 for Windows) and change dir to `scripts`:
```
$ cd Ren-Engine/libs/scripts
```


2. Set <start> status:
```
$ ./progress.py
Choose need actions for all libs:
1. Download and build
2. Build
> 1
```


3. Download (~90 MB) and extract (~400 MB) sources:
```
$ ./download.py
```
This step uses cache (dir `download`) and copy (git) or extract (archives) downloaded to `sources`.


4. Check libs ready to build:
```
$ ./check.py
All libs exists.
```
Usually, you must do steps 2, 3 and 4 only once.


5. Copy files (bash-scripts) for building libs:
```
$ ./copy_sh.py
```
Here you can choose platform (x32 or x64, Linux only), enable/disable optimisations that makes building slower, etc...


6. Build libs:
```
$ ./build.py
# ... building prev libs ...
Build <SDL2_ttf>
./conf.sh 1>conf_out.txt 2>conf_err.txt
./make.sh 1>make_out.txt 2>make_err.txt
Ok!
```


7. Copy include-files:
```
$ ./copy_incs.py
```
This step is last, because some include-files generates in `build` (substep `configure`).


After building need dir (for example, `libs/win32/no_lto`) will contain built static-libs (`*.a`).  
But python not built with MinGW, so uses dynamic std-version (`py_win32` contains it, will be auto copy).

***

## Fixing problems

If there are some errors, you can fix it and restart `./build` again.  
Logs and Errors of building you can see at `libs/sources/%lib%` in files
* `conf_out.txt` and `conf_err.txt` for configurate,
* `make_out.txt` and `make_err.txt` for make.

You can edit to fix and run files `./conf.sh` and `./make.sh` in any lib.  
After it, don't forget to open `scripts/progress_list.txt` and to change status fixed lib to `configurated` or `maked`.  
Also you can fix `scripts/%conf-or-make%/%lib%.sh` for `copy_sh.py`, that generates `*.sh`-files from this.

Status any lib in `scripts/progress_list.txt` can set to next values:
* `start` (need to download, probably using cache),
* `downloaded` (need to configure - substep 1/2 of building),
* `configurated` (need to make - substep 2/2 of building),
* `maked` (built, ready to use).

***

Example for building with lto after building without lto:
```
$ ./progress.py
> 2

$ ./copy_sh.py
# choose enable lto

$ ./build.py

# not need ./copy_incs.py, because include-files are common for lto and no_lto
```
