cmake_minimum_required(VERSION 2.8.12)

project(Ren-Engine)


if (NOT(PLATFORM))
	set(PLATFORM "linux-x86_64")

	#set(PLATFORM "linux-i686")
	#set(PLATFORM "win32")
endif()



if (PLATFORM STREQUAL "win32")
	set(OUTPUT_NAME "${PROJECT_NAME}")
	set(CMAKE_CXX_COMPILER "i686-w64-mingw32-g++")
	set(LIB "${CMAKE_CURRENT_SOURCE_DIR}/../libs/win32")

elseif (PLATFORM STREQUAL "linux-i686")
	set(OUTPUT_NAME "${PLATFORM}")
	set(CMAKE_CXX_COMPILER "i686-linux-gnu-g++")
	set(LIB "${CMAKE_CURRENT_SOURCE_DIR}/../libs/linux-i686")

elseif(PLATFORM  STREQUAL "linux-x86_64")
	set(OUTPUT_NAME "${PLATFORM}")
	set(CMAKE_CXX_COMPILER "x86_64-linux-gnu-g++")
	set(LIB "${CMAKE_CURRENT_SOURCE_DIR}/../libs/linux-x86_64")

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
	# disable, because LTO with <C++> is unstable
	#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -flto")

	# enable, because LTO with <C> is stable
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
)

if (PLATFORM STREQUAL "win32")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -mwindows -static -static-libstdc++")
	
	link_libraries(
		:python27.dll
		z
		opengl32
		winmm imm32 version setupapi # for SDL2
		ws2_32 # for avformat
	)
else()
	# for python dynamic module _ssl.so
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker --export-dynamic")

	link_libraries(
		python2.7 dl util
		z # z after python, because python's zlibmodule uses libz
		GL
		jemalloc
		c m
	)
endif()



set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "") # to remove <unrecognized command line option -rdynamic>

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wno-register -Wall -Wextra -Wpedantic")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion -Wsign-conversion -Wfloat-equal")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fopenmp")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")



include_directories(.)

file(GLOB SOURCES
	"*.cpp"
	"gui/*.cpp"
	"gui/screen/*.cpp"
	"media/*.cpp"
	"media/py_utils/*.cpp"
	"parser/*.cpp"
	"utils/*.cpp"
)

add_executable(${OUTPUT_NAME} ${SOURCES})
