#include "py_set_globals.h"

#include "py_utils/convert_to_py.h"
#include "py_utils/make_func.h"


#include "config.h"
#include "logger.h"

#include "gui/gui.h"
#include "gui/screen/screen.h"

#include "media/audio_manager.h"
#include "media/image_manipulator.h"
#include "media/scenario.h"
#include "media/sprite.h"
#include "media/translation.h"

#include "parser/mods.h"

#include "utils/file_system.h"
#include "utils/game.h"
#include "utils/math.h"
#include "utils/mouse.h"
#include "utils/path_finder.h"
#include "utils/stage.h"
#include "utils/string.h"
#include "utils/utils.h"


static PyObject* getMouse() {
	PyObject *res = PyTuple_New(2);
	PyTuple_SET_ITEM(res, 0, PyLong_FromLong(Mouse::getX() - Stage::x));
	PyTuple_SET_ITEM(res, 1, PyLong_FromLong(Mouse::getY() - Stage::y));
	return res;
}
static PyObject* getLocalMouse() {
	PyObject *res = PyTuple_New(2);
	PyTuple_SET_ITEM(res, 0, PyLong_FromLong(Mouse::getLocalX()));
	PyTuple_SET_ITEM(res, 1, PyLong_FromLong(Mouse::getLocalY()));
	return res;
}

static long ftoi(double d) {
	return long(d);
}
static std::string getCurrentMod() {
	return GV::mainExecNode->params;
}




static PyObject *_global;
static PyObject *_builtinDict;
static PyObject *_builtinStr;

template<typename T>
static void setValue(const char *key, T t) {
	PyObject *res = convertToPy(t);
	PyDict_SetItemString(_global, key, res);
	Py_DECREF(res);
}

template<typename T>
static void setFunc(const char *key, T t) {
	PyObject *pyFunc = makeFuncImpl(key, t, _builtinStr);
	PyDict_SetItemString(_builtinDict, key, pyFunc);
	Py_DECREF(pyFunc);
}

void PySetGlobals::set(PyObject *global, PyObject *builtinDict, PyObject *builtinStr) {
	_global = global;
	_builtinDict = builtinDict;
	_builtinStr = builtinStr;

	setValue("need_save", false);
	setValue("need_screenshot", false);
	setValue("screenshot_width", 640);
	setValue("screenshot_height", int(640 / String::toDouble(Config::get("window_w_div_h"))));

	setFunc("ftoi", ftoi);
	setFunc("get_md5", Utils::md5);

	setFunc("get_current_mod", getCurrentMod);
	setFunc("get_mods", Mods::getList);
	setFunc("_out_msg", Utils::outMsg);
	setFunc("_log_str_with_end", Logger::logWithEnd);

	setFunc("_register_channel", AudioManager::registerChannel);
	setFunc("_has_channel", AudioManager::hasChannel);
	setFunc("_get_audio_len", AudioManager::getAudioLen);
	setFunc("_set_mixer_volume", AudioManager::setMixerVolume);
	setFunc("_get_volume_on_channel", AudioManager::getVolumeOnChannel);
	setFunc("_set_volume_on_channel", AudioManager::setVolumeOnChannel);
	setFunc("_get_pos_on_channel", AudioManager::getPosOnChannel);
	setFunc("_set_pos_on_channel", AudioManager::setPosOnChannel);
	setFunc("_get_pause_on_channel", AudioManager::getPauseOnChannel);
	setFunc("_set_pause_on_channel", AudioManager::setPauseOnChannel);
	setFunc("_play", AudioManager::play);
	setFunc("_queue", AudioManager::queue);
	setFunc("_stop", AudioManager::stop);
	setFunc("_get_playing", AudioManager::getPlaying);

	setFunc("image_was_registered", Sprite::imageWasRegistered);
	setFunc("get_image", Sprite::getImageDeclAt);

	setFunc("replace_screen", Screen::replace);
	setFunc("_show_screen", Screen::addToShow);
	setFunc("_hide_screen", Screen::addToHide);
	setFunc("_allow_arrows", Screen::allowArrowsForCalcedScreen);
	setFunc("has_screen", Screen::hasScreen);
	setFunc("_log_screen_code", Screen::logScreenCode);
	setFunc("_SL_check_events", Screen::checkScreenEvents);

	setFunc("start_mod", Game::startMod);
	setFunc("get_mod_start_time", Game::getModStartTime);
	setFunc("get_current_mod_index", Game::getCurrentModIndex);
	setFunc("_load", Game::load);
	setFunc("exit_from_game", Game::exitFromGame);

	setFunc("_has_label", Game::hasLabel);
	setFunc("_get_all_labels", Game::getAllLabels);
	setFunc("_jump_next", Scenario::jumpNext);

	setFunc("_get_from_hard_config", Game::getFromConfig);
	setFunc("get_args", Game::getArgs);

	setFunc("_sin", Math::getSin);
	setFunc("_cos", Math::getCos);

	setFunc("get_fps", Game::getFps);
	setFunc("set_fps", Game::setFps);

	setFunc("get_stage_width", Stage::getStageWidth);
	setFunc("get_stage_height", Stage::getStageHeight);

	setFunc("set_stage_size", Stage::setStageSize);
	setFunc("set_fullscreen", Stage::setFullscreen);

	setFunc("save_image", ImageManipulator::save);
	setFunc("load_image", ImageManipulator::loadImage);
	setFunc("get_image_width", Game::getImageWidth);
	setFunc("get_image_height", Game::getImageHeight);
	setFunc("get_image_pixel", Game::getImagePixel);

	setFunc("get_mouse", getMouse);
	setFunc("get_local_mouse", getLocalMouse);
	setFunc("get_mouse_down", Mouse::getMouseDown);
	setFunc("set_can_mouse_hide", Mouse::setCanHide);

	setFunc("path_update_location", PathFinder::updateLocation);
	setFunc("path_on_location", PathFinder::findPath);
	setFunc("path_between_locations", PathFinder::findPathBetweenLocations);

	setFunc("_set_lang", Translation::setLang);
	setFunc("_known_languages", Translation::getKnownLanguages);
	setFunc("_", Translation::get);

	setFunc("get_last_tick", Game::getLastTick);
	setFunc("get_game_time", Game::getGameTime);

	setFunc("get_clipboard_text", Utils::getClipboardText);
	setFunc("set_clipboard_text", Utils::setClipboardText);

	setFunc("get_screen_times", GUI::getScreenTimes);

	setFunc("set_scale_quality", Config::setScaleQuality);
	setFunc("get_scale_quality", Config::getScaleQuality);

	setFunc("_get_cwd", FileSystem::getCurrentPath);
	setFunc("_start_file_win32", FileSystem::startFile_win32);

	setFunc("get_engine_version", Utils::getVersion);
}
