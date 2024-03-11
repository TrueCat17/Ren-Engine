#include "audio_manager.h"

#include <fstream>
#include <map>
#include <thread>
#include <vector>

#include <SDL2/SDL_audio.h>

#define ALLOW_INCLUDE_AUDIO_H
#include "media/audio.h"
#include "media/py_utils.h"

#include "utils/algo.h"
#include "utils/math.h"
#include "utils/scope_exit.h"
#include "utils/string.h"
#include "utils/utils.h"


std::mutex AudioManager::mutex;
double AudioManager::startUpdateTime = 0;
std::map<std::string, double> AudioManager::mixerVolumes;


static std::vector<Channel*> channels;
static std::vector<Audio*> audios;
static bool opened = false;


static Channel* findChannel(const std::string &name) {
	for (Channel *channel : channels) {
		if (channel->name == name) return channel;
	}
	return nullptr;
}
static Audio* findAudio(const std::string &channelName, const std::string &from, const std::string &place) {
	if (!AudioManager::hasChannel(channelName)) {
		Utils::outMsg(from, "Channel <" + channelName + "> not found\n\n" + place);
		return nullptr;
	}

	for (Audio *audio : audios) {
		if (audio->channel->name == channelName) return audio;
	}
	return nullptr;
}

#define get_place ("File <" + fileName + ">\n" \
                   "Line " + std::to_string(numLine))


static void fillAudio(void *, Uint8 *stream, int globalLen) {
	std::lock_guard g(AudioManager::mutex);

	SDL_memset(stream, 0, size_t(globalLen));

	for (Audio *audio : audios) {
		if (!audio->audioPos || audio->audioLen <= 0) continue;
		if (audio->paused) continue;

		Uint32 len = std::min<Uint32>(Uint32(globalLen), audio->audioLen);
		audio->addToCurTime(len);

		int volume = audio->getVolume();
		if (volume > 0) {
			SDL_MixAudio(stream, audio->audioPos, len, volume);
		}
		audio->audioPos += len;
		audio->audioLen -= len;
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
		AudioManager::startUpdateTime = Utils::getTimer();

		{
			std::lock_guard g(AudioManager::mutex);

			for (size_t i = 0; i < audios.size(); ++i) {
				if (GV::exit) return;

				Audio *audio = audios[i];
				if (!audio->isEnded()) {
					audio->update();
					continue;
				}

				audios.erase(audios.begin() + long(i));
				--i;
				delete audio;

				if (audios.empty()) {
					stopAudio();
				}
			}
		}
		const double spend = Utils::getTimer() - AudioManager::startUpdateTime;
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

	for (Audio *t : audios) {
		delete t;
	}
	audios.clear();
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
	return findChannel(name);
}

PyObject* AudioManager::getAudioLen(const std::string &url) {
	AVFormatContext* formatCtx = avformat_alloc_context();
	ScopeExit se([&]() {
		avformat_close_input(&formatCtx);
	});

	if (int error = avformat_open_input(&formatCtx, url.c_str(), nullptr, nullptr)) {
		if (error == AVERROR(ENOENT)) {
			Utils::outMsg("AudioManager::getAudioLen: avformat_open_input",
			              "File <" + url + "> not found");
		}else {
			Utils::outMsg("AudioManager::getAudioLen: avformat_open_input",
			              "Failed to open input stream in file <" + url + ">");
		}
		Py_RETURN_NONE;
	}
	if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
		Utils::outMsg("AudioManager::getAudioLen: avformat_find_stream_info",
		              "Failed to read stream information in file <" + url + ">");
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

void AudioManager::setVolumeOnChannel(double volume, const std::string &channelName,
                                      const std::string &fileName, size_t numLine)
{
	Channel *channel = findChannel(channelName);
	if (channel) {
		channel->volume = Math::inBounds(volume, 0, 1);
	}else {
		Utils::outMsg("AudioManager::setVolumeOnChannel",
		              "Channel <" + channelName + "> not found\n\n" + get_place);
	}
}



PyObject* AudioManager::getPosOnChannel(const std::string &channelName,
                                        const std::string &fileName, size_t numLine)
{
	Audio *audio = findAudio(channelName, "AudioManager::getPosOnChannel", get_place);
	if (audio) {
		return PyFloat_FromDouble(audio->getPos());
	}
	Py_RETURN_NONE;
}
void AudioManager::setPosOnChannel(double sec, const std::string &channelName,
                                   const std::string &fileName, size_t numLine)
{
	Audio *audio = findAudio(channelName, "AudioManager::setPosOnChannel", get_place);
	if (audio) {
		audio->setPos(sec);
	}
}


PyObject* AudioManager::getPauseOnChannel(const std::string &channelName,
                                          const std::string &fileName, size_t numLine)
{
	PyObject *res;

	Audio *audio = findAudio(channelName, "AudioManager::getPauseOnChannel", get_place);
	if (audio) {
		if (audio->paused) {
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
void AudioManager::setPauseOnChannel(bool value, const std::string &channelName,
                                     const std::string &fileName, size_t numLine)
{
	Audio *audio = findAudio(channelName, "AudioManager::setPauseOnChannel", get_place);
	if (audio) {
		audio->paused = value;
	}
}


void AudioManager::play(const std::string &desc,
                        const std::string &fileName, uint32_t numLine)
{
	const std::string place = get_place + ": <" + desc + ">";

	std::vector<std::string> args = Algo::getArgs(desc);
	if (args.size() % 2 || args.size() < 2) {
		Utils::outMsg("AudioManager::play",
		              "Expected even number of arguments:\n"
		              "channelName fileName ['fadein' time] ['volume' value]\n\n" + place);
		return;
	}

	std::string channelName = Algo::clear(args[0]);
	std::string url = PyUtils::exec(fileName, numLine, args[1], true);
	String::replaceAll(url, "\\", "/");

	double fadeIn = 0;
	double volume = 1;
	for (size_t i = 2; i < args.size(); i += 2) {
		const std::string &name = args[i];
		const std::string &valueCode = args[i + 1];
		if (name != "fadein" && name != "volume") {
			Utils::outMsg("AudioManager::play",
			              "Expected <fadein> or <volume>, got <" + name + ">\n\n" + place);
			return;
		}
		std::string valueStr = PyUtils::exec(fileName, numLine, valueCode, true);
		if (!String::isNumber(valueStr)) {
			Utils::outMsg("AudioManager::play",
			              "Expected number, got <" + valueStr + ">\n\n" + place);
			return;
		}

		double res = String::toDouble(valueStr);
		if (name == "fadein") {
			fadeIn = res;
		}else {
			volume = res;
			if (volume < 0 || volume > 1) {
				Utils::outMsg("AudioManager::play",
				              "Expected volume between 0 and 1, got <" + valueStr + ">\n\n" + place);
				volume = Math::inBounds(volume, 0, 1);
			}
		}
	}

	std::lock_guard g(AudioManager::mutex);

	for (size_t i = 0; i < audios.size(); ++i) {
		Audio *audio = audios[i];
		if (audio->channel->name == channelName) {
			audios.erase(audios.begin() + long(i));
			delete audio;

			break;
		}
	}

	Channel *channel = findChannel(channelName);
	if (!channel) {
		Utils::outMsg("AudioManager::play", "Channel <" + channelName + "> not found\n\n" + place);
		return;
	}

	Audio *audio = new Audio(url, channel, fadeIn, volume, place, fileName, numLine);
	if (audio->initCodec()) {
		delete audio;
		return;
	}

	PyUtils::exec(fileName, numLine, "persistent._seen_audio['" + url + "'] = True");

	if (!opened) {
		startAudio();
	}
	audio->update();
	audios.push_back(audio);
}

void AudioManager::stop(const std::string &desc,
                        const std::string &fileName, uint32_t numLine)
{
	const std::string place = get_place + ": <" + desc + ">";

	std::vector<std::string> args = Algo::getArgs(desc);
	if (args.size() != 1 && args.size() != 3) {
		Utils::outMsg("AudioManager::stop",
		              "Expected 1 or 3 arguments:\n"
		              "channelName ['fadeout' time]\n\n" + place);
		return;
	}

	double fadeOut = 0;
	if (args.size() > 1) {
		if (args[1] != "fadeout") {
			Utils::outMsg("AudioManager::stop",
			              "2 argument must be <fadeout>\n\n" + place);
			return;
		}
		std::string fadeOutStr = PyUtils::exec(fileName, numLine, args[2], true);
		if (!String::isNumber(fadeOutStr)) {
			Utils::outMsg("AudioManager::stop",
			              "Fadeout value expected number, got <" + args[2] + ">\n\n" + place);
			return;
		}
		fadeOut = String::toDouble(fadeOutStr);
	}

	std::lock_guard g(AudioManager::mutex);

	std::string channelName = Algo::clear(args[0]);
	Audio *audio = findAudio(channelName, "AudioManager::stop", place);
	if (audio) {
		audio->setFadeOut(fadeOut);
	}
}



void AudioManager::save(std::ofstream &infoFile) {
	infoFile << channels.size() << '\n';
	for (const Channel *channel : channels) {
		infoFile << channel->name << ' '
		         << channel->mixer << ' '
		         << (channel->loop ? "True" : "False") << ' '
		         << int(channel->volume * 1000) / 1000.0 << '\n';
	}
	infoFile << '\n';

	infoFile << audios.size() << '\n';
	for (const Audio *audio : audios) {
		std::string audioUrl = audio->url;
		std::string audioFileName = audio->fileName;

		infoFile << audioUrl << '\n'
		         << audioFileName << '\n'
		         << audio->numLine << ' '
		         << audio->channel->name << ' '
		         << audio->getFadeIn() << ' '
		         << audio->getFadeOut() << ' '
		         << audio->relativeVolume << ' '
		         << audio->getPos() << ' '
		         << (audio->paused ? "True" : "False") << '\n';
	}
	infoFile << '\n';

	infoFile << mixerVolumes.size() << '\n';
	for (const auto &p : mixerVolumes) {
		infoFile << p.first << ' ' << p.second << '\n';
	}
	infoFile << '\n';
}
void AudioManager::load(std::ifstream &infoFile) {
	auto &is = infoFile;
	std::string tmp;

	const char *loadFile = "CPP_Embed: AudioManager::load";

	std::getline(is, tmp);
	size_t countAudioChannels = size_t(String::toInt(tmp));
	for (size_t i = 0; i < countAudioChannels; ++i) {
		std::getline(is, tmp);
		const std::vector<std::string> tmpVec = String::split(tmp, " ");
		if (tmpVec.size() != 4) {
			Utils::outMsg(loadFile, "In string <" + tmp + "> expected 4 args");
			continue;
		}

		const std::string &name = tmpVec[0];
		const std::string &mixer = tmpVec[1];
		const std::string &loop = tmpVec[2];
		double volume = String::toDouble(tmpVec[3]);

		if (!AudioManager::hasChannel(name)) {
			AudioManager::registerChannel(name, mixer, loop == "True", loadFile, 0);
		}
		AudioManager::setVolumeOnChannel(volume, name, loadFile, 0);
	}
	std::getline(is, tmp);

	std::getline(is, tmp);
	size_t countAudios = size_t(String::toInt(tmp));
	for (size_t i = 0; i < countAudios; ++i) {
		std::string url, fileName;

		std::getline(is, url);
		std::getline(is, fileName);

		std::getline(is, tmp);
		const std::vector<std::string> tmpVec = String::split(tmp, " ");
		if (tmpVec.size() != 7) {
			Utils::outMsg(loadFile, "In string <" + tmp + "> expected 7 args");
			continue;
		}

		uint32_t numLine = uint32_t(String::toInt(tmpVec[0]));
		const std::string &channel = tmpVec[1];
		double fadeIn = String::toDouble(tmpVec[2]);
		double fadeOut = String::toDouble(tmpVec[3]);
		double relativeVolume = String::toDouble(tmpVec[4]);
		double pos = String::toDouble(tmpVec[5]);
		bool paused = tmpVec[6] == "True";

		AudioManager::play(channel + " '" + url + "'", fileName, numLine);
		Audio *audio = nullptr;
		for (Audio *i : audios) {
			if (i->channel->name == channel) {
				audio = i;
				break;
			}
		}

		if (audio) {
			audio->setFadeIn(fadeIn);
			if (fadeOut > 0) {
				audio->setFadeOut(fadeOut);
			}
			audio->relativeVolume = relativeVolume;
			audio->setPos(pos);
			audio->paused = paused;
		}else {
			Utils::outMsg(loadFile,
			              "Sound file <" + url + "> not restored\n"
			              "Played from:\n"
			              "  File <" + fileName + ">\n"
			              "  Line " + std::to_string(numLine));
		}
	}
	std::getline(is, tmp);


	std::getline(is, tmp);
	size_t countMixers = size_t(String::toInt(tmp));
	for (size_t i = 0; i < countMixers; ++i) {
		std::getline(is, tmp);

		const std::vector<std::string> tmpVec = String::split(tmp, " ");
		if (tmpVec.size() != 2) {
			Utils::outMsg(loadFile, "In string <" + tmp + "> expected 2 args");
			continue;
		}

		const std::string &name = tmpVec[0];
		double volume = String::toDouble(tmpVec[1]);

		AudioManager::setMixerVolume(volume, name, loadFile, 0);
	}
	std::getline(is, tmp);
}
