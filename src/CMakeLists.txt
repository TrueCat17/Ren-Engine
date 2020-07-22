cmake_minimum_required(VERSION 2.8.6)

project(Ren-Engine)


if (NOT(PLATFORM))
	set(PLATFORM "linux-x86_64")

	#set(PLATFORM "linux-i686")
	#set(PLATFORM "win32")
endif()



if (PLATFORM STREQUAL "win32")
	set(CMAKE_CXX_COMPILER "i686-w64-mingw32-g++")
	set(LIB "${CMAKE_CURRENT_SOURCE_DIR}/../lib/win32")

elseif (PLATFORM STREQUAL "linux-i686")
	set(CMAKE_CXX_COMPILER "i686-linux-gnu-g++")
	set(LIB "${CMAKE_CURRENT_SOURCE_DIR}/../lib/linux-i686")

elseif(PLATFORM  STREQUAL "linux-x86_64")
	set(CMAKE_CXX_COMPILER "g++")
	set(LIB "${CMAKE_CURRENT_SOURCE_DIR}/../lib/linux-x86_64")

else()
	message(FATAL_ERROR "Unknown PLATFORM")
endif()



if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	set(PROFILE "")
	set(LTO "0")
else()
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -s -static-libgcc")

	#set(PROFILE "generate")
	#set(PROFILE "use")
	#set(LTO "1")
endif()



include_directories("${LIB}/include")
include_directories("${LIB}/include/python2.7")
if ("${LTO}" STREQUAL "1")
	link_directories("${LIB}/lto")
else()
	link_directories("${LIB}/no_lto")
endif()


if ("${LTO}" STREQUAL "1")
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -flto")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -flto")
endif()

if ("${PROFILE}" STREQUAL "generate")
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fprofile-generate")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fprofile-generate")
elseif ("${PROFILE}" STREQUAL "use")
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fprofile-use -fprofile-correction")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fprofile-use -fprofile-correction")
endif()



link_libraries(
	pthread
	stdc++fs
	avformat avcodec avutil swresample
	SDL2 SDL2_image SDL2_ttf
	jpeg png16 webp
	freetype
	brotlidec-static
	brotlicommon-static
	z
)

if (PLATFORM STREQUAL "win32")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -mwindows -static -static-libstdc++")
	
	link_libraries(
		:python27.dll
		opengl32
		winmm imm32 version setupapi # for SDL2
		ws2_32 # for avformat
	)
else()
	# to downgrade glibc-version
	set_property(SOURCE glibc_ver.cpp APPEND_STRING PROPERTY COMPILE_FLAGS " -fno-lto")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,-wrap=exp2,-wrap=exp,-wrap=pow,-wrap=log,-wrap=exp2f,-wrap=expf,-wrap=powf,-wrap=logf,-wrap=fcntl,-wrap=fcntl64")

	link_libraries(
		python2.7 dl util
		GL
		jemalloc
		c m
	)
endif()



set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "") # to remove <unrecognized command line option -rdynamic>

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wno-register -Wall -Wextra -pedantic")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fopenmp")

#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
#set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")



include_directories(.)

set(SOURCES
	
	glibc_ver.cpp

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
	utils/path_finder.cpp
	utils/string.cpp
	utils/utils.cpp
)


add_executable(${PROJECT_NAME} ${SOURCES})
