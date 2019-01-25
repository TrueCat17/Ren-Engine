cmake_minimum_required(VERSION 2.8)

project(Ren-Engine)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++17 -Wno-register")


if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
#	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fprofile-generate")
#	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fprofile-generate")
	
#	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fprofile-use")
#	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fprofile-use")
	
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -flto")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -flto -s")
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
	
	parser/node.cpp
	parser/parser.cpp
	parser/screen_node_utils.cpp
	parser/syntax_checker.cpp
	
	utils/algo.cpp
	utils/btn_rect.cpp
	utils/game.cpp
	utils/math.cpp
	utils/image_caches.cpp
	utils/mouse.cpp
	utils/string.cpp
	utils/utils.cpp
)

link_libraries(
	pthread
	stdc++fs
	:libpython2.7.a z dl util
	SDL2 SDL2_image SDL2_ttf
)

if (WIN32)
	include_directories(./include)
#	include_directories(./include/python27)
	
	link_libraries(
		msvcr90 # for python
		opengl32
		avformat-57 avcodec-57 avutil-55 swresample-2
	)
else()
	include_directories(/usr/include/python2.7)

	link_libraries(
		jemalloc
		GL
		avformat avcodec avutil swresample
	)
endif()


add_executable(${PROJECT_NAME} ${SOURCES})

