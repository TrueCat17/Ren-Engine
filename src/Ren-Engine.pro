TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt debug_and_release debug_and_release_target

QMAKE_CXXFLAGS += -std=c++17

HEADERS += \
	config.h \
	gv.h \
	logger.h \
	renderer.h \
	\
	gui/gui.h \
	gui/display_object.h \
	gui/group.h \
	gui/text_field.h \
	\
	gui/screen/child.h \
	gui/screen/container.h \
	gui/screen/hotspot.h \
	gui/screen/image.h \
	gui/screen/imagemap.h \
	gui/screen/key.h \
	gui/screen/screen.h \
	gui/screen/style.h \
	gui/screen/text.h \
	gui/screen/textbutton.h \
	\
	media/image_manipulator.h \
	media/music.h \
	media/py_utils.h \
	media/py_utils/convert_from_py.h \
	media/py_utils/convert_to_py.h \
	media/py_utils/make_func.h \
	\
	parser/node.h \
	parser/parser.h \
	parser/screen_node_utils.h \
	parser/syntax_checker.h \
	\
	utils/algo.h \
	utils/btn_rect.h \
	utils/game.h \
	utils/image_caches.h \
	utils/image_typedefs.h \
	utils/math.h \
	utils/mouse.h \
	utils/string.h \
	utils/utils.h

SOURCES += \
	main.cpp \
	config.cpp \
	gv.cpp \
	logger.cpp \
	renderer.cpp \
	\
	gui/gui.cpp \
	gui/display_object.cpp \
	gui/group.cpp \
	gui/text_field.cpp \
	\
	gui/screen/child.cpp \
	gui/screen/container.cpp \
	gui/screen/hotspot.cpp \
	gui/screen/image.cpp \
	gui/screen/imagemap.cpp \
	gui/screen/key.cpp \
	gui/screen/screen.cpp \
	gui/screen/style.cpp \
	gui/screen/text.cpp \
	gui/screen/textbutton.cpp \
	\
	media/image_manipulator.cpp \
	media/music.cpp \
	media/py_utils.cpp \
	media/py_utils/convert_from_py.cpp \
	media/py_utils/convert_to_py.cpp \
	media/py_utils/make_func.cpp \
	\
	parser/node.cpp \
	parser/parser.cpp \
	parser/screen_node_utils.cpp \
	parser/syntax_checker.cpp \
	\
	utils/algo.cpp \
	utils/btn_rect.cpp \
	utils/game.cpp \
	utils/math.cpp \
	utils/image_caches.cpp \
	utils/mouse.cpp \
	utils/string.cpp \
	utils/utils.cpp

QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

if (false) {
	QMAKE_CXXFLAGS += -fprofile-generate
	QMAKE_LFLAGS += -fprofile-generate
}
if (false) {
	QMAKE_CXXFLAGS += -fprofile-use
	QMAKE_LFLAGS += -fprofile-use
}

if (false) {
	QMAKE_CXXFLAGS += -flto
	QMAKE_LFLAGS += -flto
}
CONFIG(release, debug|release) {
	QMAKE_LFLAGS += -s
}

win32 {
LIBS += -pthread \
		-lpython27 -lmsvcr90 \
		-lSDL2 -lSDL2_image -lSDL2_ttf -lopengl32 \
		-lavformat-57 -lavcodec-57 -lavutil-55 -lswresample-2

INCLUDEPATH += $$PWD/include/ $$PWD/include/python27/

}else {
LIBS += -pthread -lstdc++fs -ljemalloc \
		-l:libpython2.7.a -lz -ldl -lutil \
		-lSDL2 -lSDL2_image -lSDL2_ttf -lGL \
		-lavformat -lavcodec -lavutil -lswresample

INCLUDEPATH += /usr/include/python2.7/
}
