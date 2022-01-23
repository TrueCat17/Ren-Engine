#include "music.h"

#include <thread>

#include <SDL2/SDL_audio.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include "gv.h"
#include "media/py_utils.h"

#include "utils/algo.h"
#include "utils/math.h"
#include "utils/string.h"
#include "utils/utils.h"


static const size_t PART_SIZE = (1 << 10) * 32;//32 Kb
static const size_t MIN_PART_COUNT = 2;
static const size_t MAX_PART_COUNT = 5;


static std::mutex musicMutex;

static double startUpdateTime = 0;

static std::map<std::string, double> mixerVolumes;

static std::vector<Channel*> channels;
static std::vector<Music*> musics;


static void fillAudio(void *, Uint8 *stream, int globalLen) {
	std::lock_guard g(musicMutex);

	SDL_memset(stream, 0, size_t(globalLen));

	for (size_t i = 0; i < musics.size(); ++i) {
		Music *music = musics[i];
		if (!music->audioPos || music->audioLen <= 0) continue;

		Uint32 len = Uint32(globalLen) > music->audioLen ? music->audioLen : Uint32(globalLen);

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

				if (music->channel->loop && music->ended) {//ended, not stopped
					if (av_seek_frame(music->formatCtx, music->audioStream, 0, AVSEEK_FLAG_ANY) < 0) {
						Utils::outMsg("Music::loop",
						              "Error on restart, play from:\n" + music->place);
					}else {
						music->ended = false;
						music->audioPos = nullptr;
						music->audioLen = 0;
						music->loadNextParts(MIN_PART_COUNT);
						continue;
					}
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

	for (size_t i = 0; i < channels.size(); ++i) {
		Channel *channel = channels[i];
		if (channel->name == name) {
			const std::string place = "File <" + fileName + ">\n"
			                          "Line " + std::to_string(numLine);
			Utils::outMsg("Music::registerChannel",
			              "Channel <" + name + "> already exists\n\n" + place);
			return;
		}
	}

	Channel *t = new Channel(name, mixer, loop);
	channels.push_back(t);

	if (mixerVolumes.find(mixer) == mixerVolumes.end()) {
		mixerVolumes[mixer] = 1;
	}
}
bool Music::hasChannel(const std::string &name) {
	for (const Channel *channel : channels) {
		if (channel->name == name) return true;
	}
	return false;
}


void Music::setVolume(double volume, const std::string &channelName,
                      const std::string &fileName, size_t numLine)
{
	for (Channel *channel : channels) {
		if (channel->name == channelName) {
			channel->volume = Math::inBounds(volume, 0, 1);
			return;
		}
	}

	const std::string place = "File <" + fileName + ">\n"
	                          "Line " + std::to_string(numLine);
	Utils::outMsg("Music::setVolume",
	              "Channel <" + channelName + "> not found\n\n" + place);
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


void Music::play(const std::string &desc,
                 const std::string &fileName, size_t numLine)
{
	const std::string place = "File <" + fileName + ">\n"
	                          "Line " + std::to_string(numLine) + ": <" + desc + ">";

	std::vector<std::string> args = Algo::getArgs(desc);
	if (args.size() != 2 && args.size() != 4) {
		Utils::outMsg("Music::play",
		              "Expected 2 or 4 arguments:\n"
		              "channelName fileName ['fadein' time]\n"
		              "Got: <" + desc + ">\n\n" + place);
		return;
	}

	std::string channelName = Algo::clear(args[0]);
	std::string url = PyUtils::exec(fileName, numLine, args[1], true);
	String::replaceAll(url, "\\", "/");

	double fadeIn = 0;
	if (args.size() > 2) {
		if (args[2] != "fadein") {
			Utils::outMsg("Music::play",
			              "3 argument must be <fadein>\n\n" + place);
			return;
		}
		std::string fadeInStr = PyUtils::exec(fileName, numLine, args[3], true);
		if (!String::isNumber(fadeInStr)) {
			Utils::outMsg("Music::play",
			              "Fadein value expected number, got <" + args[3] + ">\n\n" + place);
			return;
		}
		fadeIn = String::toDouble(fadeInStr);
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

	for (Channel *channel : channels) {
		if (channel->name != channelName) continue;

		Music *music = new Music(url, channel, fadeIn, fileName, numLine, place);
		if (music->initCodec()){
			delete music;
		}else {
			PyUtils::exec(fileName, numLine, "persistent._seen_audio['" + url + "'] = True");

			if (wasEmpty) {
				startMusic();
			}
			music->loadNextParts(MIN_PART_COUNT);
			musics.push_back(music);
		}
		return;
	}

	Utils::outMsg("Music::play", "Channel <" + channelName + "> not found\n\n" + place);
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

	std::string channelName = Algo::clear(args[0]);
	if (!hasChannel(channelName)) {
		Utils::outMsg("Music::stop", "Channel <" + channelName + "> not found\n\n" + place);
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

	for (Music *music : musics) {
		if (music->channel->name != channelName) continue;

		music->fadeOut = fadeOut;
		music->startFadeOutTime = Utils::getTimer();
		return;
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



Music::Music(const std::string &url, Channel *channel, double fadeIn,
             const std::string &fileName, size_t numLine, const std::string &place):
	url(url),
	channel(channel),
    startFadeInTime(Utils::getTimer()),
	fadeIn(fadeIn),
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

	swr_free(&auConvertCtx);
}


bool Music::initCodec() {
	formatCtx = avformat_alloc_context();
	if (int error = avformat_open_input(&formatCtx, url.c_str(), nullptr, nullptr)) {
		if (error == AVERROR(ENOENT)) {
			Utils::outMsg("Music::initCodec: avformat_open_input",
			              "File <" + url + "> not found\n\n" + place);
		}else {
			Utils::outMsg("Music::initCodec: avformat_open_input",
			              "Could not to open input stream in file <" + url + ">\n\n" + place);
		}
		return true;
	}
	if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
		Utils::outMsg("Music::initCodec: avformat_find_stream_info",
		              "Could not to found info about stream in file <" + url + ">\n\n" + place);
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
		              "Could not to found audio-stream in file <" + url + ">\n\n" + place);
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
		              "Could not to open codec for file <" + url + ">\n\n" + place);
		return true;
	}

	int64_t outChannelLayout = AV_CH_LAYOUT_STEREO;
	AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_S16;
	int64_t inChannelLayout = av_get_default_channel_layout(codecCtx->channels);

	auConvertCtx = swr_alloc_set_opts(nullptr, outChannelLayout, outSampleFmt, 44100,
									  inChannelLayout, codecCtx->sample_fmt, codecCtx->sample_rate, 0, nullptr);
	if (!auConvertCtx) {
		Utils::outMsg("Music::initCodec: swr_alloc_set_opts",
		              "Could not to create SwrContext for convert stream of file <" + url + ">\n\n" + place);
		return true;
	}
	if (swr_init(auConvertCtx)) {
		Utils::outMsg("Music::initCodec: swr_init",
		              "Could not to init SwrContext for convert stream of file <" + url + ">\n\n" + place);
		return true;
	}

	return false;
}



void Music::update() {
	if (audioLen < Uint32(PART_SIZE * MIN_PART_COUNT)) {
		loadNextParts(MAX_PART_COUNT);
	}
}
void Music::loadNextParts(size_t count) {
	while (audioLen < PART_SIZE * count && !av_read_frame(formatCtx, packet)) {
		if (packet->stream_index != audioStream) {
			av_packet_unref(packet);
			continue;
		}

		int sendError = avcodec_send_packet(codecCtx, packet);
		if (sendError && sendError != AVERROR(EAGAIN)) {
			Utils::outMsg("Music::loadNextParts: avcodec_send_packet",
			              "Could not to send packet to codec of file <" + url + ">\n\n" + place);
			return;
		}

		int reveiveError = !sendError ? avcodec_receive_frame(codecCtx, frame) : 0;
		if (reveiveError && reveiveError != AVERROR(EAGAIN)) {
			av_packet_unref(packet);
			Utils::outMsg("Music::loadNextParts: avcodec_receive_frame",
			              "Could not to receive frame from codec of file <" + url + ">\n\n" + place);
			return;
		}
		lastFramePts = frame->pts;

		int countSamples = 0;
		if (!sendError && !reveiveError) {
			countSamples = swr_convert(auConvertCtx,
									   &tmpBuffer, PART_SIZE,
			                           const_cast<const uint8_t**>(frame->data), frame->nb_samples);
			if (countSamples < 0) {
				Utils::outMsg("Music::loadNextParts: swr_convert",
				              "Could not to convert frame of file <" + url + ">\n\n" + place);
				av_packet_unref(packet);
				av_frame_unref(frame);
				return;
			}
		}

		if (audioPos && audioLen && audioPos != buffer) {
			memmove(buffer, audioPos, audioLen);
		}
		Uint32 lastLen = Uint32(countSamples) * 2 * 2;//stereo (2 channels), 16 bits (2 bytes)
		memcpy(buffer + audioLen, tmpBuffer, lastLen);

		audioPos = buffer;
		audioLen += lastLen;

		av_frame_unref(frame);
		av_packet_unref(packet);
	}

	if (!audioLen) {
		ended = true;
	}
}

int Music::getVolume() const {
	double max = SDL_MIX_MAXVOLUME * channel->volume * mixerVolumes[channel->mixer];

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
	return ended || (startFadeOutTime > 0 && startFadeOutTime + fadeOut < startUpdateTime);
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
double Music::getPos() const {
	auto k = formatCtx->streams[audioStream]->time_base;
	return double(lastFramePts) * k.num / k.den;
}

void Music::setFadeIn(double v) {
	fadeIn = v;
	startFadeInTime = startUpdateTime;
}
void Music::setFadeOut(double v) {
	fadeOut = v;
	startFadeOutTime = startUpdateTime;
}
void Music::setPos(double sec) {
	std::lock_guard g(musicMutex);

	audioPos = buffer;
	audioLen = 0;

	auto k = formatCtx->streams[audioStream]->time_base;
	int64_t ts = int64_t(sec / k.num * k.den);

	if (av_seek_frame(formatCtx, audioStream, ts, AVSEEK_FLAG_ANY) < 0) {
		Utils::outMsg("Music::setPos", "Could not to rewind file <" + url + ">");
	}else {
		loadNextParts(MIN_PART_COUNT);
	}
}
