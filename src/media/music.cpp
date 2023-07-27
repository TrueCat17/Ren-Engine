#include "music.h"

#include <thread>

#include <SDL2/SDL_audio.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include <Python.h>

#include "gv.h"
#include "media/py_utils.h"

#include "utils/algo.h"
#include "utils/math.h"
#include "utils/scope_exit.h"
#include "utils/string.h"
#include "utils/utils.h"


static const size_t PART_SIZE = (1 << 10) * 32;//32 Kb
static const size_t MIN_PART_COUNT = 2;
static const size_t MAX_PART_COUNT = 5;

static const size_t SAMPLE_SIZE = 2 * 2;//stereo (2 channels), 16 bits (2 bytes)
static const size_t SAMPLES_PER_PART = PART_SIZE / SAMPLE_SIZE;


static std::mutex musicMutex;

static double startUpdateTime = 0;

static std::map<std::string, double> mixerVolumes;

static std::vector<Channel*> channels;
static std::vector<Music*> musics;


static Channel* findChannel(const std::string &name) {
	for (Channel *channel : channels) {
		if (channel->name == name) return channel;
	}
	return nullptr;
}
static Music* findMusic(const std::string &channelName, const std::string &from, const std::string &place) {
	if (!Music::hasChannel(channelName)) {
		Utils::outMsg(from, "Channel <" + channelName + "> not found\n\n" + place);
		return nullptr;
	}

	for (Music *music : musics) {
		if (music->getChannel()->name == channelName) return music;
	}
	return nullptr;
}



static void fillAudio(void *, Uint8 *stream, int globalLen) {
	std::lock_guard g(musicMutex);

	SDL_memset(stream, 0, size_t(globalLen));

	for (Music *music : musics) {
		if (!music->audioPos || music->audioLen <= 0) continue;
		if (music->paused) continue;

		Uint32 len = std::min<Uint32>(Uint32(globalLen), music->audioLen);
		music->addToCurTime(len);

		int volume = music->getVolume();
		if (volume > 0) {
			SDL_MixAudio(stream, music->audioPos, len, volume);
		}
		music->audioPos += len;
		music->audioLen -= len;
	}
}
static void startMusic() {
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
}
static void stopMusic() {
	SDL_CloseAudio();
}

void Music::loop() {
	Utils::setThreadName("music_loop");

	while (!GV::exit) {
		startUpdateTime = Utils::getTimer();

		{
			std::lock_guard g(musicMutex);

			for (size_t i = 0; i < musics.size(); ++i) {
				if (GV::exit) return;

				Music *music = musics[i];
				if (!music->isEnded()) {
					music->update();
					continue;
				}

				musics.erase(musics.begin() + long(i));
				--i;
				delete music;

				if (musics.empty()) {
					stopMusic();
				}
			}
		}
		const double spend = Utils::getTimer() - startUpdateTime;
		Utils::sleep(0.010 - spend, false);
	}
}

void Music::init() {
	av_log_set_level(AV_LOG_ERROR);
	std::thread(loop).detach();
}

void Music::clear() {
	std::lock_guard g(musicMutex);

	stopMusic();
	mixerVolumes.clear();

	for (Channel *t : channels) {
		delete t;
	}
	channels.clear();

	for (Music *t : musics) {
		delete t;
	}
	musics.clear();
}

void Music::registerChannel(const std::string &name, const std::string &mixer, bool loop,
                            const std::string &fileName, size_t numLine)
{
	std::lock_guard g(musicMutex);

	if (Music::hasChannel(name)) {
		const std::string place = "File <" + fileName + ">\n"
		                          "Line " + std::to_string(numLine);
		Utils::outMsg("Music::registerChannel",
		              "Channel <" + name + "> already exists\n\n" + place);
		return;
	}

	Channel *channel = new Channel(name, mixer, loop);
	channels.push_back(channel);

	if (mixerVolumes.find(mixer) == mixerVolumes.end()) {
		mixerVolumes[mixer] = 1;
	}
}
bool Music::hasChannel(const std::string &name) {
	return findChannel(name);
}

PyObject* Music::getAudioLen(const std::string &url) {
	AVFormatContext* formatCtx = avformat_alloc_context();
	ScopeExit se([&]() {
		avformat_close_input(&formatCtx);
	});

	if (int error = avformat_open_input(&formatCtx, url.c_str(), nullptr, nullptr)) {
		if (error == AVERROR(ENOENT)) {
			Utils::outMsg("Music::getAudioLen: avformat_open_input",
			              "File <" + url + "> not found");
		}else {
			Utils::outMsg("Music::getAudioLen: avformat_open_input",
			              "Failed to open input stream in file <" + url + ">");
		}
		Py_RETURN_NONE;
	}
	if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
		Utils::outMsg("Music::getAudioLen: avformat_find_stream_info",
		              "Failed to read stream information in file <" + url + ">");
		Py_RETURN_NONE;
	}

	double duration = double(formatCtx->duration) / double(AV_TIME_BASE);
	return PyFloat_FromDouble(duration);
}

void Music::setMixerVolume(double volume, const std::string &mixer,
                           const std::string &fileName, size_t numLine)
{
	if (mixerVolumes.count(mixer)) {
		mixerVolumes[mixer] = Math::inBounds(volume, 0, 1);
	}else {
		const std::string place = "File <" + fileName + ">\n"
		                          "Line " + std::to_string(numLine);
		Utils::outMsg("Music::setMixerVolume",
		              "Mixer <" + mixer + "> is not used by any channel now\n\n" + place);
	}
}

void Music::setVolumeOnChannel(double volume, const std::string &channelName,
                               const std::string &fileName, size_t numLine)
{
	Channel *channel = findChannel(channelName);
	if (channel) {
		channel->volume = Math::inBounds(volume, 0, 1);
	}else {
		const std::string place = "File <" + fileName + ">\n"
		                          "Line " + std::to_string(numLine);
		Utils::outMsg("Music::setVolumeOnChannel",
		              "Channel <" + channelName + "> not found\n\n" + place);
	}
}



PyObject* Music::getPosOnChannel(const std::string &channelName,
                                 const std::string &fileName, size_t numLine)
{
	const std::string place = "File <" + fileName + ">\n"
	                          "Line " + std::to_string(numLine);
	Music *music = findMusic(channelName, "Music::getPosOnChannel", place);
	if (music) {
		return PyFloat_FromDouble(music->getPos());
	}
	Py_RETURN_NONE;
}
void Music::setPosOnChannel(double sec, const std::string &channelName,
                            const std::string &fileName, size_t numLine)
{
	const std::string place = "File <" + fileName + ">\n"
	                          "Line " + std::to_string(numLine);
	Music *music = findMusic(channelName, "Music::setPosOnChannel", place);
	if (music) {
		music->setPos(sec);
	}
}


PyObject* Music::getPauseOnChannel(const std::string &channelName,
                                   const std::string &fileName, size_t numLine)
{
	PyObject *res;

	const std::string place = "File <" + fileName + ">\n"
	                          "Line " + std::to_string(numLine);
	Music *music = findMusic(channelName, "Music::getPauseOnChannel", place);
	if (music) {
		if (music->paused) {
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
void Music::setPauseOnChannel(bool value, const std::string &channelName,
                              const std::string &fileName, size_t numLine)
{
	const std::string place = "File <" + fileName + ">\n"
	                          "Line " + std::to_string(numLine);
	Music *music = findMusic(channelName, "Music::setPauseOnChannel", place);
	if (music) {
		music->paused = value;
	}
}


void Music::play(const std::string &desc,
                 const std::string &fileName, size_t numLine)
{
	const std::string place = "File <" + fileName + ">\n"
	                          "Line " + std::to_string(numLine) + ": <" + desc + ">";

	std::vector<std::string> args = Algo::getArgs(desc);
	if (args.size() % 2) {
		Utils::outMsg("Music::play",
		              "Expected even number of arguments:\n"
		              "channelName fileName ['fadein' time] ['volume' value]\n"
		              "Got: <" + desc + ">\n\n" + place);
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
			Utils::outMsg("Music::play",
			              "Expected <fadein> or <volume>, got <" + name + ">\n\n" + place);
			return;
		}
		std::string valueStr = PyUtils::exec(fileName, numLine, valueCode, true);
		if (!String::isNumber(valueStr)) {
			Utils::outMsg("Music::play",
			              "Expected number, got <" + valueStr + ">\n\n" + place);
			return;
		}

		double res = String::toDouble(valueStr);
		if (name == "fadein") {
			fadeIn = res;
		}else {
			volume = res;
			if (volume < 0 || volume > 1) {
				Utils::outMsg("Music::play",
				              "Expected volume between 0 and 1, got <" + valueStr + ">\n\n" + place);
				volume = Math::inBounds(volume, 0, 1);
			}
		}
	}

	std::lock_guard g(musicMutex);

	bool wasEmpty = musics.empty();

	for (size_t i = 0; i < musics.size(); ++i) {
		Music *music = musics[i];
		if (music->channel->name == channelName) {
			musics.erase(musics.begin() + long(i));
			delete music;

			break;
		}
	}

	Channel *channel = findChannel(channelName);
	if (channel) {
		Music *music = new Music(url, channel, fadeIn, volume, fileName, numLine, place);
		if (music->initCodec()) {
			delete music;
		}else {
			PyUtils::exec(fileName, numLine, "persistent._seen_audio['" + url + "'] = True");

			if (wasEmpty) {
				startMusic();
			}
			music->update();
			musics.push_back(music);
		}
	}else {
		Utils::outMsg("Music::play", "Channel <" + channelName + "> not found\n\n" + place);
	}
}

void Music::stop(const std::string &desc,
                 const std::string &fileName, size_t numLine)
{
	const std::string place = "File <" + fileName + ">\n"
	                          "Line " + std::to_string(numLine) + ": <" + desc + ">";

	std::vector<std::string> args = Algo::getArgs(desc);
	if (args.size() != 1 && args.size() != 3) {
		Utils::outMsg("Music::stop",
		              "Expected 1 or 3 arguments:\n"
		              "channelName ['fadeout' time]\n"
		              "Got: <" + desc + ">\n\n" + place);
		return;
	}

	double fadeOut = 0;
	if (args.size() > 1) {
		if (args[1] != "fadeout") {
			Utils::outMsg("Music::stop",
			              "2 argument must be <fadeout>\n\n" + place);
			return;
		}
		std::string fadeOutStr = PyUtils::exec(fileName, numLine, args[2], true);
		if (!String::isNumber(fadeOutStr)) {
			Utils::outMsg("Music::stop",
			              "Fadeout value expected number, got <" + args[2] + ">\n\n" + place);
			return;
		}
		fadeOut = String::toDouble(fadeOutStr);
	}

	std::lock_guard g(musicMutex);

	std::string channelName = Algo::clear(args[0]);
	Music *music = findMusic(channelName, "Music::stop", place);
	if (music) {
		music->fadeOut = fadeOut;
		music->startFadeOutTime = Utils::getTimer();
	}
}


const std::vector<Channel*>& Music::getChannels() {
	return channels;
}
const std::vector<Music*>& Music::getMusics() {
	return musics;
}
const std::map<std::string, double>& Music::getMixerVolumes() {
	return mixerVolumes;
}



Music::Music(const std::string &url, Channel *channel, double fadeIn, double volume,
             const std::string &fileName, size_t numLine, const std::string &place):
    url(url),
    channel(channel),
    startFadeInTime(Utils::getTimer()),
    fadeIn(fadeIn),
    relativeVolume(volume),
    fileName(fileName),
    numLine(numLine),
    place(place),

    packet(av_packet_alloc()),
    frame(av_frame_alloc()),

    tmpBuffer((uint8_t *)av_malloc(PART_SIZE)),
    buffer((uint8_t *)av_malloc(PART_SIZE * (MAX_PART_COUNT + 1)))
{ }

Music::~Music() {
	audioPos = nullptr;
	audioLen = 0;

	av_freep(&buffer);
	av_freep(&tmpBuffer);

	av_packet_free(&packet);
	av_frame_free(&frame);

	avcodec_free_context(&codecCtx);
	avformat_close_input(&formatCtx);

	swr_free(&convertCtx);
}


bool Music::initCodec() {
	formatCtx = avformat_alloc_context();
	if (int error = avformat_open_input(&formatCtx, url.c_str(), nullptr, nullptr)) {
		if (error == AVERROR(ENOENT)) {
			Utils::outMsg("Music::initCodec: avformat_open_input",
			              "File <" + url + "> not found\n\n" + place);
		}else {
			Utils::outMsg("Music::initCodec: avformat_open_input",
			              "Failed to open input stream in file <" + url + ">\n\n" + place);
		}
		return true;
	}
	if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
		Utils::outMsg("Music::initCodec: avformat_find_stream_info",
		              "Failed to read stream information in file <" + url + ">\n\n" + place);
		return true;
	}

	for (uint32_t i = 0; i < formatCtx->nb_streams; ++i) {
		if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStream = int(i);
			break;
		}
	}
	if (audioStream == -1) {
		Utils::outMsg("Music::initCodec",
		              "Failed to find audio-stream in file <" + url + ">\n\n" + place);
		return true;
	}

	AVCodecParameters *codecPars = formatCtx->streams[audioStream]->codecpar;
	const AVCodec *codec = avcodec_find_decoder(codecPars->codec_id);
	if (!codec) {
		Utils::outMsg("Music::initCodec: avcodec_find_decoder",
		              "Codec for file <" + url + "> not found\n\n" + place);
		return true;
	}

	codecCtx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(codecCtx, codecPars);

	if (avcodec_open2(codecCtx, codec, nullptr)) {
		Utils::outMsg("Music::initCodec: avcodec_open2",
		              "Failed to open codec for file <" + url + ">\n\n" + place);
		return true;
	}

	AVChannelLayout in, out;
	av_channel_layout_default(&in, codecCtx->ch_layout.nb_channels);
	av_channel_layout_default(&out, 2); //2 = stereo

	convertCtx = nullptr;
	if (swr_alloc_set_opts2(&convertCtx,
	                        &out, AV_SAMPLE_FMT_S16, 44100,
	                        &in, codecCtx->sample_fmt, codecCtx->sample_rate,
	                        0, nullptr))
	{
		Utils::outMsg("Music::initCodec: swr_alloc_set_opts2",
		              "Failed to create SwrContext for convert stream of file <" + url + ">\n\n" + place);
		return true;
	}
	if (swr_init(convertCtx)) {
		Utils::outMsg("Music::initCodec: swr_init",
		              "Failed to init SwrContext for convert stream of file <" + url + ">\n\n" + place);
		return true;
	}

	return false;
}



void Music::update() {
	if (audioLen >= PART_SIZE * MIN_PART_COUNT) return;

	while (audioLen < PART_SIZE * MAX_PART_COUNT && !decoded) {
		loadNextPart();
	}
}
void Music::loadNextPart() {
	while (true) {
		int readError = av_read_frame(formatCtx, packet);
		if (readError < 0) {
			if (readError != AVERROR_EOF) {
				Utils::outMsg("Music::loadNextPart: av_read_frame",
				              "Failed to read frame from file <" + url + ">\n\n" + place);
				continue;
			}

			if (!channel->loop) {
				decoded = true;
				return;
			}

			curTime = -1;
			if (av_seek_frame(formatCtx, audioStream, 0, AVSEEK_FLAG_ANY) < 0) {
				Utils::outMsg("Music::loadNextPart",
				              "Error on restart, playing from:\n" + place);
				return;
			}
			continue;
		}

		if (packet->stream_index != audioStream) {
			av_packet_unref(packet);
			continue;
		}

		int sendError = avcodec_send_packet(codecCtx, packet);
		if (sendError && sendError != AVERROR(EAGAIN)) {
			Utils::outMsg("Music::loadNextPart: avcodec_send_packet",
			              "Failed to send packet to codec of file <" + url + ">\n\n" + place);
			return;
		}

		int reveiveError = !sendError ? avcodec_receive_frame(codecCtx, frame) : 0;
		if (reveiveError && reveiveError != AVERROR(EAGAIN)) {
			av_packet_unref(packet);
			Utils::outMsg("Music::loadNextPart: avcodec_receive_frame",
			              "Failed to receive frame from codec of file <" + url + ">\n\n" + place);
			return;
		}

		int sampleCount = 0;
		if (!sendError && !reveiveError) {
			sampleCount = swr_convert(convertCtx,
			                          &tmpBuffer, SAMPLES_PER_PART,
			                          const_cast<const uint8_t**>(frame->data), frame->nb_samples);
			if (sampleCount < 0) {
				Utils::outMsg("Music::loadNextPart: swr_convert",
				              "Failed to convert frame of file <" + url + ">\n\n" + place);
				av_packet_unref(packet);
				av_frame_unref(frame);
				return;
			}
		}

		if (curTime < 0) {
			auto k = formatCtx->streams[audioStream]->time_base;
			curTime = double(frame->pts) * k.num / k.den;
		}

		if (audioPos && audioLen && audioPos != buffer) {
			memmove(buffer, audioPos, audioLen);
		}
		Uint32 lastLen = Uint32(sampleCount) * SAMPLE_SIZE;
		memcpy(buffer + audioLen, tmpBuffer, lastLen);

		audioPos = buffer;
		audioLen += lastLen;

		av_frame_unref(frame);
		av_packet_unref(packet);
		break;
	}
}

int Music::getVolume() const {
	double max = SDL_MIX_MAXVOLUME * relativeVolume * channel->volume * mixerVolumes[channel->mixer];

	//fadeIn
	if (fadeIn > 0 && startFadeInTime + fadeIn > startUpdateTime) {
		return int(max * (startUpdateTime - startFadeInTime) / fadeIn);
	}

	//fadeOut
	if (fadeOut > 0 && startFadeOutTime + fadeOut > startUpdateTime) {
		return int(max - max * (startUpdateTime - startFadeOutTime) / fadeOut);
	}

	//usual
	return int(max);
}
bool Music::isEnded() const {
	return !audioLen || (startFadeOutTime > 0 && startFadeOutTime + fadeOut < startUpdateTime);
}


const Channel* Music::getChannel() const {
	return channel;
}
const std::string& Music::getUrl() const {
	return url;
}
const std::string& Music::getFileName() const {
	return fileName;
}
size_t Music::getNumLine() const {
	return numLine;
}


double Music::getFadeIn() const {
	return std::max(fadeIn + startFadeInTime - startUpdateTime, 0.0);
}
double Music::getFadeOut() const {
	return std::max(fadeOut + startFadeOutTime - startUpdateTime, 0.0);
}
double Music::getRelativeVolume() const {
	return relativeVolume;
}
double Music::getPos() const {
	return curTime;
}

void Music::setFadeIn(double v) {
	fadeIn = v;
	startFadeInTime = startUpdateTime;
}
void Music::setFadeOut(double v) {
	fadeOut = v;
	startFadeOutTime = startUpdateTime;
}
void Music::setRelativeVolume(double v) {
	relativeVolume = v;
}
void Music::setPos(double sec) {
	std::lock_guard g(musicMutex);

	curTime = -1;

	audioPos = buffer;
	audioLen = 0;

	auto k = formatCtx->streams[audioStream]->time_base;
	int64_t ts = int64_t(sec / k.num * k.den);

	if (av_seek_frame(formatCtx, audioStream, ts, AVSEEK_FLAG_ANY) < 0) {
		Utils::outMsg("Music::setPos", "Failed to rewind file <" + url + ">");
	}else {
		update();
	}
}


void Music::addToCurTime(size_t playedBytesCount) {
	size_t sampleCount = playedBytesCount / SAMPLE_SIZE;
	curTime += double(sampleCount) / 44100;
}
