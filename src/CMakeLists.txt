cmake_minimum_required(VERSION 2.8)

project(Ren-Engine)



# win32
#set(CMAKE_CXX_COMPILER "i686-w64-mingw32-g++")
#include_directories(../lib/include)
#include_directories(../lib/include/python27)

# linux-x32
#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -m32 -include ./force_link_glibc_2.17.h")
#set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -m32 -include ./force_link_glibc_2.17.h")



set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++17 -Wno-register")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fopenmp")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fopenmp")

#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
#set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -s")

#    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fprofile-generate")
#    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fprofile-generate")

#    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fprofile-use")
#    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fprofile-use")

    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -flto")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -flto")
endif()


include_directories(.)

set(SOURCES
	
	main.cpp
	config.cpp
	gv.cpp
	logger.cpp
	renderer.cpp
	
	gui/gui.cpp
	gui/display_object.cpp
	gui/group.cpp
	gui/text_field.cpp
	
	gui/screen/child.cpp
	gui/screen/container.cpp
	gui/screen/hotspot.cpp
	gui/screen/image.cpp
	gui/screen/imagemap.cpp
	gui/screen/key.cpp
	gui/screen/screen.cpp
	gui/screen/style.cpp
	gui/screen/text.cpp
	gui/screen/textbutton.cpp
	
	media/image_manipulator.cpp
	media/music.cpp
	media/py_utils.cpp
	media/py_utils/convert_from_py.cpp
	media/py_utils/convert_to_py.cpp
	media/py_utils/make_func.cpp
	media/scenario.cpp
	
	parser/node.cpp
	parser/parser.cpp
	parser/screen_node_utils.cpp
	parser/syntax_checker.cpp
	
	utils/algo.cpp
	utils/btn_rect.cpp
	utils/file_system.cpp
	utils/game.cpp
	utils/image_caches.cpp
	utils/math.cpp
	utils/mouse.cpp
	utils/string.cpp
	utils/utils.cpp
)

link_libraries(
	pthread
	stdc++fs
	SDL2 SDL2_image SDL2_ttf
)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
	if (WIN32)
		link_libraries(lz)
	else()
#		set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
#		link_libraries(c z)
	endif()
	link_libraries(:libpython2.7.a dl util z) # static
else()
	link_libraries(python2.7) # dynamic
endif()

if (WIN32)
	link_libraries(
		msvcr90 # for python
		opengl32
		avformat-57 avcodec-57 avutil-55 swresample-2
	)
else()
	include_directories(/usr/include/python2.7)

#	link_directories(../lib/linux-i686)
	link_libraries(
		jemalloc
		GL
		avformat avcodec avutil swresample
	)
endif()


add_executable(${PROJECT_NAME} ${SOURCES})
