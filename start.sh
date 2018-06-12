#!/bin/sh

# from dir <resources> to dir with file <site.py>
export PYTHONPATH=./py_libs/Lib/
export PYTHONHOME=./py_libs/Lib/

export LD_LIBRARY_PATH=./linux-i686/

chmod +x ./linux-i686/Ren-Engine
./linux-i686/Ren-Engine
