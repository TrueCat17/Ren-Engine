cmake_minimum_required(VERSION 2.8)

project(Ren-Engine)

set(ReleasePlatform "default")


# win32
set(ReleasePlatform "win")

# linux-i686
#set(ReleasePlatform "linux")


link_libraries(
	pthread
	stdc++fs
	SDL2 SDL2_image SDL2_ttf
)

if ("${ReleasePlatform}" STREQUAL "win")
	set(CMAKE_CXX_COMPILER "i686-w64-mingw32-g++-posix")
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/include)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/include/python27)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib/win32)

	link_libraries(python27) # dynamic
	link_libraries(
		msvcr90 # for python
		opengl32
		avformat-58 avcodec-58 avutil-56 swresample-3
	)
elseif ("${ReleasePlatform}" STREQUAL "linux")
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -m32 -include ./force_link_glibc_2.17.h")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -m32 -include ./force_link_glibc_2.17.h")

else() # default
	include_directories(/usr/include/python2.7)

	link_libraries(
		jemalloc
		GL
		avformat avcodec avutil swresample
	)
endif()


if (NOT "${ReleasePlatform}" STREQUAL "win")
	if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
		link_libraries(:libpython2.7.a dl util z) # static
	else()
		link_libraries(python2.7) # dynamic
	endif()
endif()

set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "") # to remove <unrecognized command line option -rdynamic>

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++17 -Wno-register -Wall -Wextra -pedantic")

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


add_executable(${PROJECT_NAME} ${SOURCES})
