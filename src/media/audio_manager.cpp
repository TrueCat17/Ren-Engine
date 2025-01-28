#include "audio_manager.h"

#include <fstream>
#include <map>
#include <thread>
#include <vector>

#include <SDL2/SDL_audio.h>

#define ALLOW_INCLUDE_AUDIO_CHANNEL_H
#include "media/audio_channel.h"
#include "media/py_utils.h"

#include "utils/algo.h"
#include "utils/game.h"
#include "utils/math.h"
#include "utils/scope_exit.h"
#include "utils/string.h"
#include "utils/utils.h"


std::recursive_mutex AudioManager::mutex;
double AudioManager::startUpdateTime = 0;
std::map<std::string, double> AudioManager::mixerVolumes;


static std::vector<Channel*> channels;
static bool opened = false;


static Channel* findChannelWithoutError(const std::string &name) {
	for (Channel *channel : channels) {
		if (channel->name == name) return channel;
	}
	return nullptr;
}

static Channel* findChannel(const std::string &name, const std::string &from, const std::string &place) {
	for (Channel *channel : channels) {
		if (channel->name == name) return channel;
	}

	Utils::outMsg(from, "Channel <" + name + "> not found\n\n" + place);
	return nullptr;
}

#define get_place ("File <" + fileName + ">\n" \
                   "Line " + std::to_string(numLine))


static void fillAudio(void *, Uint8 *stream, int globalLen) {
	std::lock_guard g(AudioManager::mutex);

	SDL_memset(stream, 0, size_t(globalLen));

	for (Channel *channel : channels) {
		if (!channel->audioPos || channel->audioLen <= 0) continue;
		if (channel->paused) continue;

		Uint32 len = std::min<Uint32>(Uint32(globalLen), channel->audioLen);
		channel->addToCurTime(len);

		int volume = channel->getVolume();
		if (volume > 0) {
			SDL_MixAudio(stream, channel->audioPos, len, volume);
		}
		channel->audioPos += len;
		channel->audioLen -= len;
	}
}
static void startAudio() {
	static SDL_AudioSpec wantedSpec;
	wantedSpec.freq = 44100;
	wantedSpec.format = AUDIO_S16SYS;
	wantedSpec.channels = 2;
	wantedSpec.silence = 0;
	wantedSpec.samples = 4096;
	wantedSpec.callback = fillAudio;

	if (SDL_OpenAudio(&wantedSpec, nullptr)) {
		Utils::outMsg("SDL_OpenAudio", SDL_GetError());
		return;
	}
	SDL_PauseAudio(0);
	opened = true;
}
static void stopAudio() {
	if (opened) {
		SDL_CloseAudio();
		opened = false;
	}
}

static void loop() {
	Utils::setThreadName("audio_loop");

	while (!GV::exit) {
		double st = Utils::getTimer();
		AudioManager::startUpdateTime = Game::getGameTime();

		{
			std::lock_guard g(AudioManager::mutex);

			bool allEmpty = true;
			for (Channel *channel : channels) {
				if (GV::exit) return;

				channel->update(false);

				if (!channel->audios.empty()) {
					allEmpty = false;
				}
			}

			if (allEmpty) {
				stopAudio();
			}
		}
		const double spend = Utils::getTimer() - st;
		Utils::sleep(0.010 - spend, false);
	}
}

void AudioManager::init() {
	av_log_set_level(AV_LOG_ERROR);
	std::thread(loop).detach();
}

void AudioManager::clear() {
	std::lock_guard g(AudioManager::mutex);

	stopAudio();
	mixerVolumes.clear();

	for (Channel *t : channels) {
		delete t;
	}
	channels.clear();
}

void AudioManager::registerChannel(const std::string &name, const std::string &mixer, bool loop,
                                   const std::string &fileName, size_t numLine)
{
	std::lock_guard g(AudioManager::mutex);

	if (name.find(' ') != size_t(-1)) {
		Utils::outMsg("AudioManager::registerChannel",
		              "Channel name <" + name + "> must not contain spaces\n\n" + get_place);
		return;
	}
	if (mixer.find(' ') != size_t(-1)) {
		Utils::outMsg("AudioManager::registerChannel",
		              "Mixer name <" + mixer + "> must not contain spaces\n\n" + get_place);
		return;
	}

	if (AudioManager::hasChannel(name)) {
		Utils::outMsg("AudioManager::registerChannel",
		              "Channel <" + name + "> already exists\n\n" + get_place);
		return;
	}

	Channel *channel = new Channel(name, mixer, loop);
	channels.push_back(channel);

	if (mixerVolumes.find(mixer) == mixerVolumes.end()) {
		mixerVolumes[mixer] = 1;
	}
}
bool AudioManager::hasChannel(const std::string &name) {
	Channel *channel = findChannelWithoutError(name);
	return channel;
}

PyObject* AudioManager::getPlaying(const std::string &name,
                                   const std::string &fileName, size_t numLine)
{
	std::lock_guard g(AudioManager::mutex);

	const Channel *channel = findChannel(name, "AudioManager::getPlaying", get_place);
	const Audio *audio = channel ? channel->getAudio() : nullptr;
	if (audio) {
		const std::string &path = audio->path;
		return PyUnicode_FromStringAndSize(path.c_str(), Py_ssize_t(path.size()));
	}

	Py_RETURN_NONE;
}

PyObject* AudioManager::getAudioLen(const std::string &path) {
	std::lock_guard g(AudioManager::mutex);

	AVFormatContext* formatCtx = avformat_alloc_context();
	ScopeExit se([&]() {
		avformat_close_input(&formatCtx);
	});

	if (int error = avformat_open_input(&formatCtx, path.c_str(), nullptr, nullptr)) {
		if (error == AVERROR(ENOENT)) {
			Utils::outMsg("AudioManager::getAudioLen: avformat_open_input",
			              "File <" + path + "> not found");
		}else {
			Utils::outMsg("AudioManager::getAudioLen: avformat_open_input",
			              "Failed to open input stream in file <" + path + ">");
		}
		Py_RETURN_NONE;
	}
	if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
		Utils::outMsg("AudioManager::getAudioLen: avformat_find_stream_info",
		              "Failed to read stream information in file <" + path + ">");
		Py_RETURN_NONE;
	}

	double duration = double(formatCtx->duration) / double(AV_TIME_BASE);
	return PyFloat_FromDouble(duration);
}

void AudioManager::setMixerVolume(double volume, const std::string &mixer,
                                  const std::string &fileName, size_t numLine)
{
	if (mixerVolumes.count(mixer)) {
		mixerVolumes[mixer] = Math::inBounds(volume, 0, 1);
	}else {
		Utils::outMsg("AudioManager::setMixerVolume",
		              "Mixer <" + mixer + "> is not used by any channel now\n\n" + get_place);
	}
}


PyObject* AudioManager::getVolumeOnChannel(const std::string &name,
                                           const std::string &fileName, size_t numLine)
{
	std::lock_guard g(AudioManager::mutex);

	const Channel *channel = findChannel(name, "AudioManager::getVolumeOnChannel", get_place);
	if (channel) {
		return PyFloat_FromDouble(channel->volume);
	}

	Py_RETURN_NONE;
}

void AudioManager::setVolumeOnChannel(double volume, const std::string &name,
                                      const std::string &fileName, size_t numLine)
{
	std::lock_guard g(AudioManager::mutex);

	Channel *channel = findChannel(name, "AudioManager::setVolumeOnChannel", get_place);
	if (channel) {
		channel->volume = Math::inBounds(volume, 0, 1);
	}
}



PyObject* AudioManager::getPosOnChannel(const std::string &name,
                                        const std::string &fileName, size_t numLine)
{
	std::lock_guard g(AudioManager::mutex);

	const Channel *channel = findChannel(name, "AudioManager::getPosOnChannel", get_place);
	if (channel && !channel->audios.empty()) {
		return PyFloat_FromDouble(channel->getPos());
	}

	Py_RETURN_NONE;
}

void AudioManager::setPosOnChannel(double sec, const std::string &name,
                                   const std::string &fileName, size_t numLine)
{
	std::lock_guard g(AudioManager::mutex);

	Channel *channel = findChannel(name, "AudioManager::setPosOnChannel", get_place);
	if (channel && !channel->audios.empty()) {
		channel->setPos(sec);
	}
}


PyObject* AudioManager::getPauseOnChannel(const std::string &name,
                                          const std::string &fileName, size_t numLine)
{
	std::lock_guard g(AudioManager::mutex);

	PyObject *res;

	const Channel *channel = findChannel(name, "AudioManager::getPauseOnChannel", get_place);
	if (channel) {
		if (channel->paused) {
			res = Py_True;
		}else {
			res = Py_False;
		}
	}else {
		res = Py_None;
	}

	Py_INCREF(res);
	return res;
}
void AudioManager::setPauseOnChannel(bool value, const std::string &name,
                                     const std::string &fileName, size_t numLine)
{
	std::lock_guard g(AudioManager::mutex);

	Channel *channel = findChannel(name, "AudioManager::setPauseOnChannel", get_place);
	if (channel) {
		channel->paused = value;
	}
}


void AudioManager::play(const std::string &channelName, std::string path,
                        double fadeOut, double fadeIn, double volume,
                        const std::string &fileName, uint32_t numLine)
{
	const std::string place = get_place;

	std::lock_guard g(AudioManager::mutex);

	Channel *channel = findChannel(channelName, "AudioManager::play", place);
	if (!channel) return;

	String::replaceAll(path, "\\", "/");

	Audio *audio = new Audio(path, fadeIn, volume, place, fileName, numLine);
	channel->clearQueue(fadeOut);
	channel->audios.push_back(audio);

	if (!opened) {
		startAudio();
	}
	channel->update(true);

	PyUtils::execWithSetTmp(fileName, numLine, "persistent._seen_audio[tmp] = True", path);
}

void AudioManager::queue(const std::string &channelName, std::string path,
                         double fadeIn, double volume,
                         const std::string &fileName, uint32_t numLine)
{
	const std::string place = get_place;

	std::lock_guard g(AudioManager::mutex);

	Channel *channel = findChannel(channelName, "AudioManager::queue", place);
	if (!channel) return;

	String::replaceAll(path, "\\", "/");

	Audio *audio = new Audio(path, fadeIn, volume, place, fileName, numLine);
	channel->audios.push_back(audio);

	if (!opened) {
		startAudio();
	}
	channel->update(true);

	PyUtils::execWithSetTmp(fileName, numLine, "persistent._seen_audio[tmp] = True", path);
}

void AudioManager::stop(const std::string &channelName, double fadeOut,
                        const std::string &fileName, uint32_t numLine)
{
	const std::string place = get_place;

	std::lock_guard g(AudioManager::mutex);

	Channel *channel = findChannel(channelName, "AudioManager::stop", place);
	if (channel) {
		channel->clearQueue(fadeOut);
	}
}


struct ParseParams {
	std::string fromFunc;
	std::string expectedArgsMsg;

	std::string channel;
	std::vector<std::string> paths;
	double fadeOut, fadeIn, volume;

	bool usePaths, useFadeIn, useFadeOut, useVolume;
};

static bool parseParams(const std::string &desc, const std::string &fileName, uint32_t numLine, ParseParams &params) {
	const std::string place = get_place + ": <" + desc + ">";

	std::vector<std::string> args = Algo::getArgs(desc);

	size_t first = 1;
	if (params.usePaths) {
		++first;
	}

	size_t countArgs = args.size() - first;
	if (countArgs % 2 || args.size() < first) {
		Utils::outMsg(params.fromFunc, params.expectedArgsMsg + "\n\n" + place);
		return true;
	}

	params.channel = args[0];
	if (params.usePaths) {
		PyObject *paths = PyUtils::execRetObj(fileName, numLine, args[1]);
		if (PyUnicode_CheckExact(paths)) {
			Py_ssize_t size;
			const char *data = PyUnicode_AsUTF8AndSize(paths, &size);
			std::string path = std::string(data, size_t(size));
			params.paths.push_back(path);
		}else

		if (PyList_CheckExact(paths) || PyTuple_CheckExact(paths)) {
			size_t len = size_t(Py_SIZE(paths));
			for (size_t i = 0; i < len; ++i) {
				PyObject *pyPath = PySequence_Fast_GET_ITEM(paths, i);
				if (!PyUnicode_CheckExact(pyPath)) {
					std::string pos = std::to_string(i);
					std::string type = paths->ob_type->tp_name;
					Utils::outMsg(params.fromFunc,
					              "Element paths[" + pos + "] must be str, got <" + type + ">\n\n" + place);
					return true;
				}

				Py_ssize_t size;
				const char *data = PyUnicode_AsUTF8AndSize(pyPath, &size);
				std::string path = std::string(data, size_t(size));
				params.paths.push_back(path);
			}
		}

		else {
			std::string type = paths->ob_type->tp_name;
			Utils::outMsg(params.fromFunc, "Arg <paths> must be str, list or tuple, got <" + type + ">\n\n" + place);
			return true;
		}
	}

	params.fadeOut = 0;
	params.fadeIn = 0;
	params.volume = 1;

	for (size_t i = first; i < args.size(); i += 2) {
		const std::string &name = args[i];
		const std::string &valueCode = args[i + 1];

		std::string valueStr = PyUtils::exec(fileName, numLine, valueCode, true);
		if (!String::isNumber(valueStr)) {
			Utils::outMsg(params.fromFunc, "Expected number, got <" + valueStr + ">\n\n" + place);
			return true;
		}
		double res = String::toDouble(valueStr);

		if (params.useFadeOut && name == "fadeout") {
			params.useFadeOut = false;
			params.fadeOut = res;
			continue;
		}
		if (params.useFadeIn && name == "fadein") {
			params.useFadeIn = false;
			params.fadeIn = res;
			continue;
		}
		if (params.useVolume && name == "volume") {
			params.useVolume = false;

			if (res < 0 || res > 1) {
				Utils::outMsg(params.fromFunc, "Expected volume between 0 and 1, got <" + valueStr + ">\n\n" + place);
				res = Math::inBounds(res, 0, 1);
			}

			params.volume = res;
			continue;
		}

		Utils::outMsg(params.fromFunc, "Unexpected param <" + name + ">\n\n" + place);
		return true;
	}

	//set default value
	if (params.useFadeOut) {
		std::string valueStr = PyUtils::exec(fileName, numLine, "config.fadeout_audio", true);

		if (String::isNumber(valueStr)) {
			params.fadeOut = String::toDouble(valueStr);
		}else {
			Utils::outMsg(params.fromFunc,
			              "Expected number in config.fadeout_audio, got <" + valueStr + ">\n\n" + place);
		}
	}

	return false;
}


void AudioManager::playWithParsing(const std::string &desc, const std::string &fileName, uint32_t numLine) {
	ParseParams params;
	params.fromFunc = "AudioManager::playWithParsing";
	params.expectedArgsMsg = "Expected even number of arguments:\n"
	                         "channel path ['fadeout' time] ['fadein' time] ['volume' value]";
	params.usePaths = true;
	params.useFadeOut = true;
	params.useFadeIn = true;
	params.useVolume = true;

	if (parseParams(desc, fileName, numLine, params)) return;

	for (size_t i = 0; i < params.paths.size(); ++i) {
		const std::string &path = params.paths[i];
		if (i == 0) {
			play(params.channel, path, params.fadeOut, params.fadeIn, params.volume, fileName, numLine);
		}else {
			queue(params.channel, path, 0, params.volume, fileName, numLine);
		}
	}
}

void AudioManager::queueWithParsing(const std::string &desc, const std::string &fileName, uint32_t numLine) {
	ParseParams params;
	params.fromFunc = "AudioManager::queueWithParsing";
	params.expectedArgsMsg = "Expected even number of arguments:\n"
	                         "channel path ['fadein' time] ['volume' value]";
	params.usePaths = true;
	params.useFadeOut = false;
	params.useFadeIn = true;
	params.useVolume = true;

	if (parseParams(desc, fileName, numLine, params)) return;

	for (size_t i = 0; i < params.paths.size(); ++i) {
		const std::string &path = params.paths[i];
		double fadeIn = i == 0 ? params.fadeIn : 0;

		queue(params.channel, path, fadeIn, params.volume, fileName, numLine);
	}
}

void AudioManager::stopWithParsing(const std::string &desc, const std::string &fileName, uint32_t numLine) {
	ParseParams params;
	params.fromFunc = "AudioManager::stopWithParsing";
	params.expectedArgsMsg = "Expected 1 or 3 arguments:\n"
	                         "channel ['fadeout' time]";
	params.usePaths = false;
	params.useFadeOut = true;
	params.useFadeIn = false;
	params.useVolume = false;

	if (parseParams(desc, fileName, numLine, params)) return;

	stop(params.channel, params.fadeOut, fileName, numLine);
}



void AudioManager::save(std::ofstream &infoFile) {
	std::lock_guard g(AudioManager::mutex);

	auto doubleToStr = [](double d) -> std::string {
		char buf[50];
		sprintf(buf, "%.3f", d);

		std::string res = std::string(buf);
		while (true) {
			if (res.empty()) return "0";

			char c = res.back();
			if (c == '0' || c == '.') {
				res.pop_back();
			}else {
				break;
			}
		}

		return res;
	};

	infoFile << channels.size() << '\n';
	for (const Channel *channel : channels) {
		infoFile << channel->name << ' '
		         << channel->mixer << ' '
		         << (channel->loop ? "True" : "False") << ' '
		         << doubleToStr(channel->volume) << ' '

		         << doubleToStr(channel->curTime) << ' '
		         << (channel->paused ? "True" : "False") << ' '
		         << (channel->audios.size()) << '\n';

		for (const Audio *audio : channel->audios) {
			infoFile << audio->path << '\n'
			         << audio->fileName << '\n'
			         << audio->numLine << ' '
			         << doubleToStr(audio->volume) << ' '
			         << doubleToStr(audio->fadeIn) << ' '
			         << doubleToStr(audio->fadeOut) << ' '
			         << doubleToStr(audio->startFadeInTime) << ' '
			         << doubleToStr(audio->startFadeOutTime) << '\n';
		}

		infoFile << '\n';
	}

	infoFile << mixerVolumes.size() << '\n';
	for (const auto &p : mixerVolumes) {
		infoFile << p.first << ' ' << p.second << '\n';
	}
	infoFile << '\n';
}
void AudioManager::load(std::ifstream &infoFile) {
	std::lock_guard g(AudioManager::mutex);

	auto &is = infoFile;
	std::string tmp;

	const char *loadFile = "CPP_Embed: AudioManager::load";

	std::getline(is, tmp);
	size_t countAudioChannels = size_t(String::toInt(tmp));
	for (size_t i = 0; i < countAudioChannels; ++i) {
		std::getline(is, tmp);
		std::vector<std::string> tmpVec = String::split(tmp, " ");
		if (tmpVec.size() != 7) {
			Utils::outMsg(loadFile, "In string <" + tmp + "> expected 7 args");
			return;
		}

		const std::string &name = tmpVec[0];
		const std::string &mixer = tmpVec[1];
		const std::string &loop = tmpVec[2];
		double volume = String::toDouble(tmpVec[3]);
		double curTime = String::toDouble(tmpVec[4]);
		bool paused = tmpVec[5] == "True";
		size_t countAudios = size_t(String::toInt(tmpVec[6]));

		if (!AudioManager::hasChannel(name)) {
			AudioManager::registerChannel(name, mixer, loop == "True", loadFile, 0);
		}

		Channel *channel = findChannelWithoutError(name);
		if (!channel) return;

		channel->volume = volume;

		channel->audios.reserve(countAudios);
		for (size_t j = 0; j < countAudios; ++j) {
			std::string path, fileName;
			std::getline(is, path);
			std::getline(is, fileName);

			std::getline(is, tmp);
			tmpVec = String::split(tmp, " ");
			if (tmpVec.size() != 6) {
				Utils::outMsg(loadFile, "In string <" + tmp + "> expected 6 args");
				return;
			}

			uint32_t numLine = uint32_t(String::toInt(tmpVec[0]));
			double volume = String::toDouble(tmpVec[1]);
			double fadeIn  = String::toDouble(tmpVec[2]);
			double fadeOut = String::toDouble(tmpVec[3]);
			double startFadeInTime  = String::toDouble(tmpVec[4]);
			double startFadeOutTime = String::toDouble(tmpVec[5]);

			std::string place = get_place;

			Audio *audio = new Audio(path, fadeIn, volume, place, fileName, numLine);
			audio->fadeOut = fadeOut;
			audio->startFadeInTime  = startFadeInTime;
			audio->startFadeOutTime = startFadeOutTime;

			channel->audios.push_back(audio);
		}

		if (countAudios) {
			if (!channel->initCodec()) {
				channel->paused = paused;
				channel->setPos(curTime);
			}else {
				delete channel->audios[0];
				channel->audios.erase(channel->audios.begin());
			}
		}

		std::getline(is, tmp);
	}

	size_t countMixers = size_t(String::toInt(tmp));
	for (size_t i = 0; i < countMixers; ++i) {
		std::getline(is, tmp);

		const std::vector<std::string> tmpVec = String::split(tmp, " ");
		if (tmpVec.size() != 2) {
			Utils::outMsg(loadFile, "In string <" + tmp + "> expected 2 args");
			return;
		}

		const std::string &name = tmpVec[0];
		double volume = String::toDouble(tmpVec[1]);

		AudioManager::setMixerVolume(volume, name, loadFile, 0);
	}
	std::getline(is, tmp);

	if (!opened) {
		startAudio();
	}
}
