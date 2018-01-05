TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt debug_and_release debug_and_release_target

HEADERS += \
	config.h \
	gv.h \
	logger.h \
	renderer.h \
	\
	gui/gui.h \
	gui/display_object.h \
	gui/group.h \
	gui/text.h \
	\
	gui/screen/screen_prop.h \
	\
	gui/screen/screen_break.h \
	gui/screen/screen_child.h \
	gui/screen/screen_container.h \
	gui/screen/screen_continue.h \
	gui/screen/screen_elif.h \
	gui/screen/screen_else.h \
	gui/screen/screen_for.h \
	gui/screen/screen.h \
	gui/screen/screen_hbox.h \
	gui/screen/screen_hotspot.h \
	gui/screen/screen_if.h \
	gui/screen/screen_image.h \
	gui/screen/screen_imagemap.h \
	gui/screen/screen_key.h \
	gui/screen/screen_null.h \
	gui/screen/screen_python.h \
	gui/screen/screen_textbutton.h \
	gui/screen/screen_text.h \
	gui/screen/screen_vbox.h \
	gui/screen/screen_while.h \
	gui/screen/style.h \
	\
	media/image.h \
	media/music.h \
	media/py_utils.h \
	\
	parser/node.h \
	parser/parser.h \
	parser/syntax_checker.h \
	\
	utils/btn_rect.h \
	utils/game.h \
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
	gui/text.cpp \
	\
	gui/screen/screen_prop.cpp \
	\
	gui/screen/screen_break.cpp \
	gui/screen/screen_child.cpp \
	gui/screen/screen_container.cpp \
	gui/screen/screen_continue.cpp \
	gui/screen/screen.cpp \
	gui/screen/screen_elif.cpp \
	gui/screen/screen_else.cpp \
	gui/screen/screen_for.cpp \
	gui/screen/screen_hbox.cpp \
	gui/screen/screen_hotspot.cpp \
	gui/screen/screen_if.cpp \
	gui/screen/screen_image.cpp \
	gui/screen/screen_imagemap.cpp \
	gui/screen/screen_key.cpp \
	gui/screen/screen_null.cpp \
	gui/screen/screen_python.cpp \
	gui/screen/screen_textbutton.cpp \
	gui/screen/screen_text.cpp \
	gui/screen/screen_vbox.cpp \
	gui/screen/screen_while.cpp \
	gui/screen/style.cpp \
	\
	media/image.cpp \
	media/music.cpp \
	media/py_utils.cpp \
	\
	parser/node.cpp \
	parser/parser.cpp \
	parser/syntax_checker.cpp \
	\
	utils/btn_rect.cpp \
	utils/game.cpp \
	utils/mouse.cpp \
	utils/string.cpp \
	utils/utils.cpp


win32 {
LIBS += -lSDL2 -lSDL2_image -lSDL2_ttf -lopengl32 \
		-pthread \
		-lavformat-57 -lavcodec-57 -lavutil-55 -lswresample-2 \
		-lpython27 -lmsvcr90 \
		$$PWD/boost_system_1_62.a $$PWD/boost_filesystem_1_62.a \
		-llibboost_python-mgw53-mt-1_62

INCLUDEPATH += $$PWD/include/ $$PWD/include/python27/

}else {
LIBS += -lSDL2 -lSDL2_image -lSDL2_ttf -lGL \
		-pthread \
		-lavformat -lavcodec -lavutil -lswresample \
		-lboost_system -lboost_filesystem \
		-lboost_python -lpython2.7 \
		-ljemalloc

INCLUDEPATH += /usr/include/python2.7/
}
